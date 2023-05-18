#include "initialize.h"
#include "gameoflife.h"

int get_unique_seed(int omp_rank, int mpi_rank) {
    return (int) (123 + omp_rank * mpi_rank * 7 + 8 * omp_rank * omp_rank + 9 * mpi_rank * mpi_rank * time(NULL));
}

void initialize_parallel(long total_size, int mpi_size, int mpi_rank, int debug_info) {
    long chunk_size; // chunk of rows of each MPI task: world_chunk rows / number of threads (mpi_size)
    chunk_size = total_size % mpi_size - mpi_rank <= 0 ? (long) (total_size / mpi_size) : (long) (total_size / mpi_size) + 1;
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
    write_pgm_image_chunk(world_chunk, 255, total_size, chunk_size, "", IMAGE_FILE_CHUNK_INIT_PREFIX, "", FILE_EXTENSION_PGMPART, mpi_rank, mpi_size
                          , debug_info);
    free(world_chunk);
    if (debug_info > 0)
        printf("DEBUG1 - initialize_parallel - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
}

void initialize_serial(long total_size, int debug_info) {
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
    write_pgm_image_chunk(world, 255, total_size, total_size, "", IMAGE_FILE_CHUNK_INIT_PREFIX, "", FILE_EXTENSION_PGMPART, 0, 1, debug_info);
    free(world);
    if (debug_info > 0)
        printf("DEBUG1 - initialize_serial - END\n");
}

void initialization(long world_size, const char *filename, int *argc, char ***argv, int mpi_rank, int mpi_size, int debug_info) {
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
        initialize_serial(world_size, debug_info);
    else
        initialize_parallel(world_size, mpi_size, mpi_rank, debug_info);
    MPI_Barrier(MPI_COMM_WORLD);
    // Join the chunks into a single image file
    //        init_004_000.pgmpart
    //        init_004_001.pgmpart
    //        init_004_002.pgmpart
    //        init_004_003.pgmpart
    if (mpi_rank == 0) {
        char *fn[mpi_size];
        for (int i = 0; i < mpi_size; i++) {
            if (debug_info > 0)
                printf("DEBUG1 - initialization - JOIN1: %s_%03d_%03d.%s\n", IMAGE_FILE_CHUNK_INIT_PREFIX, mpi_size, i, FILE_EXTENSION_PGMPART);
            fn[i] = (char *) malloc(sizeof(IMAGE_FILE_CHUNK_INIT_PREFIX) + 16 + 1);
            sprintf(fn[i], "%s%03d_%03d.%s", IMAGE_FILE_CHUNK_INIT_PREFIX, mpi_size, i, FILE_EXTENSION_PGMPART);
        }
        if (debug_info > 0)
            for (int i = 0; i < mpi_size; i++)
                printf("DEBUG1 - initialization - JOIN2: %s\n", fn[i]);
        // delete if already existing
        int remove_result = remove(pathname);
        if (debug_info > 0)
            printf("DEBUG1 - initialization - remove_result: %d\n", remove_result);
        for (int i = 0; i < mpi_size; i++)
            file_merge(pathname, fn[i], debug_info); // TODO: manage error result
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
    }
    MPI_Finalize();
    if (mpi_rank == 0)
        printf("Initialization completed, data written to file %s\n", pathname);
    if (debug_info > 0)
        printf("DEBUG1 - initialization - END - rank %d/%d\n", mpi_rank, mpi_size);
}
