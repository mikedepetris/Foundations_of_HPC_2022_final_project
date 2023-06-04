#include <sys/stat.h>
#include <string.h>
#include "read_write_pgm.h"

/*
* Create directory if it doesn't exist
*/
double make_directory(char *directory_name, int debug_info) {
    if (debug_info == 1)
        printf("DEBUG1 - make_directory: %s\n", directory_name);
    struct stat st = {0};
    double t_point = MPI_Wtime();
    if (stat(directory_name, &st) == -1)
        mkdir(directory_name, 0700);
    return MPI_Wtime() - t_point;
}

void calculate_sizes_indexes(int mpi_rank, int mpi_size, long world_size, long *first_row, long *last_row, long *local_size) {
    *local_size = world_size % mpi_size - mpi_rank <= 0 ? (long) (world_size / mpi_size) : (long) (world_size / mpi_size) + 1;
    *first_row = world_size % mpi_size - mpi_rank >= 0 ?
                 mpi_rank * (long) (world_size / mpi_size) + mpi_rank :
                 mpi_rank * (long) (world_size / mpi_size) + world_size % mpi_size;
    *last_row = (*first_row) + (*local_size) - 1;
}

//double file_pgm_write_all(void *image, int maxval, int xsize, int ysize, const char *image_name) {
//    FILE *image_file;
//    image_file = fopen(image_name, "w");
//    int color_depth = 1 + (maxval > 255);
//    fprintf(image_file, "P5\n#" PGM_COMMENT "\n%d %d\n%d\n", xsize, ysize, maxval);
//    fwrite(image, 1, xsize * ysize * color_depth, image_file);
//    fclose(image_file);
//}

