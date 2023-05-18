#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include "read_write_pgm.h"
#include "gameoflife.h"

#define CPU_TIME (clock_gettime( CLOCK_MONOTONIC, &ts ), (double)ts.tv_sec +    \
          (double)ts.tv_nsec * 1e-9)

// Update cell values
void update_cell(unsigned char *world, long rows, long world_size, long num_local_rows) {
    for (long i = world_size; i < world_size * (rows + 1); i++) {
        // Calculate column and row from i index
        long col = i % world_size;
        long r = i / world_size;
        // Calculate neighbour of actual cell
        long col_prev = col - 1 >= 0 ? col - 1 : world_size - 1;
        long col_next = col + 1 < world_size ? col + 1 : 0;
        long r_prev = r - 1;
        long r_next = r + 1;
        // Determine the number of dead neighbours
        int sum = world[r_prev * world_size + col_prev] +
                  world[r_prev * world_size + col] +
                  world[r_prev * world_size + col_next] +
                  world[r * world_size + col_prev] +
                  world[r * world_size + col_next] +
                  world[r_next * world_size + col_prev] +
                  world[r_next * world_size + col] +
                  world[r_next * world_size + col_next];
        int cond = sum / DEAD;

        // Update cell
        if (cond == 5 || cond == 6)
            world[i] = ALIVE;
        else
            world[i] = DEAD;
    }
}

void update_cell_serial(unsigned char *world, long world_size) {
    // Fill the first and row of the local matrix with the last row of the last row of the world
    for (long i = 0; i < world_size; i++)
        world[i] = world[world_size * world_size + i];
    // Update the matrix
    for (long i = world_size; i < world_size * (world_size + 1); i++) {
        // Calculate the column and row from the index i
        long col = i % world_size;
        long row = i / world_size;
        // Calculate actual cell neighbour of the actual cell
        long col_prev = col - 1 >= 0 ? col - 1 : world_size - 1;
        long col_next = col + 1 < world_size ? col + 1 : 0;
        long row_prev = row - 1;
        long row_next = row + 1;

        int sum = world[row_prev * world_size + col_prev] +
                  world[row_prev * world_size + col] +
                  world[row_prev * world_size + col_next] +
                  world[row * world_size + col_prev] +
                  world[row * world_size + col_next] +
                  world[row_next * world_size + col_prev] +
                  world[row_next * world_size + col] +
                  world[row_next * world_size + col_next];
        int cond = sum / DEAD;

        // Update the cell
        world[i] = DEAD;
        if (cond >= 5 & cond <= 6)
            world[i] = 0;

        // After the update of the first row we copy updated row on the
        // last row of the entire matrix
        if (i == world_size * 2) {
            for (long j = 0; j < world_size; j++) {
                world[world_size * (world_size + 1) + j] = world[world_size + j];
            }
        }

    }

}

void iterate_serial(unsigned char *world, long world_size, int times, int snap, int debug_info) {
    char *image_filename_prefix = (char *) malloc(60);
    sprintf(image_filename_prefix, "snap/snap_ordered_");
    char *image_filename_suffix = (char *) malloc(60);
    for (int i = 1; i <= times; i++) {
        update_cell_serial(world, world_size);
        if (i % snap == 0) {
            sprintf(image_filename_suffix, "_%05d", i);
            //TODO: write_pgm_image_chunk(world, 255, world_size, world_size, image_filename_prefix, image_filename_suffix, FILE_EXTENSION_PGM, 0, 1, debug_info);
        }
    }
    free(image_filename_prefix);
    free(image_filename_suffix);
}

