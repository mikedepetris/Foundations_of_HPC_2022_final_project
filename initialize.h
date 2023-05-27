//
// Created by mikedepetris on 13/05/2023.
//

#ifndef GAMEOFLIFE_INITIALIZE_H
#define GAMEOFLIFE_INITIALIZE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "read_write_pgm.h"

#define IMAGE_FILENAME_PREFIX_INIT "init"

void initialization(long world_size, const char *filename, int *argc, char ***argv, int mpi_rank, int mpi_size, int debug_info);

#endif //GAMEOFLIFE_INITIALIZE_H
