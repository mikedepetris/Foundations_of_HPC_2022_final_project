//
// Created by mikedepetris on 14/05/2023.
//

#ifndef GAMEOFLIFE_GAMEOFLIFE_H
#define GAMEOFLIFE_GAMEOFLIFE_H

// activate debug code
//#define DEBUG1
//#define DEBUG2
//#define DEBUG_ADVANCED_MALLOC_FREE
//#define DEBUG_ADVANCED_B
//#define DEBUG_ADVANCED_COORDINATES

// max iterations, must print as nnnnn
#define MAX_NUMBER_OF_STEPS 99999

// cell status
#define DEAD 255
#define ALIVE 0

#define CSV_OUTPUT_FALSE 0
#define CSV_OUTPUT_TRUE 1

#define FILE_EXTENSION_PGM "pgm"
#define FILE_EXTENSION_PGMPART "pgmpart"

#define CPU_TIME (clock_gettime( CLOCK_MONOTONIC, &ts ), (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

#define TAG_0 0
#define TAG_1 1
#define TAG_X 65365
#define TAG_T 65364

//#define SNAPSHOT_00000 "snapshot_00000"

//void replace_str(char *string, const char *find, const char *replaceWith);

char *replace_char(char *str, char find, char replace);

void new_playground(long world_size, const char *filename, int *argc, char ***argv, int mpi_rank, int mpi_size, int csv_output, int debug_info);

void evolution_static(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info);

void evolution_ordered(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info);

void evolution_wave(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info);

void evolution_whiteblack(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int csv_output, int debug_info);

#endif //GAMEOFLIFE_GAMEOFLIFE_H