void iterate(unsigned char *world_local, long world_size, long rows, int rank, int size, int times, int snap
             , MPI_Status *mpi_status, MPI_Request *mpi_request, int debug_info) {
    long num_local_rows = rows + 2;
    int tag_odd = 0;
    int tag_even = 1;
    if (rank == size - 1)
        MPI_Isend(&world_local[(rows) * world_size], world_size, MPI_UNSIGNED_CHAR, 0, 100, MPI_COMM_WORLD, mpi_request);
    char *image_filename_prefix = (char *) malloc(60);
    sprintf(image_filename_prefix, "snap/snap_ordered_");
    char *image_filename_suffix = (char *) malloc(60);
    for (long i = 1; i <= times; i++) {
        if (rank != 0) {
            // send the first rows of the local part of the world
            MPI_Isend(&world_local[world_size], world_size, MPI_UNSIGNED_CHAR, rank - 1, tag_odd, MPI_COMM_WORLD, mpi_request);
            MPI_Isend(&world_local[world_size], world_size, MPI_UNSIGNED_CHAR, rank - 1, tag_odd, MPI_COMM_WORLD, mpi_request);
        }
        // receive needed lines
        if (rank == size - 1) {
            MPI_Recv(world_local, world_size, MPI_UNSIGNED_CHAR, rank - 1, tag_odd, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(&world_local[(num_local_rows - 1) * world_size], world_size, MPI_UNSIGNED_CHAR, 0, 100,
                     MPI_COMM_WORLD, mpi_status);
        }
        if (rank == 0) {
            MPI_Recv(world_local, world_size, MPI_UNSIGNED_CHAR, size - 1, 100, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(&world_local[(num_local_rows - 1) * world_size], world_size, MPI_UNSIGNED_CHAR, 1, tag_odd,
                     MPI_COMM_WORLD, mpi_status);
        }
        if (rank != 0 & rank != size - 1) {
            MPI_Recv(world_local, world_size, MPI_UNSIGNED_CHAR, rank - 1, tag_odd, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(&world_local[(num_local_rows - 1) * world_size], world_size, MPI_UNSIGNED_CHAR, rank + 1, tag_odd,
                     MPI_COMM_WORLD, mpi_status);
        }

        update_cell(world_local, rows, world_size, num_local_rows);

        // send last lines after update
        if (rank == 0) {
            // process 0 sends its first line after the update to the process size-1
            MPI_Isend(&world_local[(rows) * world_size], world_size, MPI_UNSIGNED_CHAR, 1, tag_odd, MPI_COMM_WORLD, mpi_request);
            MPI_Isend(&world_local[world_size], world_size, MPI_UNSIGNED_CHAR, size - 1, 100, MPI_COMM_WORLD, mpi_request);
        }
        if (rank != 0 & rank != size - 1) {
            MPI_Isend(&world_local[(rows) * world_size], world_size, MPI_UNSIGNED_CHAR, rank + 1, tag_odd,
                      MPI_COMM_WORLD, mpi_request);
        }
        if (rank == size - 1 && i != times - 1) {
            MPI_Isend(&world_local[(rows) * world_size], world_size, MPI_UNSIGNED_CHAR, 0, 100, MPI_COMM_WORLD, mpi_request);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (i % snap == 0) {
            sprintf(image_filename_suffix, "_%05ld", i);
            //TODO: write_pgm_image_chunk(world_local, 255, world_size, rows, image_filename_prefix, image_filename_suffix, FILE_EXTENSION_PGM, rank, size, debug_info);
        }
    }
    free(image_filename_prefix);
    free(image_filename_suffix);
}

void run_ordered(char *filename, int times, int s, int *argc, char **argv[], int debug_info) {
    int rank, size;
    unsigned char *world;
    long world_size = 0;
    long local_size = 0;
    int maxval = 0;
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    int mpi_provided_thread_level;
    MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        printf("a problem arised asking for MPI_THREAD_FUNNELED level\n");
        MPI_Finalize();
        exit(1);
    }
    //start time
    double t_start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Read the local chunk world data from file:
      - Calculate the number of local rows
      - Allocate memory for the local world (world_size*(local_rows+2)).
        The first row is used to store the last row of the previous thread (mpi_size-1 if mpi_rank == 0)
        and the last row is used to store the first row of the next thread process.
        In the case of a single MPI Task the first row will store the last row
        of the world_local and the last row will store the first row of the world_local
      - Read the values: the first value is stored in world_local[world_size]
    */
    read_pgm_image(&world, &maxval, &local_size, &world_size, filename, rank, size, debug_info);

    if (size > 1)
        iterate(world, world_size, local_size, rank, size, times, s, &mpi_status, &mpi_request, debug_info);
    else
        iterate_serial(world, world_size, times, s, debug_info);

    MPI_Barrier(MPI_COMM_WORLD);
    char *fname = (char *) malloc(30);
    sprintf(fname, "output/out_ordered");
    //TODO: write_pgm_image_chunk(world, 255, world_size, local_size, fname, "", FILE_EXTENSION_PGM, rank, size, debug_info);
    free(fname);

    MPI_Barrier(MPI_COMM_WORLD);
    //print time
    if (rank == 0)
        printf("%d,%d,%f\n", size, omp_get_max_threads(), MPI_Wtime() - t_start);
    MPI_Finalize();
    free(world);
}
