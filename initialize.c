#include "initialize.h"
#include "gameoflife.h"

int get_unique_seed(int omp_rank, int mpi_rank) {
    return (int) (123 + omp_rank * mpi_rank * 7 + 8 * omp_rank * omp_rank + 9 * mpi_rank * mpi_rank * time(NULL));
}

double initialize_parallel(long total_size, int mpi_size, int mpi_rank, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    long chunk_size; // chunk of rows of each MPI task: world_chunk rows / number of threads (mpi_size)
    chunk_size =
            total_size % mpi_size - mpi_rank <= 0 ? (long) (total_size / mpi_size) : (long) (total_size / mpi_size) + 1;
    if (debug_info > 0) {
        printf("DEBUG1 - initialize_parallel - BEGIN - rank=%d/%d, chunk_size=%ld/%ld\n", mpi_rank, mpi_size, chunk_size, total_size);
        // not much sense in parallel execution
        //if (debug_info > 1  && mpi_rank == 0)
        //  printf("DEBUG2 - initialize_parallel - values: ");
    }
    unsigned char *world_chunk;
    world_chunk = (unsigned char *) malloc(total_size * (chunk_size + 1) * sizeof(unsigned char));
    MPI_Barrier(MPI_COMM_WORLD);
#pragma omp parallel default(none) shared(mpi_rank, total_size, chunk_size, world_chunk)
    {
        int seed = get_unique_seed(omp_get_thread_num(), mpi_rank);
        srand(seed);
#pragma omp for schedule(static, 1)
        for (long long i = total_size; i < total_size * (chunk_size + 1); i++) {
            int val = rand() % 100;
            //if (debug_info > 1)
            //    printf("%d, ", val);
            if (val < 70)
                world_chunk[i] = DEAD;
            else
                world_chunk[i] = ALIVE;
        }
    }
    //if (debug_info > 1 && mpi_rank == 0)
    //    printf("\n");
    t_io += file_pgm_write_chunk(world_chunk, 255, total_size, chunk_size, "", IMAGE_FILENAME_PREFIX_INIT, "", FILE_EXTENSION_PGMPART, mpi_rank, mpi_size, debug_info);
    free(world_chunk);
    if (debug_info > 0)
        printf("DEBUG1 - initialize_parallel - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    return t_io;
}

double initialize_serial(const char *filename, long total_size, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    if (debug_info > 0)
        printf("DEBUG1 - initialize_serial - BEGIN\n");
    if (debug_info > 1)
        printf("DEBUG2 - initialize_serial - values: ");
    unsigned char *world;
    world = (unsigned char *) malloc(total_size * (total_size + 1) * sizeof(unsigned char));
#pragma omp parallel default(none) shared(total_size, debug_info, world)
    {
        int seed = get_unique_seed(omp_get_thread_num(), 0);
        srand(seed);
#pragma omp for schedule(static, 1)
        for (long long i = total_size; i < total_size * (total_size + 1); i++) {
            int val = rand() % 100;
            if (debug_info > 1)
                printf("%d, ", val);
            if (val < 70)
                world[i] = 255;
            else
                world[i] = 0;
        }
    }
    if (debug_info > 1)
        printf("\n");
    t_io += file_pgm_write_chunk(world, 255, total_size, total_size, "", filename, "", FILE_EXTENSION_PGM, 0, 1, debug_info);
    free(world);
    if (debug_info > 0)
        printf("DEBUG1 - initialize_serial - END\n");
    return t_io;
}

void initialization(long world_size, const char *filename, int *argc, char ***argv, int mpi_rank, int mpi_size, int debug_info) {
    double t_io = 0; // total I/O time spent
    double t_io_accumulator = 0; // total I/O time spent by processes > 0
    double t_start = MPI_Wtime(); // start time
    MPI_Status mpi_status;
    MPI_Request mpi_request;
    if (debug_info > 0 && mpi_rank == 0)
        printf("DEBUG1 - initialization - BEGIN filename=%s\n", filename);
    // Concatenate: pathname + extension
    //if (debug_info > 0 && mpi_rank == 0)
    char *pathname = malloc(strlen(filename) + strlen("." FILE_EXTENSION_PGM) + 1);
    strcpy(pathname, filename);
    strcat(pathname, "." FILE_EXTENSION_PGM);
    if (debug_info > 0)
        //printf("DEBUG1 - initialization - pathname=%s\n", pathname);
        printf("DEBUG1 - initialization - rank %d/%d, pathname=%s\n", mpi_rank, mpi_size, pathname);
    if (mpi_size == 1)
        t_io += initialize_serial(filename, world_size, debug_info);
    else
        t_io += initialize_parallel(world_size, mpi_size, mpi_rank, debug_info);
    // all processes send io-time to process zero
    if (mpi_rank > 0)
        MPI_Isend(&t_io, 1, MPI_DOUBLE, 0, TAG_T, MPI_COMM_WORLD, &mpi_request);

    MPI_Barrier(MPI_COMM_WORLD);
    // Join the chunks into a single image file
    //        init_004_000.pgmpart
    //        init_004_001.pgmpart
    //        init_004_002.pgmpart
    //        init_004_003.pgmpart
    if (mpi_size > 1 && mpi_rank == 0) {
        char *fn[mpi_size];
        for (int i = 0; i < mpi_size; i++) {
            if (debug_info > 0)
                printf("DEBUG1 - initialization - JOIN1: %s%03d_%03d.%s\n", IMAGE_FILENAME_PREFIX_INIT, mpi_size, i, FILE_EXTENSION_PGMPART);
            fn[i] = (char *) malloc(
                    strlen(IMAGE_FILENAME_PREFIX_INIT) + strlen("_000_000.") + strlen(FILE_EXTENSION_PGMPART) + 1);
            sprintf(fn[i], "%s_%03d_%03d.%s", IMAGE_FILENAME_PREFIX_INIT, mpi_size, i, FILE_EXTENSION_PGMPART);
        }
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - initialization - JOIN2: %s\n", fn[i]);
        // delete if already existing
        int remove_result = remove(pathname);
        if (debug_info > 0)
            printf("DEBUG1 - initialization - remove_result: %d\n", remove_result);
        for (int i = 0; i < mpi_size; i++)
            t_io += file_chunk_merge(pathname, fn[i], debug_info); // TODO: manage error result
        // delete chunks but keep them in debug mode
        if (debug_info == 0)
            for (int i = 0; i < mpi_size; i++)
                remove(fn[i]);
        for (int i = 0; i < mpi_size; i++)
            free(fn[i]); // TODO: verify
        // using system commands to join chunks:
        //char *command = (char *) malloc(50);
        //sprintf(command, "cat <fileprefix>* > %s", pathname);
        //printf("command: %s\n", command);
        //system(command);
        //sprintf(command, "rm <fileprefix>*");
        //printf("command: %s\n", command);
        //system(command);
        //free(command);
        double t_io_other = 0;
        for (int i = 1; i < mpi_size; i++) {
            MPI_Recv(&t_io_other, 1, MPI_DOUBLE, i, TAG_T, MPI_COMM_WORLD, &mpi_status);
            t_io_accumulator += t_io_other;
            if (debug_info > 1)
                printf("DEBUG2 - run_wave ACCU1 - i=%d, t_io_other=%f, t_io_accumulator=%f\n", i, t_io_other, t_io_accumulator);
        }
        if (debug_info > 0)
            printf("DEBUG1 - run_wave ACCU2 - t_io=%f, t_io_other=%f, t_io_accumulator=%f, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1)=%f\n", t_io, t_io_other, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
    }
    MPI_Finalize();
    if (mpi_rank == 0) {
        //printf("Initialization completed, data written to file %s\n", pathname);
        printf("mpi=%d, omp=%d, total time=%f, I/O time=%f, I/O time t_io_accumulator=%f, t_io_accumulator mean=%f\n", mpi_size, omp_get_max_threads(), MPI_Wtime() - t_start, t_io, t_io_accumulator, mpi_size == 1 ? 0 : t_io_accumulator / (mpi_size - 1));
    }
    if (debug_info > 0)
        printf("DEBUG1 - initialization - END - rank %d/%d\n", mpi_rank, mpi_size);
}
