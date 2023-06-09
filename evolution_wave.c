#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "files_io.h"
#include "gameoflife.h"

#define EVOLUTION_TYPE "wave"
#define IMAGE_FILENAME_PREFIX_SNAP_WAVE "snapshot"
#define IMAGE_FILENAME_PREFIX_FINAL_WAVE "final"

void set_dead_or_alive_wave_single(unsigned char *world, long world_size, int iteration_step, long startX, long startY, int debug_info) {
    // iterate the square wave from 1 cell to squares of size 3, 5, 7... up to world size
    // TODO: manage joined borders of the world when the world size is even
    //       updating of biggest square would lead to multiple cell updating
    for (long square_size = 1; square_size <= world_size; square_size += 2) {
#ifdef DEBUG_ADVANCED_COORDINATES
        if (debug_info > 1)
            printf("DEBUG2 - set_dead_or_alive_wave_single 1 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld\n", iteration_step, startX, startY, square_size);
#endif
        // integer division: 1/2=0 3/2=1 5/2=2 7/2=3...
        long half_size = square_size / 2;
        long x, y; // absolute coordinates of cell to be updated
        long x_prev, x_next, y_prev, y_next; // coords of neighbours
        long array_index;
#ifdef DEBUG_ADVANCED_COORDINATES
        if (debug_info > 1)
            printf("DEBUG2 - set_dead_or_alive_wave_single 2 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, half_size=%ld\n", iteration_step, startX, startY, square_size, half_size);
#endif
        // horizontal and vertical iterations from -half_size to half_size
        // 1:1, 3:-1 0 1, 5: -2 -1 0 1 2,...
        // 1: start from upper left to upper right (constant y)
        y = (startY + half_size) % world_size; // cross boundary if exceeding
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - set_dead_or_alive_wave_single 3 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, half_size=%ld, y=%ld\n", iteration_step, startX, startY, square_size, half_size, y);
#endif
        // upper horizontal line from left to right
        for (long i = startX - half_size; i <= startX + half_size; i++) {
            // actual cell x coordinate, crossing boundary if exceeding
            x = (i + world_size) % world_size;
#ifdef DEBUG_ADVANCED_COORDINATES
            if (debug_info > 1)
                printf("DEBUG2 - set_dead_or_alive_wave_single 4 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld\n", iteration_step, startX, startY, square_size, i, x);
#endif
            // neighbours of actual cell
            x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
            x_next = x + 1 < world_size ? x + 1 : 0;
            y_prev = y - 1 >= 0 ? y - 1 : world_size - 1;
            y_next = y + 1 < world_size ? y + 1 : 0;
#ifdef DEBUG_ADVANCED_COORDINATES
            if (debug_info > 1)
                printf("DEBUG2 - set_dead_or_alive_wave_single 5 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, x_prev=%ld, x_next=%ld, y=%ld, y_prev=%ld, y_next=%ld\n"
                       , iteration_step, startX, startY, square_size, i, x, x_prev, x_next, y, y_prev, y_next);
#endif
            // determine the number of dead neighbours
            int sum = world[y_prev * world_size + x_prev] + // top left
                      world[y_prev * world_size + x] +      // top
                      world[y_prev * world_size + x_next] + // top right
                      world[y * world_size + x_prev] +      // left
                      world[y * world_size + x_next] +      // right
                      world[y_next * world_size + x_prev] + // low left
                      world[y_next * world_size + x] +      // low
                      world[y_next * world_size + x_next];  // low right
#ifdef DEBUG_ADVANCED_COORDINATES
            if (debug_info > 1)
                printf("DEBUG2 - set_dead_or_alive_wave_single 6 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, sum=%d\n", iteration_step, startX, startY, square_size, i, x, sum);
#endif
            int number_of_dead_neighbours = sum / DEAD;
            array_index = y * world_size + x;
#ifdef DEBUG_ADVANCED_COORDINATES
            if (debug_info > 1)
                printf("DEBUG2 - set_dead_or_alive_wave_single 7 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, i, x, y, array_index);
#endif
            // Update cell
            if (world[array_index] == ALIVE // if actual cell is alive
                && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
                // do nothing world_local[i] = ALIVE;
            } else if (world[array_index] == DEAD // if actual cell is dead
                       && (number_of_dead_neighbours == 5))
                world[array_index] = ALIVE;
            else // default is: cell will die
                world[array_index] = DEAD;
        }
#ifdef DEBUG_ADVANCED_COORDINATES
        if (debug_info > 1)
            printf("DEBUG2 - set_dead_or_alive_wave_single 8 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, x, y, array_index);
#endif

        if (square_size > 1) {
            // 2: from upper right down to lower right (constant x)
            // x set in previous  1:
            // right vertical line from top to bottom
            // skip first cell already updated in previous 1:
            // skip last cell that will be updated in next 3:
            for (long j = startY + half_size - 1; j > startY - half_size; j--) {
                // actual cell y coordinate, crossing boundary if exceeding
                y = (j + world_size) % world_size;
                // neighbours of actual cell
                x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
                x_next = x + 1 < world_size ? x + 1 : 0;
                y_prev = y - 1 >= 0 ? y - 1 : world_size - 1;
                y_next = y + 1 < world_size ? y + 1 : 0;
                // determine the number of dead neighbours
                int sum = world[y_prev * world_size + x_prev] + // top left
                          world[y_prev * world_size + x] +      // top
                          world[y_prev * world_size + x_next] + // top right
                          world[y * world_size + x_prev] +      // left
                          world[y * world_size + x_next] +      // right
                          world[y_next * world_size + x_prev] + // low left
                          world[y_next * world_size + x] +      // low
                          world[y_next * world_size + x_next];  // low right
                int number_of_dead_neighbours = sum / DEAD;
                array_index = y * world_size + x;
#ifdef DEBUG_ADVANCED_COORDINATES
                if (debug_info > 1)
                    printf("DEBUG2 - set_dead_or_alive_wave_single 9 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, j=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, j, x, y, array_index);
#endif
                // Update cell
                if (world[array_index] == ALIVE // if actual cell is alive
                    && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
                    // do nothing world_local[i] = ALIVE;
                } else if (world[array_index] == DEAD // if actual cell is dead
                           && (number_of_dead_neighbours == 5))
                    world[array_index] = ALIVE;
                else // default is: cell will die
                    world[array_index] = DEAD;
            }

            // 3: from lower right to lower left (constant y)
            // bottom horizontal line from right to left
            // skip last cell that will be updated in next 4:
            y = (y - 1 + world_size) % world_size;
            for (long i = startX + half_size; i > startX - half_size; i--) {
                // actual cell x coordinate, crossing boundary if exceeding
                x = (i + world_size) % world_size;
                // neighbours of actual cell
                x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
                x_next = x + 1 < world_size ? x + 1 : 0;
                y_prev = y - 1 >= 0 ? y - 1 : world_size - 1;
                y_next = y + 1 < world_size ? y + 1 : 0;
                // determine the number of dead neighbours
                int sum = world[y_prev * world_size + x_prev] + // top left
                          world[y_prev * world_size + x] +      // top
                          world[y_prev * world_size + x_next] + // top right
                          world[y * world_size + x_prev] +      // left
                          world[y * world_size + x_next] +      // right
                          world[y_next * world_size + x_prev] + // low left
                          world[y_next * world_size + x] +      // low
                          world[y_next * world_size + x_next];  // low right
                int number_of_dead_neighbours = sum / DEAD;
                array_index = y * world_size + x;
#ifdef DEBUG_ADVANCED_COORDINATES
                if (debug_info > 1)
                    printf("DEBUG2 - set_dead_or_alive_wave_single 10 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, i, x, y, array_index);
#endif
                // Update cell
                if (world[array_index] == ALIVE // if actual cell is alive
                    && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
                    // do nothing world_local[i] = ALIVE;
                } else if (world[array_index] == DEAD // if actual cell is dead
                           && (number_of_dead_neighbours == 5))
                    world[array_index] = ALIVE;
                else // default is: cell will die
                    world[array_index] = DEAD;
            }

            // 4: from lower left up to upper left (constant x)
            // left vertical line from bottom to top
            // SKIP LAST CELL already updated as FIRST
            x = (x - 1 + world_size) % world_size;
            for (long j = startY - half_size; j < startY + half_size; j++) {
                // actual cell y coordinate, crossing boundary if exceeding
                y = (j + world_size) % world_size;
                // neighbours of actual cell
                x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
                x_next = x + 1 < world_size ? x + 1 : 0;
                y_prev = y - 1 >= 0 ? y - 1 : world_size - 1;
                y_next = y + 1 < world_size ? y + 1 : 0;
                // determine the number of dead neighbours
                int sum = world[y_prev * world_size + x_prev] + // top left
                          world[y_prev * world_size + x] +      // top
                          world[y_prev * world_size + x_next] + // top right
                          world[y * world_size + x_prev] +      // left
                          world[y * world_size + x_next] +      // right
                          world[y_next * world_size + x_prev] + // low left
                          world[y_next * world_size + x] +      // low
                          world[y_next * world_size + x_next];  // low right
                int number_of_dead_neighbours = sum / DEAD;
                array_index = y * world_size + x;
#ifdef DEBUG_ADVANCED_COORDINATES
                if (debug_info > 1)
                    printf("DEBUG2 - set_dead_or_alive_wave_single 11 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, j=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, j, x, y, array_index);
#endif
                // Update cell
                if (world[array_index] == ALIVE // if actual cell is alive
                    && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
                    // do nothing world_local[i] = ALIVE;
                } else if (world[array_index] == DEAD // if actual cell is dead
                           && (number_of_dead_neighbours == 5))
                    world[array_index] = ALIVE;
                else // default is: cell will die
                    world[array_index] = DEAD;
            }
        } // if (square_size > 1)
//        for (int j = startY - half_size; j <= startY + half_size; j++) {}
//                int row = (i + SIZE) % SIZE;
//                int col = (j + SIZE) % SIZE;
//                int count = 0;
//                    for (int dx = -1; dx <= 1; dx++) {
//                    for (int dy = -1; dy <= 1; dy++) {
//                        int neighborRow = (row + dx + SIZE) % SIZE;
//                        int neighborCol = (col + dy + SIZE) % SIZE;
//
//                        if (board[neighborRow][neighborCol]) {
//                            count++;
//                        }
//                    }
//                }
//
//                if (board[row][col]) {
//                    count--; // Subtract current cell from the count
//                }
//
//                if (board[row][col] && (count < 2 || count > 3)) {
//                    board[row][col] = false;
//                } else if (!board[row][col] && count == 3) {
//                    board[row][col] = true;
//                }
    }
}

double evolution_wave_parallel(int mpi_rank, int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char *world_local, long world_size, long local_size
                               , int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_parallel 1 - *alloc* *free* mpi_rank=%d/%d, char *image_filename_suffix = (char *) malloc(60); BEFORE\n", mpi_rank, mpi_size);
#endif
    // TODO: calculate exact string length instead of malloc(60)
    char *image_filename_suffix = (char *) malloc(60);
    if (image_filename_suffix == NULL) {
        perror("Unable to allocate buffer for image_filename_suffix\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    unsigned char *world;
    long size;

    // iterations has to be done by one single process
    // so make process 0 gather all data from the others
    if (mpi_rank != 0) {
        // other processes send matrix to process 0
        MPI_Isend(world_local, (int) (local_size * world_size), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, mpi_request);
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave_parallel 2 - *MPI* MPI_Isend mpi_rank=%d, local_size=%ld, world_size=%ld, destination=0, tag=0\n", mpi_rank, local_size, world_size);
#endif
    } else {
        // process 0 allocates the full matrix and receives data from all other processes
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave_parallel 4 - *alloc* *free* mpi_rank=%d/%d, unsigned char *world = (unsigned char *) malloc(world_size * world_size * sizeof(unsigned char)); BEFORE\n", mpi_rank, mpi_size);
#endif
        world = (unsigned char *) malloc(world_size * world_size * sizeof(unsigned char));
        if (world == NULL) {
            perror("Unable to allocate buffer for world\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // copy values from process zero matrix
        for (long long i = 0; i < local_size * world_size; i++)
            world[i] = world_local[i];
        // receive all other matrix data
        //long size = world_size / mpi_size;
        for (int i = 1; i < mpi_size; i++) {
            size = world_size % mpi_size - i <= 0 ? (long) (world_size / mpi_size) : (long) (world_size / mpi_size) + 1;
            long start = local_size * world_size + (i - 1) * size * world_size;
#ifdef DEBUG2
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_wave_parallel 5 - *MPI* BEFORE MPI_Recv destination=i=%d, tag=0, local_size=%ld, size=%ld, world_size=%ld, start=%ld\n", i, local_size, size, world_size, start);
#ifdef DEBUG3
                for (long long j = start; j < start + size * world_size; j++)
                    printf("DEBUG2 - evolution_wave_parallel 6 world[%lld]=%d\n", j, world[j]);
#endif
            }
#endif
            MPI_Recv(&world[start], (int) (size * world_size), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
#ifdef DEBUG3
        if (debug_info > 1)
            for (long long i = 0; i < world_size * world_size; i++)
                printf("DEBUG2 - evolution_wave_parallel 7 world[%lld]=%d\n", i, world[i]);
#endif

        //// save initial matrix copy as snapshot zero, we force mpi_size=1 to obtain a clean filename snapshot_00000
        //t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, SNAPSHOT_00000, "", FILE_EXTENSION_PGM, 0, 1, debug_info);
    } // process 0
#ifdef DEBUG2
    if (debug_info > 1) printf("DEBUG2 - evolution_wave_parallel 12 iteration END\n");
#endif

    // all processes iterate, but only process 0 does the update and sends results
    // processes > 0 receive back the local chunk to save it to file
    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
        if (mpi_rank == 0) {
            // 8. [ OPTIONAL ] implement the evolution with a square-wave signal from a grid
            //                 point randomly chosen at every time-step.
            long x = 0, y = 0;
#ifdef DEBUG1
            // for testing purposes we want to be able to fix a point to compare results
            if (debug_info == 0) { // activate debug to set x,y to 0,0
#endif
                x = rand() % world_size;
                y = rand() % world_size;
                //printf("DEBUG2 - evolution_wave_parallel 8a start point x=%ld, y=%ld END\n", x, y);
#ifdef DEBUG1
            }
            if (debug_info > 0)
                printf("DEBUG2 - evolution_wave_parallel 8 start point x=%ld, y=%ld END\n", x, y);
#endif

            // square wave update, only serial
            set_dead_or_alive_wave_single(world, world_size, iteration_step, x, y, debug_info);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave_parallel 9 AFTER UPDATE iteration=%d rank=%d\n", iteration_step, mpi_rank);
#endif

            // send back to all processes for parallel writing
            for (int i = 1; i < mpi_size; i++) {
                size = world_size % mpi_size - i <= 0 ? (long) (world_size / mpi_size) : (long) (world_size / mpi_size) + 1;
                long start = local_size * world_size + (i - 1) * size * world_size;
                MPI_Isend(&world[start], (int) (size * world_size), MPI_UNSIGNED_CHAR, i, iteration_step, MPI_COMM_WORLD, mpi_request);
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_wave_parallel 10 - *MPI* MPI_Isend mpi_rank=%d, size=%ld, local_size=%ld, world_size=%ld, destination=i=%d, tag=iteration_step=%d\n", mpi_rank, size, local_size, world_size, i, iteration_step);
#endif
            }
        } // RANK = 0
#ifdef DEBUG2
        if (debug_info > 1) printf("DEBUG2 - evolution_wave_parallel 11 end iteration cycle\n");
#endif
        MPI_Barrier(MPI_COMM_WORLD);
        if (mpi_rank > 0) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave_parallel 3 *MPI* BEFORE MPI_Recv destination=0 tag=iteration=%d mpi_rank=%d, local_size=%ld, world_size=%ld\n", iteration_step, mpi_rank, local_size, world_size);
#endif
            MPI_Recv(world_local, (int) (local_size * world_size), MPI_UNSIGNED_CHAR, 0, iteration_step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        // when needed save snapshot
        if (iteration_step % number_of_steps_between_file_dumps == 0 && iteration_step < number_of_steps) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            if (mpi_rank == 0)
                t_io += file_pgm_write_chunk_noghost(world, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
            else
                t_io += file_pgm_write_chunk_noghost(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
        }
    }
    // copy values back to set final result RANK 0
    if (mpi_rank == 0)
        for (long long i = 0; i < local_size * world_size; i++)
            world_local[i] = world[i];

    //free(image_filename_prefix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_parallel 13 - *alloc* *free* mpi_rank=%d/%d, free(image_filename_suffix); BEFORE\n", mpi_rank, mpi_size);
#endif
    free(image_filename_suffix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_parallel 14 - *alloc* *free* mpi_rank=%d/%d, free(image_filename_suffix); AFTER\n", mpi_rank, mpi_size);
#endif
    return t_io;
}

double evolution_wave_single(unsigned char *world, long world_size, int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_single 1 - *alloc* *free* mpi_rank=0/1, char *image_filename_prefix = (char *) malloc(60); BEFORE\n");
#endif
    // TODO: calculate exact string length instead of malloc(60)
    char *image_filename_prefix = (char *) malloc(60);
    if (image_filename_prefix == NULL) {
        perror("Unable to allocate buffer for image_filename_prefix\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    sprintf(image_filename_prefix, IMAGE_FILENAME_PREFIX_SNAP_WAVE);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_single 2 - *alloc* *free* mpi_rank=0/1, char *image_filename_suffix = (char *) malloc(60); BEFORE\n");
#endif
    //// save initial matrix copy as snapshot zero
    //t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, SNAPSHOT_00000, "", FILE_EXTENSION_PGM, 0, 1, debug_info);
    char *image_filename_suffix = (char *) malloc(60);
    if (image_filename_suffix == NULL) {
        perror("Unable to allocate buffer for image_filename_suffix (single)\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    long x = 0, y = 0;
    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
        // 8. [ OPTIONAL ] implement the evolution with a square-wave signal from a grid
        //                 point randomly chosen at every time-step.
#ifdef DEBUG1
        // for testing purposes we want to be able to fix a point to compare results
        if (debug_info == 0) { // activate debug to set x,y to 0,0
#endif
            x = rand() % world_size;
            y = rand() % world_size;
            //printf("DEBUG1 - evolution_wave_single 1a start point x=%ld, y=%ld END\n", x, y);
#ifdef DEBUG1
        }
        if (debug_info > 0)
            printf("DEBUG1 - evolution_wave_single 1 start point x=%ld, y=%ld END\n", x, y);
#endif

        // square wave update, only serial
        set_dead_or_alive_wave_single(world, world_size, iteration_step, x, y, debug_info);

#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave_single 3 AFTER UPDATE iteration=%d\n", iteration_step);
#endif
        // when needed save snapshot
        if (iteration_step % number_of_steps_between_file_dumps == 0 && iteration_step < number_of_steps) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, image_filename_prefix, image_filename_suffix, FILE_EXTENSION_PGM, 0, 1, debug_info);
        }
    }
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_single 4 - *alloc* *free* mpi_rank=0/1, free(image_filename_prefix); BEFORE\n");
#endif
    free(image_filename_prefix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1) {
        printf("DEBUG2 - evolution_wave_single 5 - *alloc* *free* mpi_rank=0/1, free(image_filename_prefix); AFTER\n");
        printf("DEBUG2 - evolution_wave_single 6 - *alloc* *free* mpi_rank=0/1, free(image_filename_suffix); BEFORE\n");
    }
#endif
    free(image_filename_suffix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave_single 7 - *alloc* *free* mpi_rank=0/1, free(image_filename_suffix); AFTER\n");
#endif
    return t_io;
}

void evolution_wave(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info) {
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    int mpi_provided_thread_level;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 1 MPI_Init_thread BEFORE\n");
#endif
    MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        perror("a problem occurred asking for MPI_THREAD_FUNNELED level\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
#endif
    int mpi_rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 2 *alloc* *free* mpi_rank=%d/%d, MPI_Init_thread AFTER\n", mpi_rank, mpi_size);
#endif

    double t_io = 0; // total I/O time spent
    double t_io_accumulator = 0; // total I/O time spent by processes > 0
    double t_start = MPI_Wtime(); // start time
    char *directoryname;
    int directoryname_len = 0;
    const char *file_extension_pgm = FILE_EXTENSION_PGM;
    const char *file_extension_pgmpart = FILE_EXTENSION_PGMPART;
    const char *partial_file_extension = file_extension_pgmpart; // default is partial
    // chunk of the world_local for each MPI process
    unsigned char *world_local;
    long world_size = 0;
    long local_size = 0;
    int maxval = 0;

    if (mpi_rank == 0 && csv_output == CSV_OUTPUT_FALSE)
        printf("Run request with EVOLUTION_WAVE of %d steps of filename=%s saving snaps each %d steps\n", number_of_steps, filename, number_of_steps_between_file_dumps);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_wave 1 BEGIN - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif

    if (mpi_rank == 0) {
        if (number_of_steps > MAX_NUMBER_OF_STEPS) {
            printf("Value %d is too big to be passed as -n <num> number of steps to be iterated, max admitted value is %d\n", number_of_steps, MAX_NUMBER_OF_STEPS);
            perror("Value is too big to be passed as -n <num> number of steps to be iterated\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        // concatenate: directory name + steps + mpi_size + timestamp
        size_t string_with_num_size = strlen("_" EVOLUTION_TYPE "_00000_000_%Y-%m-%d_%H_%M_%S");
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 3 - *alloc* *free* mpi_rank=%d/%d, char *string_with_num = malloc(string_with_num_size + 1); BEFORE\n", mpi_rank, mpi_size);
#endif
        char *string_with_num = malloc(string_with_num_size + 1);
        if (string_with_num == NULL) {
            perror("Unable to allocate buffer for string_with_num\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        sprintf(string_with_num, "_" EVOLUTION_TYPE "_%05d_%03d_%%Y-%%m-%%d_%%H_%%M_%%S", number_of_steps, mpi_size);
        size_t string_with_timestamp_size = strlen("_" EVOLUTION_TYPE "_00000_000_2023-02-13_23:37:01");
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 4 - *alloc* *free* mpi_rank=%d/%d, char *string_with_timestamp = malloc(string_with_timestamp_size + 1); BEFORE\n", mpi_rank, mpi_size);
#endif
        char *string_with_timestamp = malloc(string_with_timestamp_size + 1);
        if (string_with_timestamp == NULL) {
            perror("Unable to allocate buffer for string_with_timestamp\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        unsigned long len = strftime(string_with_timestamp, string_with_timestamp_size + 1, string_with_num, timenow);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 5 - *alloc* *free* mpi_rank=%d/%d, free(string_with_num); BEFORE\n", mpi_rank, mpi_size);
#endif
        free(string_with_num);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 6 - *alloc* *free* mpi_rank=%d/%d, free(string_with_num); AFTER\n", mpi_rank, mpi_size);
#endif
#ifdef DEBUG1
        if (debug_info > 0) {
            printf("DEBUG1 - evolution_wave 2 - rank %d/%d, len=%ld string_with_timestamp=%s\n", mpi_rank, mpi_size, len, string_with_timestamp);
            printf("DEBUG1 - evolution_wave 3 - rank %d/%d, strlen(filename) + strlen(string_with_timestamp) + 1=%lu\n", mpi_rank, mpi_size, strlen(filename) + strlen(string_with_timestamp) + 1);
        }
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 7 - *alloc* *free* mpi_rank=%d/%d, directoryname = malloc(strlen(filename) + strlen(string_with_timestamp) + 1); BEFORE\n", mpi_rank, mpi_size);
#endif
        directoryname_len = strlen(filename) + strlen(string_with_timestamp) + 1;
        directoryname = malloc(directoryname_len);
        if (directoryname == NULL) {
            perror("Unable to allocate buffer for directoryname in evolution_wave\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        strcpy(directoryname, filename);
        strcat(directoryname, string_with_timestamp);
        replace_char(directoryname, '/', '_');
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 8 - *alloc* *free* mpi_rank=%d/%d, free(string_with_timestamp); BEFORE\n", mpi_rank, mpi_size);
#endif
        free(string_with_timestamp);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 9 - *alloc* *free* mpi_rank=%d/%d, free(string_with_timestamp); AFTER\n", mpi_rank, mpi_size);
#endif
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_wave 4a - rank %d/%d MPI_Bcast BEFORE strlen(directoryname)=%lu, directoryname=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname);
#endif
        // if serial then avoid chunk files
        if (mpi_size == 1)
            partial_file_extension = file_extension_pgm;
        t_io += make_directory(directoryname, debug_info);
    }
    // Broadcast the string to all other processes
    MPI_Bcast(&directoryname_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (mpi_rank > 0)
        directoryname = malloc(directoryname_len);
    MPI_Bcast(directoryname, directoryname_len, MPI_CHAR, 0, MPI_COMM_WORLD);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_wave 4b - rank %d/%d MPI_Bcast AFTER strlen(directoryname)=%lu, directoryname=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname);
#endif
    // wait for master to broadcast message
//    MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_wave 6 - rank %d/%d directoryname=%s\n", mpi_rank, mpi_size, directoryname);
#endif

    t_io += file_pgm_read_noghost(&world_local, &maxval, &local_size, &world_size, filename, mpi_rank, mpi_size, debug_info);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_wave 7 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, filename);
#endif
    if (mpi_size > world_size || local_size == 0) {
        perror("ERROR: wrong situation with more processes than domain decomposition slices\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (mpi_size > 1)
        t_io += evolution_wave_parallel(mpi_rank, mpi_size, &mpi_status, &mpi_request, world_local, world_size, local_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
    else
        t_io += evolution_wave_single(world_local, world_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 12 - ITERATED - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, directoryname=%s, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, directoryname, filename);
#endif
    // wait for all iterations to complete
    MPI_Barrier(MPI_COMM_WORLD);
    // write final iteration output
    t_io += file_pgm_write_chunk_noghost(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, "", partial_file_extension, mpi_rank, mpi_size, debug_info);
    // all processes send io-time to process zero
    if (mpi_rank > 0) {
        MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request);
#ifdef DEBUG2
        if (debug_info > 0)
            printf("DEBUG1 - evolution_wave 8 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s, *MPI* MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request)\n"
                   , mpi_rank, mpi_size, maxval, local_size, world_size, filename);
#endif
    }

    MPI_Barrier(MPI_COMM_WORLD);
    // merge chunks of final output if needed
    if (mpi_size > 1 && mpi_rank == 0) {
        // Join the chunks into a single image file
        //    final_wave002.pgm              (final output with 2 MPI processes)
        //    final_wave002_000.pgmpart      (final output chunk 0)
        //    final_wave002_001.pgmpart      (final output chunk 1)
        //    snap_wave002_000_00001.pgmpart (snap of iteration 1 chunk 0)
        //    snap_wave002_001_00001.pgmpart (snap of iteration 1 chunk 1)
        //    snap_wave002_000_00002.pgmpart (snap 2 = final chunk 0)
        //    snap_wave002_001_00002.pgmpart (snap 2 = final chunk 1)
        //sprintf(file_name, "%s/%s%03d_%03d%s.%s", directoryname, image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
        //DEBUG2 - evolution_wave 5 - MERGE CHUNKS rank 0/2, pattern_random16.pgm_wave_2023-05-16_08_45_36/final_wave002_000.pgmpart
        // join chunks of all iteration steps
        for (int iteration_step = number_of_steps_between_file_dumps;
             iteration_step < number_of_steps; iteration_step += number_of_steps_between_file_dumps) {
#ifdef DEBUG2
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_wave 13 - rank %d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, iteration_step, number_of_steps);
                printf("DEBUG2 - evolution_wave 14 - rank %d/%d, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            }
#endif
            // filename of joined iteration snap
            char *snap_fn;
            unsigned long snap_fn_len = strlen("/_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_WAVE) + strlen(FILE_EXTENSION_PGM) + 1;
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 15 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, snap_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 16 - *alloc* *free* mpi_rank=%d/%d, snap_fn = (char *) malloc(snap_fn_len); BEFORE\n", mpi_rank, mpi_size);
#endif
            snap_fn = (char *) malloc(snap_fn_len);
            if (snap_fn == NULL) {
                perror("Unable to allocate buffer for snap_fn\n");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

//          sprintf(snap_fn, "%s/%s%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            sprintf(snap_fn, "%s/%s_%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, iteration_step, FILE_EXTENSION_PGM);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 17 - snap_fn_len=%ld, strlen(%s)=%ld\n", snap_fn_len, snap_fn, strlen(snap_fn));
#endif

            // array of filenames of snap chunks to be joined
            char *snap_chunks_fn[mpi_size];
            unsigned long snap_chunks_fn_len = strlen("/_000_000_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_WAVE) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
            if (debug_info > 1) // test chunks fn length
                printf("DEBUG2 - evolution_wave 18 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%03d_%05d%s.%s\n"
                       , mpi_rank, mpi_size, snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, mpi_rank, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
            for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_wave 19: LEN=%lu %s/%s%03d_%03d_%05d%s.%s\n"
                           , snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_wave 20 - *alloc* *free* mpi_rank=%d/%d, snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len); BEFORE\n", mpi_rank, mpi_size);
#endif
                snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len);
                if (snap_chunks_fn[i] == NULL) {
                    perror("Unable to allocate buffer for snap_chunks_fn[i]\n");
                    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
                }
                sprintf(snap_chunks_fn[i], "%s/%s_%03d_%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
//              sprintf(snap_chunks_fn[i], "%s/%s%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, iteration_step, FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_wave 21 LEN=%lu %s\n", strlen(snap_chunks_fn[i]), snap_chunks_fn[i]);
#endif
            }
#ifdef DEBUG1
            if (debug_info > 0)
                for (int i = 0; i < mpi_size; i++)
                    printf("DEBUG1 - evolution_wave 9 %s\n", snap_chunks_fn[i]);
#endif
            for (int i = 0; i < mpi_size; i++)
                t_io += file_chunk_merge(snap_fn, snap_chunks_fn[i], debug_info); // TODO: manage error result
            double t_point = MPI_Wtime();
#ifdef DEBUG1
            if (debug_info == 0)
#endif
                // delete chunks but keep them in debug mode
                for (int i = 0; i < mpi_size; i++)
                    remove(snap_chunks_fn[i]);
            t_io += MPI_Wtime() - t_point;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 22 - *alloc* *free* mpi_rank=%d/%d, free(snap_fn); BEFORE\n", mpi_rank, mpi_size);
#endif
            free(snap_fn);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_wave 23 - *alloc* *free* mpi_rank=%d/%d, free(snap_fn); AFTER\n", mpi_rank, mpi_size);
                printf("DEBUG2 - evolution_wave 24 - *alloc* *free* mpi_rank=%d/%d, free(snap_chunks_fn[i]); BEFORE\n", mpi_rank, mpi_size);
            }
#endif
            for (int i = 0; i < mpi_size; i++)
                free(snap_chunks_fn[i]); // TODO: verify
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 25 - *alloc* *free* mpi_rank=%d/%d, free(snap_chunks_fn[i]); AFTER\n", mpi_rank, mpi_size);
#endif
        }

        // join chunks of final output
        // filename of joined final output
        char *final_fn;
        unsigned long final_fn_len =
//              strlen("/000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_WAVE) +
                strlen("/.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_WAVE) + strlen(FILE_EXTENSION_PGM) + 1;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 26 - *alloc* *free* mpi_rank=%d/%d, final_fn = (char *) malloc(final_fn_len); BEFORE\n", mpi_rank, mpi_size);
#endif
        final_fn = (char *) malloc(final_fn_len);
        if (final_fn == NULL) {
            perror("Unable to allocate buffer for final_fn\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        //sprintf(final_fn, "%s/%s%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, "", FILE_EXTENSION_PGM);
        sprintf(final_fn, "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, FILE_EXTENSION_PGM);
#ifdef DEBUG2
        if (debug_info > 1) {
            // NB: strlen=string length, sizeof=length+1 one more char for the terminating zero
            printf("DEBUG2 - evolution_wave 27 - final_fn_len=%ld, strlen(%s)=%ld\n", final_fn_len, final_fn, strlen(final_fn));
            printf("DEBUG2 - evolution_wave 28 - strlen(\"/000_000.\")=%ld\n", strlen("/000_000."));
            printf("DEBUG2 - evolution_wave 29 - strlen(%s)=%ld\n", directoryname, strlen(directoryname));
            printf("DEBUG2 - evolution_wave 30 - sizeof(%s)=%ld\n", IMAGE_FILENAME_PREFIX_FINAL_WAVE, sizeof(IMAGE_FILENAME_PREFIX_FINAL_WAVE));
            printf("DEBUG2 - evolution_wave 31 - sizeof(%s)=%ld\n", FILE_EXTENSION_PGMPART, sizeof(FILE_EXTENSION_PGMPART));
        }
#endif
        // array of filenames of chunks to be joined
        char *final_chunks_fn[mpi_size];
        unsigned long final_chunks_fn_len = strlen("/_000_000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_WAVE) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
        if (debug_info > 1) // test chunks fn length
            printf("DEBUG2 - evolution_wave 32 - MERGE CHUNKS FINAL rank %d/%d, LEN=%lu, final_chunks_fn=%s/%s%03d_%03d%s.%s\n"
                   , mpi_rank, mpi_size, final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, mpi_rank, "", FILE_EXTENSION_PGMPART);
#endif
        for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 33 - JOIN1a: LEN=%lu %s/%s%03d_%03d%s.%s\n"
                       , final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 34- *alloc* *free* mpi_rank=%d/%d, final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len); BEFORE\n", mpi_rank, mpi_size);
#endif
            final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len);
            if (final_chunks_fn[i] == NULL) {
                perror("Unable to allocate buffer for final_chunks_fn[i]\n");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
            sprintf(final_chunks_fn[i], "%s/%s_%03d_%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, i, "", FILE_EXTENSION_PGMPART);
//          sprintf(final_chunks_fn[i], "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 35 - JOIN1b: LEN=%lu %s\n", strlen(final_chunks_fn[i]), final_chunks_fn[i]);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - evolution_wave 10 - JOIN2: %s\n", final_chunks_fn[i]);
#endif
        // delete if already existing, unnecessary as we create a new dir
        //int remove_result = remove(pathname);
        //if (debug_info > 0)
        //    printf("DEBUG1 - evolution_wave 11 - remove_result: %d\n", remove_result);
        for (int i = 0; i < mpi_size; i++)
            t_io += file_chunk_merge(final_fn, final_chunks_fn[i], debug_info); // TODO: manage error result
        double t_point = MPI_Wtime();
#ifdef DEBUG1
        if (debug_info == 0)
#endif
            // delete chunks but keep them in debug mode
            for (int i = 0; i < mpi_size; i++)
                remove(final_chunks_fn[i]);
        t_io += MPI_Wtime() - t_point;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 36 - *alloc* *free* mpi_rank=%d/%d, free(final_fn); BEFORE\n", mpi_rank, mpi_size);
#endif
        free(final_fn);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1) {
            printf("DEBUG2 - evolution_wave 37 - *alloc* *free* mpi_rank=%d/%d, free(final_fn); AFTER\n", mpi_rank, mpi_size);
            printf("DEBUG2 - evolution_wave 38- *alloc* *free* mpi_rank=%d/%d, free(final_chunks_fn[i]); BEFORE\n", mpi_rank, mpi_size);
        }
#endif
        for (int i = 0; i < mpi_size; i++)
            free(final_chunks_fn[i]); // TODO: verify
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - evolution_wave 39 - *alloc* *free* mpi_rank=%d/%d, free(final_chunks_fn[i]); AFTER\n", mpi_rank, mpi_size);
#endif
        double t_io_other = 0;
        for (int i = 1; i < mpi_size; i++) {
#ifdef DEBUG2
            if (debug_info > 0)
                printf("DEBUG1 - evolution_wave 12 ACCU0 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s, *MPI* BEFORE MPI_Recv(&t_io_other, 1, MPI_DOUBLE, i=%d, TAG_T, MPI_COMM_WORLD, &mpi_status)\n", mpi_rank, mpi_size, maxval
                       , local_size, world_size, filename, i);
#endif
            MPI_Recv(&t_io_other, 1, MPI_DOUBLE, i, TAG_T, MPI_COMM_WORLD, &mpi_status);
            t_io_accumulator += t_io_other;
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_wave 40 ACCU1 - i=%d, t_io_other=%f, t_io_accumulator=%f\n", i, t_io_other, t_io_accumulator);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_wave 13 ACCU2 - t_io=%f, t_io_other=%f, t_io_accumulator=%f, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1)=%f\n", t_io, t_io_other, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#endif
    }
    MPI_Barrier(MPI_COMM_WORLD);

#ifdef DEBUG1
    if (mpi_rank == 0 && debug_info > 0)
        //DEBUG1 - evolution_wave 6 - mpi=2, omp=2, time taken=0.007455
        printf("DEBUG1 - evolution_wave 14 - mpi=%d, omp=%d, time taken=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 41 - *alloc* *free* mpi_rank=%d/%d, free(directoryname=%p); BEFORE\n", mpi_rank, mpi_size, directoryname);
#endif
    free(directoryname);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 42 - *alloc* *free* mpi_rank=%d/%d, free(directoryname); AFTER\n", mpi_rank, mpi_size);
#endif
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 43 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 44 - *alloc* *free* mpi_rank=%d/%d, free(world_local=%p); BEFORE\n", mpi_rank, mpi_size, world_local);
#endif
    free(world_local);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 45 - *alloc* *free* mpi_rank=%d/%d, free(world_local); AFTER\n", mpi_rank, mpi_size);
#endif
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 46 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
    if (mpi_rank == 0)
        if (csv_output == CSV_OUTPUT_FALSE)
            printf("mpi=%d, omp=%d, total time=%f, I/O time=%f, I/O time t_io_accumulator=%f, t_io_accumulator mean=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator,
                    mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
        else
            printf("e2,%ld,%d,%d,%d,%d,%f,%f,%f,%f\n", world_size, number_of_steps, number_of_steps_between_file_dumps, mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_wave 15 END - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - evolution_wave 47 - BEFORE MPI_Finalize()\n");
#endif
    MPI_Finalize();
}
