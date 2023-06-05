#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <string.h>
#include <omp.h>
#include "files_io.h"
#include "gameoflife.h"

#define EVOLUTION_TYPE "static"
#define IMAGE_FILENAME_PREFIX_SNAP_STATIC "snapshot"
#define IMAGE_FILENAME_PREFIX_FINAL_STATIC "final"

void set_dead_or_alive_static_parallel(int mpi_rank, int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char *world_local, unsigned char *world_next, long long world_size, long local_size, int iteration_step) {
#pragma omp master
    {
        // tags definition for the MPI message exchange
        int tag_0 = 2 * iteration_step;
        int tag_1 = 2 * iteration_step + 1;

#ifdef DEBUG_ADVANCED_MALLOC_FREE
        printf("DEBUGA - set_dead_or_alive_parallel_static - mpi_rank=%d/%d, world_size=%lld, local_size=%ld\n", mpi_rank, mpi_size, world_size, local_size);
#endif
        // each process send his first row to the process with mpi_rank-1
        // and last row to mpi_rank + 1.
        // process 0      sends his first row to process mpi_size -1
        // process mpi_size-1 sends his last  row to process 0
        // TODO: chunk size passed as INT by MPI, needs better implementation to work with bigger sizes
        if (mpi_rank != 0 && mpi_rank != mpi_size - 1) {
            MPI_Isend(&world_local[world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, tag_0, MPI_COMM_WORLD, mpi_request);
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank + 1, tag_1, MPI_COMM_WORLD, mpi_request);
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank + 1, tag_0, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, tag_1, MPI_COMM_WORLD, mpi_status);
        }
        if (mpi_rank == 0) {
#ifdef DEBUG_ADVANCED_B
            if (iteration_step == 1) {
                printf("DEBUGB - set_dead_or_alive_parallel_static 0a - mpi_rank=%d/%d, iteration_step=%d, world_local[0-7]=%d %d %d %d %d %d %d %d\n", mpi_rank, mpi_size
                       , iteration_step, world_local[world_size], world_local[world_size + 1], world_local[world_size + 2], world_local[world_size + 3]
                       , world_local[world_size + 4], world_local[world_size + 5], world_local[world_size + 6], world_local[world_size + 7]);
                for (long long i = 0; i < world_size * (local_size + 2); i++) {
                    if (i % 16 == 0)
                        printf("DEBUGB - set_dead_or_alive_parallel_static 0a - %08X: ", (unsigned int) i);
                    //printf("%d ", world_local[i] == DEAD); 0/1
                    printf("%02X ", world_local[i]);
                    if (i % 16 == 15)
                        printf("\n");
                }
                printf("\n");
            }
#endif
            MPI_Isend(&world_local[world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_size - 1, tag_0, MPI_COMM_WORLD, mpi_request);
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 1, tag_1, MPI_COMM_WORLD, mpi_request);
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 1, tag_0, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_size - 1, tag_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (iteration_step == 1) {
#ifdef DEBUG_ADVANCED_B
                printf("DEBUGB - set_dead_or_alive_parallel_static 0b - mpi_rank=%d/%d, iteration_step=%d, world_local[0-7]=%d %d %d %d %d %d %d %d\n", mpi_rank, mpi_size
                       , iteration_step, world_local[world_size], world_local[world_size + 1], world_local[world_size + 2], world_local[world_size + 3]
                       , world_local[world_size + 4], world_local[world_size + 5], world_local[world_size + 6], world_local[world_size + 7]);
#endif
                for (long long i = 0; i < world_size * (local_size + 2); i++) {
                    if (i % 16 == 0)
                        printf("DEBUGA - set_dead_or_alive_parallel_static 0b - %08X: ", (unsigned int) i);
                    //printf("%d ", world_local[i] == DEAD); 0/1
                    printf("%02X ", world_local[i]);
                    if (i % 16 == 15)
                        printf("\n");
                }
                printf("\n");
            }
#endif
        }
        if (mpi_rank == mpi_size - 1) {
            MPI_Isend(&world_local[world_size], (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, tag_0, MPI_COMM_WORLD, mpi_request);
            MPI_Isend(&world_local[(local_size) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 0, tag_1, MPI_COMM_WORLD, mpi_request);
            MPI_Recv(&world_local[(local_size + 1) * world_size], (int) world_size, MPI_UNSIGNED_CHAR, 0, tag_0, MPI_COMM_WORLD, mpi_status);
            MPI_Recv(world_local, (int) world_size, MPI_UNSIGNED_CHAR, mpi_rank - 1, tag_1, MPI_COMM_WORLD, mpi_status);
#ifdef DEBUG_ADVANCED_MALLOC_FREE
            if (iteration_step == 1) {
                printf("DEBUGA - set_dead_or_alive_parallel_static 0c - mpi_rank=%d/%d, iteration_step=%d, world_local[0-7]=%d %d %d %d %d %d %d %d\n", mpi_rank, mpi_size
                       , iteration_step, world_local[world_size], world_local[world_size + 1], world_local[world_size + 2], world_local[world_size + 3]
                       , world_local[world_size + 4], world_local[world_size + 5], world_local[world_size + 6], world_local[world_size + 7]);
                for (long long i = 0; i < world_size * (local_size + 2); i++) {
                    if (i % 16 == 0)
                        printf("DEBUGA - set_dead_or_alive_parallel_static 0c - %08X: ", (unsigned int) i);
                    //printf("%d ", world_local[i] == DEAD); 0/1
                    printf("%02X ", world_local[i]);
                    if (i % 16 == 15)
                        printf("\n");
                }
                printf("\n");
            }
#endif
        }
    }
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    printf("DEBUGA - set_dead_or_alive_parallel_static 1 - mpi_rank=%d/%d, iteration_step=%d, local_size=%ld, world_size * (local_size + 1)=%lld\n", mpi_rank, mpi_size, iteration_step, local_size, world_size * (local_size + 1));
#endif
#pragma omp for
    // TODO: declare vars before loops to optimize performance
    for (long long i = world_size; i < world_size * (local_size + 1); i++) {
#ifdef DEBUG_ADVANCED_B
        printf("DEBUGB - set_dead_or_alive_parallel_static 2 - mpi_rank=%d/%d, iteration_step=%d, i/local_size=%lld/%ld\n", mpi_rank, mpi_size, iteration_step, i, local_size);
#endif
        // actual cell coordinates
        long x = i % world_size;
        long y = i / world_size;
#ifdef DEBUG_ADVANCED_B
        printf("DEBUGB - set_dead_or_alive_parallel_static 3 - mpi_rank=%d/%d, iteration_step=%d, i/local_size=%03lld/%ld x=%03ld, y=%03ld\n", mpi_rank, mpi_size, iteration_step, i, local_size, x, y);
#endif
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
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        printf("DEBUGA - set_dead_or_alive_parallel_static 4 - mpi_rank=%d/%d, iteration_step=%d, i/local_size=%03lld/%ld x=%03ld, y=%03ld sum=%d\n", mpi_rank, mpi_size, iteration_step, i, local_size, x, y, sum);
#endif
        // default is: cell will die
        world_next[i] = DEAD;
        int number_of_dead_neighbours = sum / DEAD; // 8-sum/255
        // https://en.wikipedia.org/wiki/Conway's_Game_of_Life
        // Any live cell with two or three live neighbours survives.    : 1+2=3 1+3=4 means 8-3=5 8-4=4 dead
        // Any dead cell with three live neighbours becomes a live cell.: 3           means 8-3=5       dead
        // All other alive cells die in the next generation. Similarly, all other dead cells stay dead.
        //if (number_of_dead_neighbours >= 5 & number_of_dead_neighbours <= 6)
        //    world_next[i] = ALIVE;
        if (world_local[y * world_size + x] == ALIVE) { // if actual cell is alive
            if (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)
                world_next[i] = ALIVE;
        } else // actual cell is dead
        if (number_of_dead_neighbours == 5) {
            world_next[i] = ALIVE;
        }
    }
#pragma omp barrier
}

void set_dead_or_alive_static_single(unsigned char *world, unsigned char *world_next, long world_size) {
#ifdef DEBUG_ADVANCED_MALLOC_FREE
    printf("DEBUGA - set_dead_or_alive_serial_static 0 - world_size=%ld, world_size * (world_size + 1)=%ld\n", world_size, world_size * (world_size + 1));
#endif
// copy last row [world_size] before first [0]
// and first [1] after last [world_size + 1] (ghost rows)
#pragma omp for
    for (long i = 0; i < world_size; i++) {
        world[i] = world[world_size * world_size + i];
        world[world_size * (world_size + 1) + i] = world[world_size + i];
    }
#ifdef DEBUG_ADVANCED_B
#pragma omp barrier
    for (long long i = 0; i < world_size * (world_size + 2); i++) {
        if (i % 16 == 0)
            printf("DEBUGB - set_dead_or_alive_serial_static 1 - %08X: ", (unsigned int) i);
        printf("%02X ", world[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n");
#endif
#pragma omp for
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
#ifdef DEBUG_ADVANCED_MALLOC_FREE
        printf("DEBUGA - set_dead_or_alive_serial_static 4 - i/world_size=%03lld/%ld x=%03ld, y=%03ld sum=%d\n", i, world_size, x, y, sum);
#endif
        // default is: cell will die
        world_next[i] = DEAD;
        int number_of_dead_neighbours = sum / DEAD; // 8-sum/255
        // https://en.wikipedia.org/wiki/Conway's_Game_of_Life
        // Any live cell with two or three live neighbours survives.    : 1+2=3 1+3=4 means 8-3=5 8-4=4 dead
        // Any dead cell with three live neighbours becomes a live cell.: 3           means 8-3=5       dead
        // All other alive cells die in the next generation. Similarly, all other dead cells stay dead.
        //if (number_of_dead_neighbours >= 5 & number_of_dead_neighbours <= 6)
        //    world_next[i] = ALIVE;
        if (world[y * world_size + x] == ALIVE) { // if actual cell is alive
            if (number_of_dead_neighbours == 5 || number_of_dead_neighbours == 6)
                world_next[i] = ALIVE;
        } else // actual cell is dead
        if (number_of_dead_neighbours == 5) {
            world_next[i] = ALIVE;
        }
    }
}

double evolution_static_parallel(const int mpi_rank, const int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char **world_local, const long world_size, const long local_size, const int number_of_steps
                                 , const int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_static_parallel - BEGIN - mpi_rank=%d/%d, world_size=%ld\n", mpi_rank, mpi_size, world_size);
#endif
    unsigned char *world_local_actual = *world_local;
    // allocate memory for the next state
    unsigned char *world_local_next = (unsigned char *) malloc(world_size * (local_size + 2) * sizeof(unsigned char));
    // keep a copy to free at the end
    unsigned char *world_local_next_original = world_local_next;
    char *image_filename_suffix = (char *) malloc(60);

    // NOTE: can't use omp parallel here, iteration can't go on for each chunk
    // each MPI process exchanges data with other segments of same iteration
    // it's ok for serial only (-np 1)
    for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_static_parallel 0 - mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
        set_dead_or_alive_static_parallel(mpi_rank, mpi_size, mpi_status, mpi_request, world_local_actual, world_local_next, world_size, local_size, iteration_step);
        // when needed save snapshot
        if (iteration_step % number_of_steps_between_file_dumps == 0) {
            sprintf(image_filename_suffix, "_%05d", iteration_step);
            t_io += file_pgm_write_chunk(world_local_next, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, image_filename_suffix, FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static_parallel 1 - snap written mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
        }
        // pointers swap to reuse allocated world_local and world_local_next for next iteration
        unsigned char *temp = world_local_actual;
        world_local_actual = world_local_next;
        world_local_next = temp;
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_static_parallel 2 - mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
    }
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static_parallel 3 - mpi_rank=%d/%d, omp_rank=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads());
#endif
    // if we had an odd swap, copy all the matrix
    if (number_of_steps % 2 == 1)
        for (long long i = world_size; i < world_size * (local_size + 1); i++)
            world_local_next[i] = world_local_actual[i];
    free(world_local_next_original);
    free(image_filename_suffix);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static_parallel END - mpi_rank=%d/%d, omp_rank=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads());
#endif

    return t_io;
}

double evolution_static_single(const int mpi_rank, const int mpi_size, MPI_Status *mpi_status, MPI_Request *mpi_request, unsigned char **world_local, const long world_size, const long local_size, const int number_of_steps
                               , const int number_of_steps_between_file_dumps, const char *directoryname, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_static_serial - BEGIN - mpi_rank=%d/%d, world_size=%ld\n", mpi_rank, mpi_size, world_size);
#endif
    unsigned char *world_local_actual = *world_local;
    // allocate memory for the next state
    unsigned char *world_local_next = (unsigned char *) malloc(world_size * (local_size + 2) * sizeof(unsigned char));
    // keep a copy to free at the end
    unsigned char *world_local_next_original = world_local_next;
    char *image_filename_suffix = (char *) malloc(60);

#pragma omp parallel default(none) shared(number_of_steps, debug_info, mpi_rank, mpi_size, world_local, world_local_actual, world_local_next, world_size, local_size, mpi_status, mpi_request, number_of_steps_between_file_dumps, image_filename_suffix, directoryname, t_io)
    {
        for (int iteration_step = 1; iteration_step <= number_of_steps; iteration_step++) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static_serial 0 - mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
            set_dead_or_alive_static_single(world_local_actual, world_local_next, world_size);
#pragma omp master
            {
                // when needed save snapshot
                if (iteration_step % number_of_steps_between_file_dumps == 0) {
                    sprintf(image_filename_suffix, "_%05d", iteration_step);
                    t_io += file_pgm_write_chunk(world_local_next, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, image_filename_suffix, FILE_EXTENSION_PGM, mpi_rank, mpi_size, debug_info);
#ifdef DEBUG2
                    if (debug_info > 1)
                        printf("DEBUG2 - evolution_static_serial 1 - snap written mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
                }
                // pointers swap to reuse allocated world_local and world_local_next for next iteration
                unsigned char *temp = world_local_actual;
                world_local_actual = world_local_next;
                world_local_next = temp;
            }
#pragma omp barrier
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static_serial 2 - mpi_rank=%d/%d, omp_rank=%d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads(), iteration_step, number_of_steps);
#endif
        }
    }
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static_serial 3 - mpi_rank=%d/%d, omp_rank=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads());
#endif
    // if we had an odd swap, copy all the matrix
    if (number_of_steps % 2 == 1)
        for (long long i = world_size; i < world_size * (local_size + 1); i++)
            world_local_next[i] = world_local_actual[i];
    free(world_local_next_original);
    free(image_filename_suffix);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static_serial END - mpi_rank=%d/%d, omp_rank=%d/%d\n", mpi_rank, mpi_size, omp_get_thread_num(), omp_get_max_threads());
