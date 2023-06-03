//
// Created by mikedepetris on 14/05/2023.
//

#ifndef GAMEOFLIFE_GAMEOFLIFE_H
#define GAMEOFLIFE_GAMEOFLIFE_H

// max iterations, must print as nnnnn
#define MAX_NUMBER_OF_STEPS 99999

// cell status
#define DEAD 255
#define ALIVE 0

#define FILE_EXTENSION_PGM "pgm"
#define FILE_EXTENSION_PGMPART "pgmpart"

#define CPU_TIME (clock_gettime( CLOCK_MONOTONIC, &ts ), (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

#define TAG_0 0
#define TAG_1 1
#define TAG_X 65365
#define TAG_T 65364

void strreplace(char *string, const char *find, const char *replaceWith);

char *replace_char(char *str, char find, char replace);

void run_static(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int debug_info);

void run_ordered(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int debug_info);

void run_wave(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int debug_info);

void run_whiteblack(const char *filename, int number_of_steps, int number_of_steps_between_file_dumps, int *argc, char **argv[], int debug_info);

#endif //GAMEOFLIFE_GAMEOFLIFE_H
