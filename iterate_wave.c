#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "read_write_pgm.h"
#include "gameoflife.h"

#define EVOLUTION_TYPE "wave"
#define IMAGE_FILENAME_PREFIX_SNAP_WAVE "snapshot"
#define IMAGE_FILENAME_PREFIX_FINAL_WAVE "final"

//#define DEBUG_ADVANCED_MALLOC_FREE
#define DEBUG_ADVANCED_COORDINATES

void update_wave_serial(unsigned char *world, long world_size, int iteration_step, long startX, long startY, int debug_info) {
    // iterate the square wave from 1 cell to squares of size 3, 5, 7... up to world size
    // TODO: manage joined borders of the world when the world size is even
    //       updating of biggest square would lead to multiple cell updating
    for (long square_size = 1; square_size <= world_size; square_size += 2) {
        if (debug_info > 1)
            printf("DEBUG2 - update_wave_serial 0 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld\n", iteration_step, startX, startY, square_size);
        // integer division: 1/2=0 3/2=1 5/2=2 7/2=3...
        long half_size = square_size / 2;
        long x, y; // absolute coordinates of cell to be updated
        long x_prev, x_next, y_prev, y_next; // coords of neighbours
        long array_index;
        // horizontal and vertical iterations from -half_size to half_size
        // 1:1, 3:-1 0 1, 5: -2 -1 0 1 2,...
        // 1: start from upper left to upper right (constant y)
        y = (startY + half_size) % world_size; // cross boundary if exceeding
        // upper horizontal line from left to right
        for (long i = startX - half_size; i <= startX + half_size; i++) {
            // actual cell x coordinate, crossing boundary if exceeding
            x = (i + world_size) % world_size;
//            if (debug_info > 1)
//                printf("DEBUG2 - update_wave_serial 00 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld\n", iteration_step, startX, startY, square_size, i, x);
            // neighbours of actual cell
            x_prev = x - 1 >= 0 ? x - 1 : world_size - 1;
            x_next = x + 1 < world_size ? x + 1 : 0;
            y_prev = y - 1;
            y_next = y + 1;
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
                printf("DEBUG2 - update_wave_serial 1 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, i, x, y, array_index);
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
                y_prev = y - 1;
                y_next = y + 1;
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
                    printf("DEBUG2 - update_wave_serial 2 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, j=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, j, x, y, array_index);
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
                y_prev = y - 1;
                y_next = y + 1;
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
                    printf("DEBUG2 - update_wave_serial 3 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, i=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, i, x, y, array_index);
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
                y_prev = y - 1;
                y_next = y + 1;
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
                    printf("DEBUG2 - update_wave_serial 4 - iteration_step=%d, startX=%ld, startY=%ld, square_size=%ld, j=%ld, x=%ld, y=%ld, array_index=%ld\n", iteration_step, startX, startY, square_size, j, x, y, array_index);
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

double iterate_wave_parallel(int mpi_rank, int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char *world_local, long world_size, long local_size, int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname
                             , int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_parallel - char *image_filename_suffix = (char *) malloc(60); free(); BEFORE\n");
#endif
    // TODO: calculate exact string length instead of malloc(60)
    char *image_filename_suffix = (char *) malloc(60);
    // iterations has to be done by one single process
    // so make process 0 gather all data from the others

    if (mpi_rank != 0) {
        // other processes send matrix to process 0
        MPI_Isend(world_local, (int) (local_size * world_size), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, mpi_request);
        if (debug_info > 1)
            printf("DEBUG2 - iterate_wave_parallel - MPI_Isend mpi_rank=%d, local_size=%ld, world_size=%ld\n", mpi_rank, local_size, world_size);
        for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
            if (debug_info > 1)
                printf("DEBUG2 - iterate_wave_parallel BEFORE UPDATE iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
            MPI_Recv(world_local, (int) (local_size * world_size), MPI_UNSIGNED_CHAR, 0, iteration_step, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (iteration_step % number_of_steps_between_file_dumps == 0) {
                sprintf(image_filename_suffix, "_%05d", iteration_step);
                t_io += file_pgm_write_chunk_noghost(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
            }
        }
    } else {
        // process 0 allocates the full matrix and receives data from all other processes
        unsigned char *world = (unsigned char *) malloc(world_size * world_size * sizeof(unsigned char));
        // copy values from process zero matrix
        for (long long i = 0; i < local_size * world_size; i++)
            world[i] = world_local[i];
        // receive all other matrixes data
        long size = world_size / mpi_size;
        for (int i = 1; i < mpi_size; i++) {
            MPI_Recv(&world[i * local_size * world_size], (int) (size * world_size), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (debug_info > 1) {
                long start = i * local_size * world_size;
                printf("DEBUG2 - iterate_wave_parallel - MPI_Recv i=%d, local_size=%ld, size=%ld, world_size=%ld, start=%ld\n", i, local_size, size, world_size, start);
                for (long long j = start; j < start + size * world_size; j++)
                    printf("DEBUG2 - iterate_wave_parallel 0 world[%lld]=%d\n", j, world[j]);
            }
        }
        if (debug_info > 1) {
            for (long long i = 0; i < world_size * world_size; i++)
                printf("DEBUG2 - iterate_wave_parallel 00 world[%lld]=%d\n", i, world[i]);
        }
        // save initial matrix copy as snapshot zero, we force mpi_size=1 to obtain a clean filename snapshot_00000
        t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, "snapshot_00000", "", FILE_EXTENSION_PGM, 0, 1, debug_info);

        // TODO: start from random point, at each iteration or all of them (pass coordinates to the function)
        // for testing purposes we want to be able to start from a given point to compare results
        time_t t;
        srand((unsigned) time(&t));
        long x = rand() % world_size, y = rand() % world_size;
        if (debug_info > 1) {
            x = 0;
            y = 0;
        }
        if (debug_info > 0)
            printf("DEBUG2 - iterate_wave_parallel start point x=%ld, y=%ld END\n", x, y);
        // all processes do the iterations, process 0 does the update and sends results
        for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
            if (debug_info > 1)
                printf("DEBUG2 - iterate_wave_parallel BEFORE UPDATE iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
            // square wave update, only serial
            update_wave_serial(world, world_size, iteration_step, x, y, debug_info);
            if (debug_info > 1)
                printf("DEBUG2 - iterate_wave_parallel AFTER UPDATE iteration=%d rank=%d, TAG_0=%d, TAG_1=%d\n", iteration_step, mpi_rank, TAG_0, TAG_1);
            // TODO: send back to all processes for parallel writing
            for (int i = 1; i < mpi_size; i++)
                MPI_Isend(&world[i * local_size * world_size], (int) (size * world_size), MPI_UNSIGNED_CHAR, i, iteration_step, MPI_COMM_WORLD, mpi_request);
            //MPI_Barrier(MPI_COMM_WORLD);
            if (iteration_step % number_of_steps_between_file_dumps == 0) {
                sprintf(image_filename_suffix, "_%05d", iteration_step);
                t_io += file_pgm_write_chunk_noghost(world, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
            }
            if (debug_info > 1) printf("DEBUG2 - iterate_wave_parallel end iteration cycle\n");
        }
        // copy values back to set final result
        for (long long i = 0; i < local_size * world_size; i++)
            world_local[i] = world[i];
    } // process 0
    if (debug_info > 1) printf("DEBUG2 - iterate_wave_parallel iteration END\n");
    //free(image_filename_prefix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_parallel - free(image_filename_suffix); BEFORE\n");
#endif
    free(image_filename_suffix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_parallel - free(image_filename_suffix); AFTER\n");
#endif
    return t_io;
}

double iterate_wave_serial(unsigned char *world, long world_size, int number_of_steps, int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_serial - char *image_filename_prefix = (char *) malloc(60); free(); BEFORE\n");
#endif
    // TODO: calculate exact string length instead of malloc(60)
    char *image_filename_prefix = (char *) malloc(60);
    sprintf(image_filename_prefix, IMAGE_FILENAME_PREFIX_SNAP_WAVE);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_serial - char *image_filename_suffix = (char *) malloc(60); free(); BEFORE\n");
#endif
    // save initial matrix copy as snapshot zero
    t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, "snapshot_00000", "", FILE_EXTENSION_PGM, 0, 1, debug_info);

    char *image_filename_suffix = (char *) malloc(60);
    // TODO: start from random point, at each iteration or all of them (pass coordinates to the function)
    // for testing purposes we want to be able to start from a given point to compare results
    // rand() % SIZE;
    time_t t;
    srand((unsigned) time(&t));
    long x = rand() % world_size, y = rand() % world_size;
    if (debug_info > 1) {
        x = 0;
        y = 0;
    }
    if (debug_info > 0)
        printf("DEBUG2 - iterate_wave_serial start point x=%ld, y=%ld END\n", x, y);
    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
        update_wave_serial(world, world_size, iteration_step, x, y, debug_info);
        if (iteration_step % number_of_steps_between_file_dumps == 0) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            t_io += file_pgm_write_chunk_noghost(world, 255, world_size, world_size, directoryname, image_filename_prefix, image_filename_suffix, FILE_EXTENSION_PGM, 0, 1, debug_info);
        }
    }
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_serial - free(image_filename_prefix); BEFORE\n");
#endif
    free(image_filename_prefix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1) {
        printf("DEBUG2 - iterate_wave_serial - free(image_filename_prefix); AFTER\n");
        printf("DEBUG2 - iterate_wave_serial - free(image_filename_suffix); BEFORE\n");
    }
#endif
    free(image_filename_suffix);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - iterate_wave_serial - free(image_filename_suffix); AFTER\n");
#endif
    return t_io;
}

void run_wave(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int debug_info) {
    double t_io = 0; // total I/O time spent
    double t_io_accumulator = 0; // total I/O time spent by processes > 0
    double t_start = MPI_Wtime(); // start time
// TODO: compute the correct size for MPI message allocation
#define MAX_STRING_LENGTH 256
    char message[MAX_STRING_LENGTH];
    char *directoryname;
    const char *file_extension_pgm = FILE_EXTENSION_PGM;
    const char *file_extension_pgmpart = FILE_EXTENSION_PGMPART;
    const char *partial_file_extension = file_extension_pgmpart; // default is partial
    // chunk of the world_local for each MPI process
    unsigned char *world_local;
    long world_size = 0;
    long local_size = 0;
    int maxval = 0;
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    int mpi_provided_thread_level;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - run_wave MPI_Init_thread free()\n");
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
    if (mpi_rank == 0)
        printf("Run request with EVOLUTION_WAVE of %d steps of filename=%s saving snaps each %d steps\n", number_of_steps, filename, number_of_steps_between_file_dumps);
    if (debug_info > 0)
        printf("DEBUG1 - run_wave BEGIN - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);

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
            printf("DEBUG2 - run_wave - char *string_with_num = malloc(string_with_num_size + 1); free(); BEFORE\n");
#endif
        char *string_with_num = malloc(string_with_num_size + 1);
        sprintf(string_with_num, "_" EVOLUTION_TYPE "_%05d_%03d_%%Y-%%m-%%d_%%H_%%M_%%S", number_of_steps, mpi_size);

        size_t string_with_timestamp_size = strlen("_" EVOLUTION_TYPE "_00000_000_2023-02-13_23:37:01");
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - char *string_with_timestamp = malloc(string_with_timestamp_size + 1); free(); BEFORE\n");
#endif
        char *string_with_timestamp = malloc(string_with_timestamp_size + 1);
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        unsigned long len = strftime(string_with_timestamp, string_with_timestamp_size + 1, string_with_num, timenow);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(string_with_num); BEFORE\n");
#endif
        free(string_with_num);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(string_with_num); AFTER\n");
#endif
        if (debug_info > 0) {
            printf("DEBUG1 - run_wave 1 - rank %d/%d, len=%ld string_with_timestamp=%s\n", mpi_rank, mpi_size, len, string_with_timestamp);
            printf("DEBUG1 - run_wave 1a - rank %d/%d, strlen(filename) + strlen(string_with_timestamp) + 1=%lu\n", mpi_rank, mpi_size, strlen(filename) + strlen(string_with_timestamp) + 1);
        }
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - directoryname = malloc(strlen(filename) + strlen(string_with_timestamp) + 1); free(); BEFORE\n");
#endif
        directoryname = malloc(strlen(filename) + strlen(string_with_timestamp) + 1);
        strcpy(directoryname, filename);
        strcat(directoryname, string_with_timestamp);
        replace_char(directoryname, '/', '_');
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(string_with_timestamp); BEFORE\n");
#endif
        free(string_with_timestamp);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(string_with_timestamp); AFTER\n");
#endif
        if (debug_info > 0)
            printf("DEBUG1 - run_wave 1b - rank %d/%d, strlen(directoryname)=%lu, directoryname=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname);
        // Broadcast the string to all other processes
        if (mpi_size > 1)
            MPI_Bcast(directoryname, (int) strlen(directoryname) + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
        else
            partial_file_extension = file_extension_pgm;
        t_io += make_directory(directoryname, debug_info);
    } else {
        // Other processes
        MPI_Bcast(message, MAX_STRING_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - directoryname = malloc(strlen(message) + 1); free(); BEFORE\n");
#endif
        directoryname = malloc(strlen(message) + 1);
        strcpy(directoryname, message);
        if (debug_info > 1)
            printf("DEBUG2 - run_wave 1c - rank %d/%d, LEN=%lu directoryname=%s, LEN=%lu message=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname, strlen(message), message);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (debug_info > 0)
        printf("DEBUG1 - run_wave 2 - rank %d/%d directoryname=%s\n", mpi_rank, mpi_size, directoryname);

    t_io += file_pgm_read_noghost(&world_local, &maxval, &local_size, &world_size, filename, mpi_rank, mpi_size, debug_info);
    if (debug_info > 0)
        printf("DEBUG1 - run_wave 3 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, filename);

    if (mpi_size > 1)
        t_io += iterate_wave_parallel(mpi_rank, mpi_size, &mpi_status, &mpi_request, world_local, world_size, local_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
    else
        t_io += iterate_wave_serial(world_local, world_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
    if (debug_info > 1)
        printf("DEBUG2 - run_wave 4 - ITERATED - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, directoryname=%s, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, directoryname, filename);
    // wait for all iterations to complete
    MPI_Barrier(MPI_COMM_WORLD);
    // write final iteration output
    t_io += file_pgm_write_chunk_noghost(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, "", partial_file_extension, mpi_rank, mpi_size, debug_info);
    // all processes send io-time to process zero
    if (mpi_rank > 0)
        MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request);

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
        //DEBUG2 - run_wave 5 - MERGE CHUNKS rank 0/2, pattern_random16.pgm_wave_2023-05-16_08_45_36/final_wave002_000.pgmpart
        // join chunks of all iteration steps
        for (int iteration_step = number_of_steps_between_file_dumps;
             iteration_step <= number_of_steps; iteration_step += number_of_steps_between_file_dumps) {
            if (debug_info > 1)
                printf("DEBUG2 - run_wave 5a0 - rank %d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, iteration_step, number_of_steps);
            if (debug_info > 1)
                printf("DEBUG2 - run_wave 5a1 - rank %d/%d, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            // filename of joined iteration snap
            char *snap_fn;
            unsigned long snap_fn_len = strlen("/_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_WAVE) + strlen(FILE_EXTENSION_PGM) + 1;
            if (debug_info > 1)
                printf("DEBUG2 - run_wave 5a2 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, snap_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - snap_fn = (char *) malloc(snap_fn_len); free(); BEFORE\n");
#endif
            snap_fn = (char *) malloc(snap_fn_len);
//          sprintf(snap_fn, "%s/%s%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            sprintf(snap_fn, "%s/%s_%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, iteration_step, FILE_EXTENSION_PGM);
            if (debug_info > 1)
                printf("DEBUG2 - run_wave 5a3 - snap_fn_len=%ld, strlen(%s)=%ld\n", snap_fn_len, snap_fn, strlen(snap_fn));

            // array of filenames of snap chunks to be joined
            char *snap_chunks_fn[mpi_size];
            unsigned long snap_chunks_fn_len = strlen("/_000_000_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_WAVE) + strlen(FILE_EXTENSION_PGMPART) + 1;
            if (debug_info > 1) // test chunks fn length
                printf("DEBUG2 - run_wave 5a4 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%03d_%05d%s.%s\n", mpi_rank, mpi_size, snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, mpi_rank, iteration_step, ""
                       , FILE_EXTENSION_PGMPART);
            for (int i = 0; i < mpi_size; i++) {
                if (debug_info > 1)
                    printf("DEBUG2 - run_wave 5a5: LEN=%lu %s/%s%03d_%03d_%05d%s.%s\n", snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
                if (debug_info > 1)
                    printf("DEBUG2 - run_wave - snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len); free(); BEFORE\n");
#endif
                snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len);
                sprintf(snap_chunks_fn[i], "%s/%s_%03d_%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
//              sprintf(snap_chunks_fn[i], "%s/%s%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_WAVE, iteration_step, FILE_EXTENSION_PGMPART);
                if (debug_info > 1)
                    printf("DEBUG2 - run_wave 5a6: LEN=%lu %s\n", strlen(snap_chunks_fn[i]), snap_chunks_fn[i]);
            }
            if (debug_info > 0)
                for (int i = 0; i < mpi_size; i++)
                    printf("DEBUG1 - run_wave 5a7: %s\n", snap_chunks_fn[i]);
            for (int i = 0; i < mpi_size; i++)
                t_io += file_chunk_merge(snap_fn, snap_chunks_fn[i], debug_info); // TODO: manage error result
            // delete chunks but keep them in debug mode
            double t_point = MPI_Wtime();
            if (debug_info == 0)
                for (int i = 0; i < mpi_size; i++)
                    remove(snap_chunks_fn[i]);
            t_io += MPI_Wtime() - t_point;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - free(snap_fn); BEFORE\n");
#endif
            free(snap_fn);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1) {
                printf("DEBUG2 - run_wave - free(snap_fn); AFTER\n");
                printf("DEBUG2 - run_wave - free(snap_chunks_fn[i]); BEFORE\n");
            }
#endif
            for (int i = 0; i < mpi_size; i++)
                free(snap_chunks_fn[i]); // TODO: verify
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - free(snap_chunks_fn[i]); AFTER\n");
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
            printf("DEBUG2 - run_wave - final_fn = (char *) malloc(final_fn_len); free(); BEFORE\n");
#endif
        final_fn = (char *) malloc(final_fn_len);
//      sprintf(final_fn, "%s/%s%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, "", FILE_EXTENSION_PGM);
        sprintf(final_fn, "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, FILE_EXTENSION_PGM);
        if (debug_info > 1) {
            // NB: strlen=string length, sizeof=length+1 one more char for the terminating zero
            printf("DEBUG2 - run_wave 5b0 - final_fn_len=%ld, strlen(%s)=%ld\n", final_fn_len, final_fn, strlen(final_fn));
            printf("DEBUG2 - run_wave 5b1 - strlen(\"/000_000.\")=%ld\n", strlen("/000_000."));
            printf("DEBUG2 - run_wave 5b2 - strlen(%s)=%ld\n", directoryname, strlen(directoryname));
            printf("DEBUG2 - run_wave 5b3 - sizeof(%s)=%ld\n", IMAGE_FILENAME_PREFIX_FINAL_WAVE, sizeof(IMAGE_FILENAME_PREFIX_FINAL_WAVE));
            printf("DEBUG2 - run_wave 5b4 - sizeof(%s)=%ld\n", FILE_EXTENSION_PGMPART, sizeof(FILE_EXTENSION_PGMPART));
        }
        // array of filenames of chunks to be joined
        char *final_chunks_fn[mpi_size];
        unsigned long final_chunks_fn_len = strlen("/_000_000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_WAVE) + strlen(FILE_EXTENSION_PGMPART) + 1;
        if (debug_info > 1) // test chunks fn length
            printf("DEBUG2 - run_wave 5 - MERGE CHUNKS FINAL rank %d/%d, LEN=%lu, final_chunks_fn=%s/%s%03d_%03d%s.%s\n", mpi_rank, mpi_size, final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, mpi_rank, ""
                   , FILE_EXTENSION_PGMPART);
        for (int i = 0; i < mpi_size; i++) {
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - JOIN1a: LEN=%lu %s/%s%03d_%03d%s.%s\n", final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len); free(); BEFORE\n");
#endif
            final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len);
            sprintf(final_chunks_fn[i], "%s/%s_%03d_%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, mpi_size, i, "", FILE_EXTENSION_PGMPART);
//          sprintf(final_chunks_fn[i], "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_WAVE, FILE_EXTENSION_PGMPART);
            if (debug_info > 1)
                printf("DEBUG2 - run_wave - JOIN1b: LEN=%lu %s\n", strlen(final_chunks_fn[i]), final_chunks_fn[i]);
        }
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - run_wave - JOIN2: %s\n", final_chunks_fn[i]);
        // delete if already existing, unnecessary as we create a new dir
        //int remove_result = remove(pathname);
        //if (debug_info > 0)
        //    printf("DEBUG1 - run_wave - remove_result: %d\n", remove_result);
        for (int i = 0; i < mpi_size; i++)
            t_io += file_chunk_merge(final_fn, final_chunks_fn[i], debug_info); // TODO: manage error result
        // delete chunks but keep them in debug mode
        double t_point = MPI_Wtime();
        if (debug_info == 0)
            for (int i = 0; i < mpi_size; i++)
                remove(final_chunks_fn[i]);
        t_io += MPI_Wtime() - t_point;
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(final_fn); BEFORE\n");
#endif
        free(final_fn);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1) {
            printf("DEBUG2 - run_wave - free(final_fn); AFTER\n");
            printf("DEBUG2 - run_wave - free(final_chunks_fn[i]); BEFORE\n");
        }
#endif
        for (int i = 0; i < mpi_size; i++)
            free(final_chunks_fn[i]); // TODO: verify
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        if (debug_info > 1)
            printf("DEBUG2 - run_wave - free(final_chunks_fn[i]); AFTER\n");
#endif
        double t_io_other = 0;
        for (int i = 1; i < mpi_size; i++) {
            MPI_Recv(&t_io_other, 1, MPI_DOUBLE, i, TAG_T, MPI_COMM_WORLD, &mpi_status);
            t_io_accumulator += t_io_other;
            if (debug_info > 1)
                printf("DEBUG2 - run_wave ACCU1 - i=%d, t_io_other=%f, t_io_accumulator=%f\n", i, t_io_other, t_io_accumulator);
        }
        if (debug_info > 0)
            printf("DEBUG1 - run_wave ACCU2 - t_io=%f, t_io_other=%f, t_io_accumulator=%f, t_io_accumulator / (mpi_size - 1)=%f\n", t_io, t_io_other, t_io_accumulator, t_io_accumulator / (mpi_size - 1));
    }

    if (mpi_rank == 0 && debug_info > 0)
        //DEBUG1 - run_wave 6 - mpi=2, omp=2, time taken=0.007455
        printf("DEBUG1 - run_wave 6 - mpi=%d, omp=%d, time taken=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - run_wave - free(directoryname); BEFORE\n");
#endif
    free(directoryname);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1) {
        printf("DEBUG2 - run_wave - free(directoryname); AFTER\n");
        printf("DEBUG2 - run_wave - BEFORE MPI_Finalize() free() - strlen(%s)=%ld\n", directoryname, strlen(directoryname));
    }
#endif
    MPI_Finalize();
    if (debug_info > 1)
        printf("DEBUG2 - run_wave 7 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - run_wave - free(world_local); BEFORE\n");
#endif
    free(world_local);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    if (debug_info > 1)
        printf("DEBUG2 - run_wave - free(world_local); AFTER\n");
#endif
    if (debug_info > 1)
        printf("DEBUG2 - run_wave 8 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
    if (mpi_rank == 0)
        printf("mpi=%d, omp=%d, total time=%f, I/O time=%f, I/O time t_io_accumulator=%f, t_io_accumulator mean=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator, t_io_accumulator / (mpi_size - 1));
    if (debug_info > 0)
        printf("DEBUG1 - run_wave END - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
}
