#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "files_io.h"
#include "gameoflife.h"

#define EVOLUTION_TYPE "ordered"
#define IMAGE_FILENAME_PREFIX_SNAP_ORDERED "snapshot"
#define IMAGE_FILENAME_PREFIX_FINAL_ORDERED "final"

void set_dead_or_alive_ordered_parallel(unsigned char *world_local, long world_size, long local_size) {
    //printf("DEBUGB - set_dead_or_alive_ordered_parallel 2 - world_size=%ld local_size=%ld\n", world_size, local_size);
    // TODO: declare vars before loops to optimize performance
    for (long long i = world_size; i < world_size * (local_size + 1); i++) {
        // actual cell coordinates
        long x = i % world_size;
        long y = i / world_size;
        // neighbours of actual cell
        long x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
        long x_next = x + 1 < world_size ? x + 1 : 0;
        long y_prev = y - 1;
        long y_next = y + 1;
        // determine the number of dead neighbours
        int sum = world_local[y_prev * world_size + x_prev] +
                  world_local[y_prev * world_size + x] +
                  world_local[y_prev * world_size + x_next] +
                  world_local[y * world_size + x_prev] +
                  world_local[y * world_size + x_next] +
                  world_local[y_next * world_size + x_prev] +
                  world_local[y_next * world_size + x] +
                  world_local[y_next * world_size + x_next];
        int number_of_dead_neighbours = sum / DEAD;
        // Update cell
        if (world_local[y * world_size + x] == ALIVE // if actual cell is alive
            && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
            // do nothing world_local[i] = ALIVE;
        } else if (world_local[y * world_size + x] == DEAD // if actual cell is dead
                   && (number_of_dead_neighbours == 5))
            world_local[i] = ALIVE;
        else // default is: cell will die
            world_local[i] = DEAD;
    }
}

void set_dead_or_alive_ordered_single(unsigned char *world, long world_size) {
    // Fill the first row of the local matrix with the last row of the world (ghost)
    for (long i = 0; i < world_size; i++)
        world[i] = world[world_size * world_size + i];

    // TODO: declare vars before loops to optimize performance
    for (long long i = world_size; i < world_size * (world_size + 1); i++) {
        // actual cell coordinates
        long x = i % world_size;
        long y = i / world_size;
        // neighbours of actual cell
        long x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
        long x_next = x + 1 < world_size ? x + 1 : 0;
        long y_prev = y - 1;
        long y_next = y + 1;
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
        // Update cell
        if (world[y * world_size + x] == ALIVE // if actual cell is alive
            && (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)) {
            // do nothing world_local[i] = ALIVE;
        } else if (world[y * world_size + x] == DEAD // if actual cell is dead
                   && (number_of_dead_neighbours == 5))
            world[i] = ALIVE;
        else // default is: cell will die
            world[i] = DEAD;

        // After the update of the first row, copy updated values on the last row of the entire matrix
        if (i == world_size * 2) {
            for (long j = 0; j < world_size; j++) {
                world[world_size * (world_size + 1) + j] = world[world_size + j];
            }
        }

    }

}

double evolution_ordered_parallel(int mpi_rank, int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char *world_local, long world_size, long local_size
                                  , int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    // TODO: better filenames
    char *image_filename_suffix = (char *) malloc(60);
    // before cycling the iterations, send the needed first ghost row
    if (mpi_rank == mpi_size - 1) {
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_ordered_parallel 00 SEND iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", -1, mpi_rank, TAG_0, TAG_1);
#endif
        // last chunk process (mpi_size - 1): send last row to first chunk process (0)
        MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 0, TAG_X, MPI_COMM_WORLD, mpi_request);
    }

    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
        if (mpi_rank != 0) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 0 SEND iteration=%d rank=%d, TAG_0=%d, TAG_1=%d, last row to %d with TAG_1\n", iteration_step, mpi_rank, TAG_0, TAG_1, mpi_rank - 1);
#endif
            // all chunks except first: send first row to previous chunk
            MPI_Isend(&world_local[world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, TAG_1, MPI_COMM_WORLD, mpi_request);
            //MPI_Isend(&world_local[world_size], world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, TAG_1, MPI_COMM_WORLD, mpi_request);
        }
        if (mpi_rank == mpi_size - 1) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 1 WAIT iteration=%d rank=%d, TAG_0=%d, TAG_1=%d, first ghost row from %d with TAG_1\n", iteration_step, mpi_rank, TAG_0, TAG_1, mpi_rank - 1);
