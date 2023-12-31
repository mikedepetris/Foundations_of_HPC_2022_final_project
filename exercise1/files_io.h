
//
// Created by mikedepetris on 12/05/2023.
//

#ifndef GAMEOFLIFE_FILES_IO_H
#define GAMEOFLIFE_FILES_IO_H

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <omp.h>

// activate debug code
//#define DEBUG1
//#define DEBUG2
//#define DEBUG_ADVANCED_MALLOC_FREE
//#define DEBUG_ADVANCED_B
//#define DEBUG_ADVANCED_COORDINATES

#define PGM_COMMENT " Created by GOL gameoflife"

// calculate the first row, last row and the total number of rows of each MPI Task
void calculate_sizes_indexes(int mpi_rank, int mpi_size, long world_size, long *first_row, long *last_row, long *local_size);

double make_directory(char *directory_name, int debug_info);

//double file_pgm_write_all(void *image, int maxval, int xsize, int ysize, const char *image_name);

double file_pgm_write_chunk(unsigned char *world_local, int maxval, long world_size, long local_size, const char *directoryname, const char *image_filename_prefix, const char *image_filename_suffix, const char *image_filename_extension
                            , int mpi_rank, int mpi_size, int debug_info);

double file_pgm_write_chunk_noghost(unsigned char *world_local, int maxval, long world_size, long local_size, const char *directoryname, const char *image_filename_prefix, const char *image_filename_suffix, const char *image_filename_extension
                                    , int mpi_rank, int mpi_size, int debug_info);

double file_pgm_read(unsigned char **world, int *maxval, long *local_size, long *world_size, const char *image_filename, int mpi_rank, int mpi_size, int debug_info);

double file_pgm_read_noghost(unsigned char **world, int *maxval, long *local_size, long *world_size, const char *image_filename, int mpi_rank, int mpi_size, int debug_info);

double file_chunk_merge(const char *filename1, const char *filename2, int debug_info);

#endif //GAMEOFLIFE_FILES_IO_H