double file_pgm_write_chunk(unsigned char *world_local, int maxval, long world_size, long local_size, const char *directoryname, const char *image_filename_prefix, const char *image_filename_suffix, const char *image_filename_extension, int mpi_rank
                            , int mpi_size, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - BEGIN - mpi_rank=%d/%d, directoryname=%s, image_filename_prefix=%s, image_filename_suffix=%s, image_filename_extension=%s\n", mpi_rank, mpi_size, directoryname, image_filename_prefix, image_filename_suffix
               , image_filename_extension);
    if (debug_info > 1) {
        for (long long i = 0; i < world_size * (local_size + 2); i++) {
            if (i % 16 == 0)
                printf("DEBUG2 - file_pgm_write_chunk 0 - %d/%d %08X: ", mpi_rank, mpi_size, (unsigned int) i);
            printf("%02X ", world_local[i]);
            if (i % 16 == 15)
                printf("\n");
        }
        printf("\n");
    }
    int color_depth = 1 + (maxval > 255);
    FILE *chunk_file = NULL;
    // TODO: calculate correct string size
    char *file_name = (char *) malloc(256);

    if (mpi_size > 1)
        // add slash to directoryname if specified
        if (directoryname && strlen(directoryname) > 0)
            sprintf(file_name, "%s/%s_%03d_%03d%s.%s", directoryname, image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
        else
            sprintf(file_name, "%s_%03d_%03d%s.%s", image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
    else if (directoryname && strlen(directoryname) > 0)
        sprintf(file_name, "%s/%s%s.%s", directoryname, image_filename_prefix, image_filename_suffix, image_filename_extension);
    else
        sprintf(file_name, "%s%s.%s", image_filename_prefix, image_filename_suffix, image_filename_extension);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - file_name=%s\n", file_name);
//    // dupes required or file_name would be changed
//    char *ts1 = strdup(file_name);
//    char *ts2 = strdup(file_name);
//    char *dir_name = dirname(ts1);
//    char *base_name = basename(ts2);
//    if (debug_info > 0)
//        printf("DEBUG1 - file_pgm_write_chunk - file_name=%s, dir_name=%s, base_name=%s\n", file_name, dir_name, base_name);
//    make_directory(dir_name, debug_info);
    double t_time_point = MPI_Wtime();
    chunk_file = fopen(file_name, "w");
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 0 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (chunk_file == NULL) {
        printf("ERROR in file_pgm_write_chunk - the file %s could not be opened\n", file_name);
        perror("ERROR in file_pgm_write_chunk - the file could not be opened\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 1 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    // write pgm header in first chunk
    if (mpi_rank == 0)
        //printf("P5\n#" PGM_COMMENT "\n%ld %ld\n%d\n", world_size, world_size, maxval);
        fprintf(chunk_file, "P5\n#" PGM_COMMENT "\n%ld %ld\n%d\n", world_size, world_size, maxval);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 2 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (ferror(chunk_file) != 0)
        printf("ERROR file_pgm_write_chunk - chunk_file error occurred while writing file_name=%s\n", file_name);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 3 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - ferror(chunk_file)=%d\n", ferror(chunk_file));
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 4 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    fwrite(&world_local[world_size], 1, world_size * local_size * color_depth, chunk_file);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - 5 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (ferror(chunk_file) != 0)
        printf("ERROR file_pgm_write_chunk - chunk_file error occurred while writing file_name=%s\n", file_name);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - ferror(chunk_file)=%d\n", ferror(chunk_file));
    fclose(chunk_file);
    t_io += MPI_Wtime() - t_time_point;
    free(file_name);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    return t_io;
}

double file_pgm_write_chunk_noghost(unsigned char *world_local, int maxval, long world_size, long local_size, const char *directoryname, const char *image_filename_prefix, const char *image_filename_suffix, const char *image_filename_extension
                                    , int mpi_rank, int mpi_size, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - BEGIN - mpi_rank=%d/%d, directoryname=%s, image_filename_prefix=%s, image_filename_suffix=%s, image_filename_extension=%s\n", mpi_rank, mpi_size, directoryname, image_filename_prefix
               , image_filename_suffix, image_filename_extension);
    int color_depth = 1 + (maxval > 255);
    if (debug_info > 1) {
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 0 - mpi_rank=%d/%d, world_size=%ld, local_size=%ld, color_depth=%d\n", mpi_rank, mpi_size, world_size, local_size, color_depth);
        for (long long i = 0; i < world_size * local_size; i++) {
            if (i % 16 == 0)
                printf("DEBUG2 - file_pgm_write_chunk_noghost 0 - %d/%d %08X: ", mpi_rank, mpi_size, (unsigned int) i);
            printf("%02X ", world_local[i]);
            if (i % 16 == 15)
                printf("\n");
        }
        printf("\n");
    }
    FILE *chunk_file = NULL;
    // TODO: calculate correct string size
    char *file_name = (char *) malloc(256);

    if (mpi_size > 1)
        // add slash to directoryname if specified
        if (directoryname && strlen(directoryname) > 0)
            sprintf(file_name, "%s/%s_%03d_%03d%s.%s", directoryname, image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
        else
            sprintf(file_name, "%s_%03d_%03d%s.%s", image_filename_prefix, mpi_size, mpi_rank, image_filename_suffix, image_filename_extension);
    else if (directoryname && strlen(directoryname) > 0)
        sprintf(file_name, "%s/%s%s.%s", directoryname, image_filename_prefix, image_filename_suffix, image_filename_extension);
    else
        sprintf(file_name, "%s%s.%s", image_filename_prefix, image_filename_suffix, image_filename_extension);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - file_name=%s\n", file_name);
//    // dupes required or file_name would be changed
//    char *ts1 = strdup(file_name);
//    char *ts2 = strdup(file_name);
//    char *dir_name = dirname(ts1);
//    char *base_name = basename(ts2);
//    if (debug_info > 0)
//        printf("DEBUG1 - file_pgm_write_chunk_noghost - file_name=%s, dir_name=%s, base_name=%s\n", file_name, dir_name, base_name);
//    make_directory(dir_name, debug_info);
    double t_time_point = MPI_Wtime();
    chunk_file = fopen(file_name, "w");
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 0 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (chunk_file == NULL) {
        printf("ERROR in file_pgm_write_chunk_noghost - the file %s could not be opened\n", file_name);
        perror("ERROR in file_pgm_write_chunk_noghost - the file could not be opened\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 1 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    // write pgm header in first chunk
    if (mpi_rank == 0)
        //printf("P5\n#" PGM_COMMENT "\n%ld %ld\n%d\n", world_size, world_size, maxval);
        fprintf(chunk_file, "P5\n#" PGM_COMMENT "\n%ld %ld\n%d\n", world_size, world_size, maxval);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 2 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (ferror(chunk_file) != 0)
        printf("ERROR file_pgm_write_chunk_noghost - chunk_file error occurred while writing file_name=%s\n", file_name);
    if (debug_info > 0) {
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 3 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
        printf("DEBUG1 - file_pgm_write_chunk_noghost - ferror(chunk_file)=%d\n", ferror(chunk_file));
    }
    if (debug_info > 1)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 4 - mpi_rank=%d/%d, world_size=%ld, local_size=%ld, color_depth=%d\n", mpi_rank, mpi_size, world_size, local_size, color_depth);
    fwrite(world_local, 1, world_size * local_size * color_depth, chunk_file);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - 5 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    if (ferror(chunk_file) != 0)
        printf("ERROR file_pgm_write_chunk_noghost - chunk_file error occurred while writing file_name=%s\n", file_name);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - ferror(chunk_file)=%d\n", ferror(chunk_file));
    fclose(chunk_file);
    t_io += MPI_Wtime() - t_time_point;
    free(file_name);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_write_chunk_noghost - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    return t_io;
}

double file_pgm_read(unsigned char **world, int *maxval, long *local_size, long *world_size, const char *image_filename, int mpi_rank, int mpi_size, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_read - BEGIN - mpi_rank=%d/%d, image_filename=%s\n", mpi_rank, mpi_size, image_filename);
    size_t number_of_read_chars = 0;
    MPI_Status mpi_status;
    long first_row, last_row;
    double t_time_point = MPI_Wtime();
    if (mpi_rank == 0) {
        FILE *image_file;
        //printf("BEFORE fopen file=%s\n", image_filename);
        image_file = fopen(image_filename, "r");
        //printf("AFTER fopen file=%s image_file=%p\n", image_filename, image_file);
        if (!image_file || ferror(image_file) != 0) {
            printf("ERROR occurred while reading file=%s\n", image_filename);
            perror("Error occurred while reading file\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        *world = NULL;
        *world_size = *local_size = *maxval = 0;
        char magic_number_p5[3];
        size_t number_of_matches;
        number_of_matches = fscanf(image_file, "%2s%*c", magic_number_p5);
        number_of_read_chars += number_of_matches * strlen(magic_number_p5);
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read - magic_number_p5=%s, number_of_matches=%zu, number_of_read_chars=%zu\n", magic_number_p5, number_of_matches, number_of_read_chars);
        char *line;
        size_t line_buffer_size = strlen("# " PGM_COMMENT "\n");
        line = (char *) malloc(line_buffer_size * sizeof(char));
        if (line == NULL) {
            perror("Unable to allocate buffer\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        size_t number_of_chars = getline(&line, &line_buffer_size, image_file);
        number_of_read_chars += number_of_chars;
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read - number_of_chars=%zu, line=%s, strlen(line)=%zu, line_buffer_size=%zu, line_buffer_size * sizeof(char)=%zu, number_of_read_chars=%zu\n", number_of_chars, line, strlen(line), line_buffer_size,
                    line_buffer_size * sizeof(char), number_of_read_chars);
        // skip comments
        while ((number_of_chars > 0) && (line[0] == '#')) {
            number_of_chars = getline(&line, &line_buffer_size, image_file);
            number_of_read_chars += number_of_chars;
        }
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read - after skipping comments number_of_chars=%zu, number_of_read_chars=%zu\n", number_of_chars, number_of_read_chars);
        long temp;
        if (number_of_chars > 0) {
            // maxval is used to store unused width side, we consider square world only world_size x world_size
            number_of_matches = sscanf(line, "%ld%*c%ld%*c%d%*c", &temp, world_size, maxval);
            if (number_of_matches < 3)
                number_of_chars = getline(&line, &line_buffer_size, image_file);
            number_of_read_chars += number_of_chars;
            number_of_matches = sscanf(line, "%d%*c", maxval);
        } else {
            *maxval = -1;
            free(line);
            return 0;
        }
        if (debug_info > 1)
//            printf("DEBUG2 - file_pgm_read - HEADER END "
//                   "number_of_chars=%zu, line=%s, strlen(line)=%zu, line_buffer_size=%zu, line_buffer_size * sizeof(char)=%zu, number_of_read_chars=%zu, world_size=%ld, maxval=%d\n"
//                   , number_of_chars, line, strlen(line), line_buffer_size, line_buffer_size * sizeof(char), number_of_read_chars, *world_size, *maxval);
            printf("DEBUG2 - file_pgm_read - HEADER END "
                   "number_of_chars=%zu, strlen(line)=%zu, line_buffer_size=%zu, line_buffer_size * sizeof(char)=%zu, number_of_read_chars=%zu, world_size=%ld, maxval=%d\n"
                   , number_of_chars, strlen(line), line_buffer_size, line_buffer_size * sizeof(char), number_of_read_chars, *world_size, *maxval);
        fclose(image_file);
        t_io += MPI_Wtime() - t_time_point;
        free(line);
    }
    MPI_Bcast(&number_of_read_chars, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(world_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(maxval, 1, MPI_INT, 0, MPI_COMM_WORLD);
    calculate_sizes_indexes(mpi_rank, mpi_size, *world_size, &first_row, &last_row, local_size);
    if (debug_info > 1)
        printf("DEBUG2 - file_pgm_read - mpi_rank=%d/%d *world_size=%ld, first_row=%ld, last_row=%ld, local_size=%ld\n", mpi_rank, mpi_size, *world_size, first_row, last_row, *local_size);
    int color_depth = 1 + (*maxval > 255);
    t_time_point = MPI_Wtime();
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, image_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    unsigned int to_read_size = (*world_size) * (*local_size) * color_depth;
    MPI_File_seek(fh, (int) number_of_read_chars + 1 + first_row * *world_size, MPI_SEEK_CUR);
    if ((*world = (unsigned char *) malloc((*local_size + 2) * (*world_size) * sizeof(unsigned char))) == NULL)
        return 0;
    unsigned char *v = *world;
    MPI_File_read(fh, &v[*world_size], (int) to_read_size, MPI_UNSIGNED_CHAR, &mpi_status);
    //MPI_Barrier(MPI_COMM_WORLD);
    MPI_File_close(&fh);
    t_io += MPI_Wtime() - t_time_point;
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_read - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    return t_io;
}

double file_pgm_read_noghost(unsigned char **world, int *maxval, long *local_size, long *world_size, const char *image_filename, int mpi_rank, int mpi_size, int debug_info) {
    double t_io = 0; // returned value: total I/O time spent
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_read_noghost 0 - BEGIN - mpi_rank=%d/%d, image_filename=%s\n", mpi_rank, mpi_size, image_filename);
    size_t number_of_read_chars = 0;
    MPI_Status mpi_status;
    long first_row, last_row;
    double t_time_point = MPI_Wtime();
    if (mpi_rank == 0) {
        FILE *image_file;
        //printf("BEFORE fopen file=%s\n", image_filename);
        image_file = fopen(image_filename, "r");
        //printf("AFTER fopen file=%s image_file=%p\n", image_filename, image_file);
        if (!image_file || ferror(image_file) != 0) {
            printf("ERROR occurred while reading file=%s\n", image_filename);
            perror("Error occurred while reading file\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        *world = NULL;
        *world_size = *local_size = *maxval = 0;
        char magic_number_p5[3];
        size_t number_of_matches;
        number_of_matches = fscanf(image_file, "%2s%*c", magic_number_p5);
        number_of_read_chars += number_of_matches * strlen(magic_number_p5);
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read_noghost 1 - magic_number_p5=%s, number_of_matches=%zu, number_of_read_chars=%zu\n", magic_number_p5, number_of_matches, number_of_read_chars);
        char *line;
        size_t line_buffer_size = strlen("# " PGM_COMMENT "\n");
        line = (char *) malloc(line_buffer_size * sizeof(char));
        if (line == NULL) {
            perror("Unable to allocate buffer\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        size_t number_of_chars = getline(&line, &line_buffer_size, image_file);
        number_of_read_chars += number_of_chars;
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read_noghost 2 - number_of_chars=%zu, line=%s, strlen(line)=%zu, line_buffer_size=%zu, line_buffer_size * sizeof(char)=%zu, number_of_read_chars=%zu\n", number_of_chars, line, strlen(line), line_buffer_size,
                    line_buffer_size * sizeof(char), number_of_read_chars);
        // skip comments
        while ((number_of_chars > 0) && (line[0] == '#')) {
            number_of_chars = getline(&line, &line_buffer_size, image_file);
            number_of_read_chars += number_of_chars;
        }
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read_noghost 3 - after skipping comments number_of_chars=%zu, number_of_read_chars=%zu\n", number_of_chars, number_of_read_chars);
        long temp;
        if (number_of_chars > 0) {
            // maxval is used to store unused width side, we consider square world only world_size x world_size
            number_of_matches = sscanf(line, "%ld%*c%ld%*c%d%*c", &temp, world_size, maxval);
            if (number_of_matches < 3)
                number_of_chars = getline(&line, &line_buffer_size, image_file);
            number_of_read_chars += number_of_chars;
            number_of_matches = sscanf(line, "%d%*c", maxval);
        } else {
            *maxval = -1;
            free(line);
            return 0;
        }
        if (debug_info > 1)
            printf("DEBUG2 - file_pgm_read_noghost 4 - HEADER END "
                   "number_of_chars=%zu, strlen(line)=%zu, line_buffer_size=%zu, line_buffer_size * sizeof(char)=%zu, number_of_read_chars=%zu, world_size=%ld, maxval=%d\n"
                   , number_of_chars, strlen(line), line_buffer_size, line_buffer_size * sizeof(char), number_of_read_chars, *world_size, *maxval);
        fclose(image_file);
        t_io += MPI_Wtime() - t_time_point;
        free(line);
    }
    MPI_Bcast(&number_of_read_chars, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(world_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(maxval, 1, MPI_INT, 0, MPI_COMM_WORLD);
    calculate_sizes_indexes(mpi_rank, mpi_size, *world_size, &first_row, &last_row, local_size);
    int color_depth = 1 + (*maxval > 255);
    t_time_point = MPI_Wtime();
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, image_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    size_t to_read_size = (*world_size) * (*local_size) * color_depth;
    if (debug_info > 1)
        printf("DEBUG2 - file_pgm_read_noghost 5a - mpi_rank=%d/%d *world_size=%ld, first_row=%ld, last_row=%ld, local_size=%ld, color_depth=%d, to_read_size=%zu\n"
               , mpi_rank, mpi_size, *world_size, first_row, last_row, *local_size, color_depth, to_read_size);
    //MPI_File_seek(fh, number_of_read_chars + 1 + first_row * (*world_size), MPI_SEEK_CUR);
    int seek_result = MPI_File_seek(fh, (int) number_of_read_chars + 1 + first_row * (*world_size), MPI_SEEK_CUR);
    if (debug_info > 1)
        printf("DEBUG2 - file_pgm_read_noghost 5b - mpi_rank=%d/%d *world_size=%ld, first_row=%ld, last_row=%ld, local_size=%ld, color_depth=%d, to_read_size=%d, seek_result=%d\n"
               , mpi_rank, mpi_size, *world_size, first_row, last_row, *local_size, color_depth, (int) to_read_size, seek_result);
    if ((*world = (unsigned char *) malloc((*local_size) * (*world_size) * sizeof(unsigned char))) == NULL)
        return 0;
    if (debug_info > 1)
        printf("DEBUG2 - file_pgm_read_noghost 5c - mpi_rank=%d/%d *world_size=%ld, first_row=%ld, last_row=%ld, local_size=%ld, color_depth=%d, to_read_size=%d, seek_result=%d, malloc((*local_size) * (*world_size) * sizeof(unsigned char))=malloc(%lu)\n"
               , mpi_rank, mpi_size, *world_size, first_row, last_row, *local_size, color_depth, (int) to_read_size, seek_result, (*local_size) * (*world_size) * sizeof(unsigned char));
    MPI_File_read(fh, *world, (int) to_read_size, MPI_UNSIGNED_CHAR, &mpi_status);
    //MPI_Barrier(MPI_COMM_WORLD);
    MPI_File_close(&fh);
    t_io += MPI_Wtime() - t_time_point;
    if (debug_info > 1)
        printf("DEBUG2 - file_pgm_read_noghost 6 - mpi_rank=%d/%d *world_size=%ld, first_row=%ld, last_row=%ld, local_size=%ld, color_depth=%d, to_read_size=%zu\n"
               , mpi_rank, mpi_size, *world_size, first_row, last_row, *local_size, color_depth, to_read_size);
    if (debug_info > 0)
        printf("DEBUG1 - file_pgm_read_noghost 7 - END - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
    return t_io;
}

double file_chunk_merge(const char *filename1, const char *filename2, int debug_info) {
    if (debug_info > 0)
        printf("DEBUG1 - file_chunk_merge - appending %s to %s.\n", filename2, filename1);
    FILE *f1, *f2;
    double t_time_point = MPI_Wtime();
    f1 = fopen(filename1, "a");
    if (f1 == NULL) {
        printf("Error joining file chunks: file=%s\n", filename1);
        perror("Error joining file chunks\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    f2 = fopen(filename2, "r");
    if (f2 == NULL) {
        printf("Error joining file chunks: file=%s\n", filename2);
        perror("Error joining file chunks\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
//    // calculate f2 size
//    fseek(f2, 0, SEEK_END);
//    long size = ftell(f2);
//    rewind(f2);
//    unsigned char buffer[size]; // fails if too big
//    fread(buffer, sizeof(buffer), 1, f2);
//    fclose(f2);
//    fwrite(buffer, sizeof(buffer), 1, f1);
//    fclose(f1);

    size_t read_size;
    unsigned char buffer[4096]; // 4k buffer
    do {
        read_size = fread(buffer, 1, sizeof(buffer), f2);
        if (debug_info > 1)
            printf("DEBUG2 - file_chunk_merge - appending %s to %s. read_size=%zu\n", filename2, filename1, read_size);
        if (read_size <= 0) break;
        fwrite(buffer, read_size, 1, f1);
    } while (read_size == sizeof buffer); // if it was a full buffer, loop again
    fclose(f1);
    fclose(f2);

    return MPI_Wtime() - t_time_point;
}