#endif
    return t_io;
}

void evolution_static(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info) {
    double t_io = 0; // total I/O time spent
    double t_io_accumulator = 0; // total I/O time spent by processes > 0
    double t_start = MPI_Wtime(); // start time
// TODO: compute the correct size for MPI message allocation
#define MAX_STRING_LENGTH 256
    char message[MAX_STRING_LENGTH];
    char *directoryname;
    const char *file_extension_pgm = FILE_EXTENSION_PGM;
    const char *file_extension_pgmpart = FILE_EXTENSION_PGMPART;
    const char *partial_file_extension = file_extension_pgmpart; // default is partial chunk
    // chunk of the world_local for each MPI process
    unsigned char *world_local;
    long world_size = 0;
    long local_size = 0;
    int maxval = 0;
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
    if (mpi_rank == 0 && csv_output == CSV_OUTPUT_FALSE)
        printf("Run request with EVOLUTION_STATIC of %d steps of filename=%s saving snaps each %d steps\n", number_of_steps, filename, number_of_steps_between_file_dumps);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_static BEGIN - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
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
        sprintf(string_with_num, "_" EVOLUTION_TYPE "_%05d_%03d_%%Y-%%m-%%d_%%H_%%M_%%S", number_of_steps, mpi_size);

        size_t string_with_timestamp_size = strlen("_" EVOLUTION_TYPE "_00000_000_2023-02-13_23:37:01");
        char *string_with_timestamp = malloc(string_with_timestamp_size + 1);
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        unsigned long len = strftime(string_with_timestamp, string_with_timestamp_size + 1, string_with_num, timenow);
        free(string_with_num);
#ifdef DEBUG1
        if (debug_info > 0) {
            printf("DEBUG1 - evolution_static 1 - rank %d/%d, len=%ld string_with_timestamp=%s\n", mpi_rank, mpi_size, len, string_with_timestamp);
            printf("DEBUG1 - evolution_static 1a - rank %d/%d, strlen(filename) + strlen(string_with_timestamp) + 1=%lu\n", mpi_rank, mpi_size, strlen(filename) + strlen(string_with_timestamp) + 1);
        }
#endif
        directoryname = malloc(strlen(filename) + strlen(string_with_timestamp) + 1);
        strcpy(directoryname, filename);
        strcat(directoryname, string_with_timestamp);
        replace_char(directoryname, '/', '_');
        free(string_with_timestamp);
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_static 1b - rank %d/%d, strlen(directoryname)=%lu, directoryname=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname);
#endif
        // Broadcast the string to all other processes
        if (mpi_size > 1)
            MPI_Bcast(directoryname, (int) strlen(directoryname) + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
        else
            partial_file_extension = file_extension_pgm;
        t_io += make_directory(directoryname, debug_info);
    } else {
        // Other processes
        MPI_Bcast(message, MAX_STRING_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
        directoryname = malloc(strlen(message) + 1);
        strcpy(directoryname, message);
#ifdef DEBUG2
        if (debug_info > 1)
            printf("DEBUG2 - evolution_static 1c - rank %d/%d, LEN=%lu directoryname=%s, LEN=%lu message=%s\n", mpi_rank, mpi_size, strlen(directoryname), directoryname, strlen(message), message);
#endif
    }
    MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_static 2 - rank %d/%d directoryname=%s\n", mpi_rank, mpi_size, directoryname);
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
        printf("DEBUG1 - evolution_static 3 - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, filename);
#endif
    if (mpi_size > world_size || local_size == 0) {
        perror("ERROR: wrong situation with more processes than domain decomposition slices\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (mpi_size > 1)
        t_io += evolution_static_parallel(mpi_rank, mpi_size, &mpi_status, &mpi_request, &world_local, world_size, local_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
    else
        t_io += evolution_static_single(mpi_rank, mpi_size, &mpi_status, &mpi_request, &world_local, world_size, local_size, number_of_steps, number_of_steps_between_file_dumps, directoryname, debug_info);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static 4 - ITERATED - rank %d/%d - maxval=%d, local_size=%ld, world_size=%ld, directoryname=%s, filename=%s\n", mpi_rank, mpi_size, maxval, local_size, world_size, directoryname, filename);
#endif
    // wait for all iterations to complete
    MPI_Barrier(MPI_COMM_WORLD);
    // write final iteration output
    t_io += file_pgm_write_chunk(world_local, 255, world_size, local_size, directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, "", partial_file_extension, mpi_rank, mpi_size, debug_info);
    // all processes send io-time to process zero
    if (mpi_rank > 0)
        MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request);

    MPI_Barrier(MPI_COMM_WORLD);
    // merge chunks of final output if needed
    if (mpi_size > 1 && mpi_rank == 0) {
        // Join the chunks into a single image file
        //    final_static002.pgm              (final output with 2 MPI processes)
        //    final_static002_000.pgmpart      (final output chunk 0)
        //    final_static002_001.pgmpart      (final output chunk 1)
        //    snap_static002_000_00001.pgmpart (snap of iteration 1 chunk 0)
        //    snap_static002_001_00001.pgmpart (snap of iteration 1 chunk 1)
        //    snap_static002_000_00002.pgmpart (snap 2 = final chunk 0)
        //    snap_static002_001_00002.pgmpart (snap 2 = final chunk 1)
        //sprintf(file_name, "%s/%s%03d_%03d%s.%s", directoryname, image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
        //DEBUG2 - evolution_static 5 - MERGE CHUNKS rank 0/2, pattern_random16.pgm_static_2023-05-16_08_45_36/final_static002_000.pgmpart
        // join chunks of all iteration steps
        for (int iteration_step = number_of_steps_between_file_dumps;
             iteration_step <= number_of_steps; iteration_step += number_of_steps_between_file_dumps) {
#ifdef DEBUG2
            if (debug_info > 1) {
                printf("DEBUG2 - evolution_static 5a0 - rank %d/%d, iteration_step=%d/%d\n", mpi_rank, mpi_size, iteration_step, number_of_steps);
                printf("DEBUG2 - evolution_static 5a1 - rank %d/%d, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            }
#endif
            // filename of joined iteration snap
            char *snap_fn;
            unsigned long snap_fn_len = strlen("/_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_STATIC) + strlen(FILE_EXTENSION_PGM) + 1;
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static 5a2 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%05d%s.%s\n", mpi_rank, mpi_size, snap_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
#endif
            snap_fn = (char *) malloc(snap_fn_len);
//          sprintf(snap_fn, "%s/%s%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, iteration_step, "", FILE_EXTENSION_PGM);
            sprintf(snap_fn, "%s/%s_%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, iteration_step, FILE_EXTENSION_PGM);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static 5a3 - snap_fn_len=%ld, strlen(%s)=%ld\n", snap_fn_len, snap_fn, strlen(snap_fn));
#endif

            // array of filenames of snap chunks to be joined
            char *snap_chunks_fn[mpi_size];
            unsigned long snap_chunks_fn_len = strlen("/_000_000_00000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_SNAP_STATIC) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
            if (debug_info > 1) // test chunks fn length
                printf("DEBUG2 - evolution_static 5a4 - rank %d/%d, LEN=%lu, snap_chunks_fn=%s/%s%03d_%03d_%05d%s.%s\n"
                       , mpi_rank, mpi_size, snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, mpi_rank, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
            for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_static 5a5: LEN=%lu %s/%s%03d_%03d_%05d%s.%s\n", snap_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
#endif
                snap_chunks_fn[i] = (char *) malloc(snap_chunks_fn_len);
                sprintf(snap_chunks_fn[i], "%s/%s_%03d_%03d_%05d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, mpi_size, i, iteration_step, "", FILE_EXTENSION_PGMPART);
//              sprintf(snap_chunks_fn[i], "%s/%s%05d.%s", directoryname, IMAGE_FILENAME_PREFIX_SNAP_STATIC, iteration_step, FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
                if (debug_info > 1)
                    printf("DEBUG2 - evolution_static 5a6: LEN=%lu %s\n", strlen(snap_chunks_fn[i]), snap_chunks_fn[i]);
#endif
            }
#ifdef DEBUG1
            if (debug_info > 0)
                for (int i = 0; i < mpi_size; i++)
                    printf("DEBUG1 - evolution_static 5a7: %s\n", snap_chunks_fn[i]);
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
                strlen("/.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_STATIC) +
                strlen(FILE_EXTENSION_PGM) + 1;
        final_fn = (char *) malloc(final_fn_len);
//      sprintf(final_fn, "%s/%s%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, mpi_size, "", FILE_EXTENSION_PGM);
        sprintf(final_fn, "%s/%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, FILE_EXTENSION_PGM);
#ifdef DEBUG2
        if (debug_info > 1) {
            // NB: strlen=string length, sizeof=length+1 one more char for the terminating zero
            printf("DEBUG2 - evolution_static 5b0 - final_fn_len=%ld, strlen(%s)=%ld\n", final_fn_len, final_fn, strlen(final_fn));
            printf("DEBUG2 - evolution_static 5b1 - strlen(\"/000_000.\")=%ld\n", strlen("/000_000."));
            printf("DEBUG2 - evolution_static 5b2 - strlen(%s)=%ld\n", directoryname, strlen(directoryname));
            printf("DEBUG2 - evolution_static 5b3 - sizeof(%s)=%ld\n", IMAGE_FILENAME_PREFIX_FINAL_STATIC, sizeof(IMAGE_FILENAME_PREFIX_FINAL_STATIC));
            printf("DEBUG2 - evolution_static 5b4 - sizeof(%s)=%ld\n", FILE_EXTENSION_PGMPART, sizeof(FILE_EXTENSION_PGMPART));
        }
#endif
        // array of filenames of chunks to be joined
        char *final_chunks_fn[mpi_size];
        unsigned long final_chunks_fn_len = strlen("/_000_000.") + strlen(directoryname) + strlen(IMAGE_FILENAME_PREFIX_FINAL_STATIC) + strlen(FILE_EXTENSION_PGMPART) + 1;
#ifdef DEBUG2
        if (debug_info > 1) // test chunks fn length
            printf("DEBUG2 - evolution_static 5 - MERGE CHUNKS FINAL rank %d/%d, LEN=%lu, final_chunks_fn=%s/%s%03d_%03d%s.%s\n"
                   , mpi_rank, mpi_size, final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, mpi_size, mpi_rank, "", FILE_EXTENSION_PGMPART);
#endif
        for (int i = 0; i < mpi_size; i++) {
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static - JOIN1a: LEN=%lu %s/%s%03d_%03d%s.%s\n"
                       , final_chunks_fn_len, directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#endif
            final_chunks_fn[i] = (char *) malloc(final_chunks_fn_len);
            sprintf(final_chunks_fn[i], "%s/%s_%03d_%03d%s.%s", directoryname, IMAGE_FILENAME_PREFIX_FINAL_STATIC, mpi_size, i, "", FILE_EXTENSION_PGMPART);
#ifdef DEBUG2
            if (debug_info > 1)
                printf("DEBUG2 - evolution_static - JOIN1b: LEN=%lu %s\n", strlen(final_chunks_fn[i]), final_chunks_fn[i]);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - evolution_static - JOIN2: %s\n", final_chunks_fn[i]);
#endif
        // delete if already existing, unnecessary as we create a new dir
        //int remove_result = remove(pathname);
        //if (debug_info > 0)
        //    printf("DEBUG1 - evolution_static - remove_result: %d\n", remove_result);
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
                printf("DEBUG2 - evolution_static ACCU1 - i=%d, t_io_other=%f, t_io_accumulator=%f\n", i, t_io_other, t_io_accumulator);
#endif
        }
#ifdef DEBUG1
        if (debug_info > 0)
            printf("DEBUG1 - evolution_static ACCU2 - t_io=%f, t_io_other=%f, t_io_accumulator=%f, t_io_accumulator / (mpi_size - 1)=%f\n", t_io, t_io_other, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#endif
    }

#ifdef DEBUG1
    if (mpi_rank == 0 && debug_info > 0)
        //DEBUG1 - evolution_static 6 - mpi=2, omp=2, time taken=0.007455
        printf("DEBUG1 - evolution_static 6 - mpi=%d, omp=%d, total time=%f, I/O time taken=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io);
#endif
    MPI_Finalize();
    free(directoryname);
#ifdef DEBUG2
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static 7 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
    free(world_local);
#ifdef DEBUG1
    if (debug_info > 1)
        printf("DEBUG2 - evolution_static 8 - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
    if (mpi_rank == 0)
        if (csv_output == CSV_OUTPUT_FALSE)
            printf("mpi=%d, omp=%d, total time=%f, I/O time=%f, I/O time t_io_accumulator=%f, t_io_accumulator mean=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator,
                    mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
        else
            printf("%d,%d,%f,%f,%f,%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
#ifdef DEBUG1
    if (debug_info > 0)
        printf("DEBUG1 - evolution_static END - rank %d/%d, filename=%s\n", mpi_rank, mpi_size, filename);
#endif
}