#endif
            // last chunk process (mpi_size - 1): wait to receive first ghost row from previous process
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, TAG_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 1 REC1 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
            // and last ghost row from chunk zero
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 0, TAG_X, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 1 REC2 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
        }
        if (mpi_rank == 0) {
#ifdef DEBUG2
            //            if (debug_info > 1) {
            //                long pos = local_size * world_size;
            //                printf("DEBUG2 - evolution_ordered_parallel 1SND - rank %d/%d, iteration_step=%d, world_local[0-31]=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
            //                       , mpi_rank, mpi_size, iteration_step
            //                       , world_local[pos], world_local[pos + 1], world_local[pos + 2], world_local[pos + 3]
            //                       , world_local[pos + 4], world_local[pos + 5], world_local[pos + 6], world_local[pos + 7]
            //                       , world_local[pos + 8], world_local[pos + 9], world_local[pos + 10], world_local[pos + 11]
            //                       , world_local[pos + 12], world_local[pos + 13], world_local[pos + 14], world_local[pos + 15]
            //                       , world_local[pos + 16], world_local[pos + 17], world_local[pos + 18], world_local[pos + 19]
            //                       , world_local[pos + 20], world_local[pos + 21], world_local[pos + 22], world_local[pos + 23]
            //                       , world_local[pos + 24], world_local[pos + 25], world_local[pos + 26], world_local[pos + 27]
            //                       , world_local[pos + 28], world_local[pos + 29], world_local[pos + 30], world_local[pos + 31]
            //                );
            //            }
                        if (debug_info > 1)
                            printf("DEBUG2 - evolution_ordered_parallel 2 WAIT iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
            // first chunk: wait to receive first ghost row from last chunk and last ghost row from chunk 1
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_size - 1, TAG_X, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_ordered_parallel 2 REC1 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
                printf("DEBUG2 - evolution_ordered_parallel 2 WAIT2 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d, last ghost row from %d with TAG_1\n", iteration_step, mpi_rank, TAG_0, TAG_1, 1);
            }
#endif
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 1, TAG_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 2 REC2 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
        }
        if (mpi_rank != 0 && mpi_rank != mpi_size - 1) {
            // middle chunk: wait to receive first ghost row from previous chunk and last ghost row from next chunk
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 3 WAIT iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, TAG_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 3 REC1 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank + 1, TAG_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 3 REC2 iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
//            if (debug_info > 1)
//                printf("DEBUG2 - evolution_ordered_parallel 1RCD - rank %d/%d, iteration_step=%d, world_local[0-31]=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
//                       , mpi_rank, mpi_size, iteration_step
//                       , world_local[0], world_local[1], world_local[2], world_local[3]
//                       , world_local[4], world_local[5], world_local[6], world_local[7]
//                       , world_local[8], world_local[9], world_local[10], world_local[11]
//                       , world_local[12], world_local[13], world_local[14], world_local[15]
//                       , world_local[16], world_local[17], world_local[18], world_local[19]
//                       , world_local[20], world_local[21], world_local[22], world_local[23]
//                       , world_local[24], world_local[25], world_local[26], world_local[27]
//                       , world_local[28], world_local[29], world_local[30], world_local[31]
//                       );
#endif
        }

#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_ordered_parallel BEFORE UPDATE iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
        set_dead_or_alive_ordered_parallel(world_local, world_size, local_size);
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_ordered_parallel AFTER UPDATE iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif

        if (mpi_rank == 0) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered_parallel 4 SEND iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
#endif
            // last row to next chunk process 0+1=1
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 1, TAG_1, MPI_COMM_WORLD, mpi_request);
            // first chunk: process 0 sends its first row to the last chunk process mpi_size-1
            MPI_Isend(&world_local[world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_size - 1, TAG_X, MPI_COMM_WORLD, mpi_request);
        }
        if (mpi_rank != 0 && mpi_rank != mpi_size - 1)
            // middle chunk: send last row to next chunk
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank + 1, TAG_1, MPI_COMM_WORLD, mpi_request);
        if (mpi_rank == mpi_size - 1 && iteration_step != number_of_steps) { // skip last iteration, no update needed
            // last chunk: send last row to first chunk zero
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 0, TAG_X, MPI_COMM_WORLD, mpi_request);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (iteration_step % number_of_steps_between_file_dumps == 0) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            t_io += file_pgm_write_chunk(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
        }
#ifdef DEBUG2
        if (debug_info > 1) printf("DEBUG2 - evolution_ordered_parallel end iteration cycle\n");
#endif
    }
#ifdef DEBUG2
    if (debug_info > 1) printf("DEBUG2 - evolution_ordered_parallel end iteration\n");
#endif

    free(image_filename_suffix);
    return t_io;
}

double evolution_ordered_single(unsigned char *world, long world_size, int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    char *image_filename_prefix = (char *) malloc(60);
    sprintf(image_filename_prefix, IMAGE_FILENAME_PREFIX_SNAP_ORDERED);
    char *image_filename_suffix = (char *) malloc(60);

    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
        set_dead_or_alive_ordered_single(world, world_size);
        if (iteration_step % number_of_steps_between_file_dumps == 0) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            t_io += file_pgm_write_chunk(world, 255, world_size, world_size, directoryname, image_filename_prefix, image_filename_suffix, FILE_EXTENSION_PGM, 0, 1, debug_info);
        }
    }
    free(image_filename_prefix);
    free(image_filename_suffix);
    return t_io;
}

void evolution_ordered(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info) {
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    int mpi_provided_thread_level;
    MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        perror("a problem occurred asking for MPI_THREAD_FUNNELED level\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    int mpi_rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    double t_io = 0; // total I/O time spent
    double t_io_accumulator = 0; // total I/O time spent by processes > 0
    double t_start = MPI_Wtime(); // start time
    char *directoryname;
    int directoryname_len = 0;
    const char *file_extension_pgm = FILE_EXTENSION_PGM;
    const char *file_extension_pgmpart = FILE_EXTENSION_PGMPART;
    const char *partial_file_extension = file_extension_pgmpart; // default is partial chunk
    // chunk of the world_local for each MPI process
    unsigned char *world_local;
    long world_size = 0;
    long local_size = 0;
    int maxval = 0;

    if (mpi_rank == 0 && csv_output == CSV_OUTPUT_FALSE)
        printf("Run request with EVOLUTION_ORDERED of %d steps of filename=%s saving snaps each %d steps\n", number_of_steps, filename, number_of_steps_between_file_dumps);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_ordered BEGIN - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif

    if (mpi_rank == 0) {
        if (number_of_steps > MAX_NUMBER_OF_STEPS) {
            printf("Value %d is too big to be passed as -n <num> number of steps to be iterated, max admitted value is %d\n", number_of_steps, MAX_NUMBER_OF_STEPS);
            perror("Value is too big to be passed as -n <num> number of steps to be iterated\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        // concatenate: directory name + steps + mpi_size + timestamp
        size_t string_with_num_size = strlen("_" EVOLUTION_TYPE "_00000_000_%Y-%m-%d_%H_%M_%S");
        char *string_with_num = malloc(string_with_num_size + 1);
        if (string_with_num == NULL) {
            perror("Unable to allocate buffer for string_with_num\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        sprintf(string_with_num, "_" EVOLUTION_TYPE "_%05d_%03d_%%Y-%%m-%%d_%%H_%%M_%%S", number_of_steps, mpi_size);
        size_t string_with_timestamp_size = strlen("_" EVOLUTION_TYPE "_00000_000_2023-02-13_23:37:01");
        char *string_with_timestamp = malloc(string_with_timestamp_size + 1);
        if (string_with_timestamp == NULL) {
            perror("Unable to allocate buffer for string_with_timestamp\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        unsigned long len = strftime(string_with_timestamp, string_with_timestamp_size + 1, string_with_num, timenow);
        free(string_with_num);
#ifdef DEBUG1
        if (debug_info > 0) {
            printf("DEBUG1 - evolution_ordered 1 - rank %d/%d, len=%ld string_with_timestamp=%s\n", mpi_rank, mpi_size, len, string_with_timestamp);
            printf("DEBUG1 - evolution_ordered 1a - rank %d/%d, strlen(filename) + strlen(string_with_timestamp) + 1=%lu\n", mpi_rank, mpi_size, strlen(filename) + strlen(string_with_timestamp) + 1);
        }
#endif
        directoryname_len = strlen(filename) + strlen(string_with_timestamp) + 1;
        directoryname = malloc(directoryname_len);
        if (directoryname == NULL) {
            perror("Unable to allocate buffer for directoryname in evolution_ordered\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        strcpy(directoryname, filename);
        strcat(directoryname, string_with_timestamp);
        replace_char(directoryname, '/', '_');
        free(string_with_timestamp);
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_ordered 1b - rank %d/%d, strlen(directoryname)=%lu, directoryname=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname);
#endif
        // if serial then avoid chunk files
        if (mpi_size == 0)
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
        printf("DEBUG1 - evolution_ordered 2 - rank %d/%d directoryname=%s\n", mpi_rank, mpi_size, directoryname);
#endif

    /* Read the local data chunk of the world from file:
      - Calculate the number of local rows
      - Allocate memory for the local world plus 2 "ghost" rows (world_size*(local_rows+2)).
        The first row is used to store the last row of the previous thread (mpi_size-1 if mpi_rank == 0)
        and the last row is used to store the first row of the next thread process.
        In the case of a single MPI Task the first row will store the last row
        of the world and the last row will store the first row of the world
      - Read the values: the first value is stored in world_local[world_size]
    */
    t_io += file_pgm_read(&world_local, &maxval, &local_size, &world_size, filename, mpi_rank, mpi_size, debug_info);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_ordered 3 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, filename);
#endif
    if (mpi_size > world_size || local_size == 0) {
        perror("ERROR: wrong situation with more processes than domain decomposition slices\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (mpi_size > 1)
        t_io += evolution_ordered_parallel(mpi_rank, mpi_size, &mpi_status, &mpi_request, world_local, world_size, local_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
    else
        t_io += evolution_ordered_single(world_local, world_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_ordered 4 - ITERATED - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, directoryname=%s, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, directoryname, filename);
#endif
    // wait for all iterations to complete
    MPI_Barrier(MPI_COMM_WORLD);
    // write final iteration output
    t_io += file_pgm_write_chunk(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, "", partial_file_extension, mpi_rank, mpi_size, debug_info);
    // all processes send io-time to process zero
    if (mpi_rank > 0)
        MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request);

    MPI_Barrier(MPI_COMM_WORLD);
    // merge chunks of final output if needed
    if (mpi_size > 1 && mpi_rank == 0) {
        // Join the chunks into a single image file
        //    final_ordered002.pgm              (final output with 2 MPI processes)
        //    final_ordered002_000.pgmpart      (final output chunk 0)
        //    final_ordered002_001.pgmpart      (final output chunk 1)
        //    snap_ordered002_000_00001.pgmpart (snap of iteration 1 chunk 0)
        //    snap_ordered002_001_00001.pgmpart (snap of iteration 1 chunk 1)
        //    snap_ordered002_000_00002.pgmpart (snap 2 = final chunk 0)
        //    snap_ordered002_001_00002.pgmpart (snap 2 = final chunk 1)
        //sprintf(file_name, "%s/%s%03d_%03d%s.%s", directoryname, image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
        //DEBUG2 - evolution_ordered 5 - MERGE CHUNKS rank 0/2, pattern_random16.pgm_ordered_2023-05-16_08_45_36/final_ordered002_000.pgmpart
        // join chunks of all iteration steps
        for (int iteration_step = number_of_steps_between_file_dumps;
             iteration_step <= number_of_steps; iteration_step += number_of_steps_between_file_dumps) {
#ifdef DEBUG2
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_ordered 5a0 - rank %d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, iteration_step, number_of_steps);
                printf("DEBUG2 - evolution_ordered 5a1 - rank %d/%d, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n"
                       , mpi_rank, mpi_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            }
#endif
            // filename of joined iteration snap
            char *snap_fn;
            unsigned long snap_fn_len = strlen("/_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_ORDERED) + strlen(FILE_EXTENSION_PGM) + 1;
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered 5a2 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, snap_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
#endif
            snap_fn = (char *) malloc(snap_fn_len);
//          sprintf(snap_fn, "%s/%s%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            sprintf(snap_fn, "%s/%s_%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, iteration_step, FILE_EXTENSION_PGM);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered 5a3 - snap_fn_len=%ld, strlen(%s)=%ld\n", snap_fn_len, snap_fn, strlen(snap_fn));
#endif

            // array of filenames of snap chunks to be joined
            char *snap_chunks_fn[mpi_size];
            unsigned long snap_chunks_fn_len = strlen("/_000_000_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_ORDERED) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
            if (debug_info > 1) // test chunks fn length
                printf("DEBUG2 - evolution_ordered 5a4 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%03d_%05d%s.%s\n"
                       , mpi_rank, mpi_size, snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, mpi_rank, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
            for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_ordered 5a5: LEN=%lu %s/%s%03d_%03d_%05d%s.%s\n"
                           , snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
                snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len);
                sprintf(snap_chunks_fn[i], "%s/%s_%03d_%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
//              sprintf(snap_chunks_fn[i], "%s/%s%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_ORDERED, iteration_step, FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_ordered 5a6: LEN=%lu %s\n", strlen(snap_chunks_fn[i]), snap_chunks_fn[i]);
#endif
            }
#ifdef DEBUG1
            if (debug_info > 0)
                for (int i = 0; i < mpi_size; i++)
                    printf("DEBUG1 - evolution_ordered 5a7: %s\n", snap_chunks_fn[i]);
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
            free(snap_fn);
            for (int i = 0; i < mpi_size; i++)
                free(snap_chunks_fn[i]); // TODO: verify
        }

        // join chunks of final output
        // filename of joined final output
        char *final_fn;
        unsigned long final_fn_len =
                strlen("/.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_ORDERED) +
                strlen(FILE_EXTENSION_PGM) + 1;
        final_fn = (char *) malloc(final_fn_len);
//      sprintf(final_fn, "%s/%s%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, mpi_size, "", FILE_EXTENSION_PGM);
        sprintf(final_fn, "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, FILE_EXTENSION_PGM);
#ifdef DEBUG2
        if (debug_info > 1) {
            // NB: strlen=string length, sizeof=length+1 one more char for the terminating zero
            printf("DEBUG2 - evolution_ordered 5b0 - final_fn_len=%ld, strlen(%s)=%ld\n", final_fn_len, final_fn, strlen(final_fn));
            printf("DEBUG2 - evolution_ordered 5b1 - strlen(\"/000_000.\")=%ld\n", strlen("/000_000."));
            printf("DEBUG2 - evolution_ordered 5b2 - strlen(%s)=%ld\n", directoryname, strlen(directoryname));
            printf("DEBUG2 - evolution_ordered 5b3 - sizeof(%s)=%ld\n", IMAGE_FILENAME_PREFIX_FINAL_ORDERED, sizeof(IMAGE_FILENAME_PREFIX_FINAL_ORDERED));
            printf("DEBUG2 - evolution_ordered 5b4 - sizeof(%s)=%ld\n", FILE_EXTENSION_PGMPART, sizeof(FILE_EXTENSION_PGMPART));
        }
#endif
        // array of filenames of chunks to be joined
        char *final_chunks_fn[mpi_size];
        unsigned long final_chunks_fn_len = strlen("/_000_000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_ORDERED) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
        if (debug_info > 1) // test chunks fn length
            printf("DEBUG2 - evolution_ordered 5 - MERGE CHUNKS FINAL rank %d/%d, LEN=%lu, final_chunks_fn=%s/%s%03d_%03d%s.%s\n"
                   , mpi_rank, mpi_size, final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, mpi_size, mpi_rank, "", FILE_EXTENSION_PGMPART);
#endif
        for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered - JOIN1a: LEN=%lu %s/%s_%03d_%03d%s.%s\n"
                       , final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#endif
            final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len);
            sprintf(final_chunks_fn[i], "%s/%s_%03d_%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_ORDERED, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered - JOIN1b: LEN=%lu %s\n", strlen(final_chunks_fn[i]), final_chunks_fn[i]);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - evolution_ordered - JOIN2: %s\n", final_chunks_fn[i]);
#endif
        // delete if already existing, unnecessary as we create a new dir
        //int remove_result = remove(pathname);
        //if (debug_info > 0)
        //    printf("DEBUG1 - evolution_ordered - remove_result: %d\n", remove_result);
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
        free(final_fn);
        for (int i = 0; i < mpi_size; i++)
            free(final_chunks_fn[i]); // TODO: verify
        double t_io_other = 0;
        for (int i = 1; i < mpi_size; i++) {
            MPI_Recv(&t_io_other, 1, MPI_DOUBLE, i, TAG_T, MPI_COMM_WORLD, &mpi_status);
            t_io_accumulator += t_io_other;
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_ordered ACCU1 - i=%d, t_io_other=%f, t_io_accumulator=%f\n", i, t_io_other, t_io_accumulator);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_ordered ACCU2 - t_io=%f, t_io_other=%f, t_io_accumulator=%f, t_io_accumulator / (mpi_size - 1)=%f\n", t_io, t_io_other, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#endif
    }

#ifdef DEBUG1
    if (mpi_rank == 0 && debug_info > 0)
        //DEBUG1 - evolution_ordered 6 - mpi=2, omp=2, time taken=0.007455
        printf("DEBUG1 - evolution_ordered 6 - mpi=%d, omp=%d, time taken=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start);
#endif

    MPI_Finalize();
    free(directoryname);

#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_ordered 7 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif

    free(world_local);

#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_ordered 8 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
    if (mpi_rank == 0)
        if (csv_output == CSV_OUTPUT_FALSE)
            printf("mpi=%d, omp=%d, total time=%f, I/O time=%f, I/O time t_io_accumulator=%f, t_io_accumulator mean=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator,
                    mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
        else
            printf("%d,%d,%f,%f,%f,%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_ordered END - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
}
