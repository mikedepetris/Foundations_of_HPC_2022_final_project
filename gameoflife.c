/**
 * Author: Mike De Petris
 * Date: February 12, 2023
 * Purpose: parallel version of a variant of the famous Conway's "game of life"
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "initialize.h"
#include "gameoflife.h"

#define DEFAULT_WORLD_SIZE 10000
#define DEFAULT_NUMBER_OF_STEPS 100

// actions
#define HELP 0
#define INIT 1
#define RUN 2

// evolution variant
#define EVOLUTION_ORDERED 0
#define EVOLUTION_STATIC 1
#define EVOLUTION_WAVE 2
#define EVOLUTION_WHITEBLACK 3

/**
 * map of the command line arguments to vars
 *    -i initialize a playground
 *    -r run a playground
 *    -k num. value playground size
 *    -e [0|1] evolution type; 0 "ordered", 1 "static", 2 "wave", 3 "black-white"
 *    -f string the name of the file to be either read or written
 *    -n num. value number of steps to be calculated
 *    -s num. value every how many steps a dump of the system is saved on a file
 *       (0 meaning only at the end)
 *
 * arg_action_to_take: action to be performed (HELP, INIT or RUN)
 * k: number of rows and columns of the image
 * e: evolution type (EVOLUTION_ORDERED, EVOLUTION_STATIC, EVOLUTION_WAVE, EVOLUTION_WHITEBLACK)
 * f: name of the file to be read or written (REQUIRED!)
 * n: number of evolutions
 * s: save the image every "s" evolutions
 */
int debug_info = 0;
int csv_output = CSV_OUTPUT_FALSE;
int arg_action_to_take = HELP;
long world_size = DEFAULT_WORLD_SIZE;
int number_of_steps = DEFAULT_NUMBER_OF_STEPS;
int number_of_steps_between_file_dumps = 0;
int evolution_type = EVOLUTION_STATIC;
char *filename;
char is_defined_filename = 0;

/**
 * Given a the argc (number of arguments) and argv (array of arguments) of the main function
 * this function parses the arguments and sets the global variables accordingly.
 *
 * @param argc number of arguments
 * @param argv array of arguments
 */
void get_arguments_util(int argc, char *argv[]) {
    char *optstring = "qD::hirk:f:n:e:s:";
    int c;
    //printf(" ");
    while ((c = getopt(argc, argv, optstring)) != -1) {
//        printf("DEBUG - getopt c=%c\n", c);
//        if (optarg)
//            printf("DEBUG - optarg=%s\n", optarg);
        switch (c) {
            case 'q':
                csv_output = CSV_OUTPUT_TRUE;
                break;
            case 'D':
                debug_info = 1;
                if (optarg)
                    debug_info = atoi(optarg);
                if (debug_info > 1)
                    printf("DEBUG1 - debug level=%d\n", debug_info);
                break;
            case 'h':
                arg_action_to_take = HELP;
                break;
            case 'i':
                arg_action_to_take = INIT;
                break;
            case 'r':
                arg_action_to_take = RUN;
                break;
            case 'k':
                world_size = atoi(optarg);
                break;
            case 'e':
                //printf("DEBUG - case 'e'\n");
                if (optarg)
                    evolution_type = atoi(optarg);
                else
                    printf("ERROR - missing evolution type number for -e, value set to %d\n", evolution_type);
                //printf("DEBUG - case 'e': evolution_type=%d\n", evolution_type);
                break;
            case 'f':
                filename = (char *) malloc(strlen(optarg) + 1);
                sprintf(filename, "%s", optarg);
                is_defined_filename = 1;
                break;
            case 'n':
                number_of_steps = atoi(optarg);
                break;
            case 's':
                number_of_steps_between_file_dumps = atoi(optarg);
                break;
            default :
                printf("ERROR - unknown argument: -%c\n", c);
                break;
        }
    }
}

void mpi_init(int *argc, char ***argv, int *mpi_rank, int *mpi_size) {
    MPI_Init(argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, mpi_size);
}

int main(int argc, char *argv[]) {
    // Parse run-time arguments
    get_arguments_util(argc, argv);

    if (number_of_steps_between_file_dumps == 0)
        number_of_steps_between_file_dumps = 100000;

    // Check if there are no arguments or help needed
    // N.B. for each process
    if (argc <= 1 || (argc == 2 && debug_info > 0) || arg_action_to_take == HELP) {
        int mpi_rank, mpi_size;
        mpi_init(&argc, &argv, &mpi_rank, &mpi_size);
        if (mpi_rank != 0) {
            if (debug_info > 0)
                printf("DEBUG1 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
        } else {
            if (debug_info == 1) {
                printf("DEBUG1 - mpi_rank=%d/%d\n", mpi_rank, mpi_size);
                printf("DEBUG1 - Number of arguments: %d\n", argc);
                printf("DEBUG1 - Argument 0: %s\n", argv[0]);
                for (int i = 0; i < argc; i++)
                    printf("DEBUG1 - argument[%d]=%s\n", i, argv[i]);
                printf("\n");
            }
        }
        if (mpi_rank == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("      -i  initialize a playground                                                    \n");
            printf("      -r  run a playground (needs -f)                                                \n");
            printf("      -k  <num> playground size (default value %d)                                   \n", DEFAULT_WORLD_SIZE);
            printf("      -e  [0|1|2|3] evolution type (0: ordered, 1: static, 2: wave, 3: black-white   \n");
            printf("      -f  <string> filename to be written (initialization) or read (run)             \n");
            printf("      -n  <num> number of steps to be iterated (default value %d, max %d)            \n", DEFAULT_NUMBER_OF_STEPS, MAX_NUMBER_OF_STEPS);
            printf("                                                                                     \n");
            printf("      -s  <num> number of steps between two dumps (default 0: print only last state) \n");
            printf("      -h  print help about program usage                                             \n");
            printf("      -q  print csv output                                                           \n");
            printf("      -D  print debug informations                                                   \n");
            printf("      -D2 print advanced debug informations                                          \n");
            printf("                                                                                     \n");
            printf("Examples:                                                                            \n");
            printf("    to create initial conditions:                                                    \n");
            printf("        gameoflife.x -i -k 10000 -f initial_condition                                \n");
            printf("    this produces a file named initial_condition.pgm that contains a playground      \n");
            printf("    of size 10000x10000                                                              \n");
            printf("    to run a play ground:                                                            \n");
            printf("        gameoflife.x -r -f initial_condition.pgm -n 10000 -e 1 -s 1                  \n");
            printf("    this evolves the initial status of the file initial_condition.pgm for 10000 steps\n");
            printf("    with the static evolution mode, producing a snapshot at every time-step          \n");
            printf("                                                                                     \n");
            printf("Parallel execution:                                                                  \n");
            printf("    mpirun -np 1 gameoflife.x -i -k 100 -f pattern_random                            \n");
            printf("    mpirun -np 4 gameoflife.x -r -f pattern_random -n 3 -e 1 -s 0                    \n");
        }
        MPI_Finalize();
        return 0;
    } else if (arg_action_to_take == INIT) {
        int mpi_rank, mpi_size;
        mpi_init(&argc, &argv, &mpi_rank, &mpi_size);
        if (mpi_rank == 0 && csv_output == CSV_OUTPUT_FALSE)
            printf("Initialization request with world size=%ld and filename=%s\n", world_size, filename);
        initialization(world_size, filename, &argc, &argv, mpi_rank, mpi_size, csv_output, debug_info);
        if (debug_info > 0 && mpi_rank == 0)
            printf("DEBUG1 - initialization request - END\n");
    } else if (arg_action_to_take == RUN && evolution_type == EVOLUTION_STATIC) {
        if (debug_info > 1)
            printf("DEBUG2 - run_static request\n");
        run_static(filename, number_of_steps, number_of_steps_between_file_dumps, &argc, &argv, csv_output, debug_info);
    } else if (arg_action_to_take == RUN && evolution_type == EVOLUTION_ORDERED) {
        if (debug_info > 1)
            printf("DEBUG2 - Run request with EVOLUTION_ORDERED of %d steps of filename=%s\n", number_of_steps, filename);
        run_ordered(filename, number_of_steps, number_of_steps_between_file_dumps, &argc, &argv, csv_output, debug_info);
        if (debug_info > 0)
            printf("DEBUG1 - run EVOLUTION_ORDERED request - END\n");
    } else if (arg_action_to_take == RUN && evolution_type == EVOLUTION_WAVE) {
        if (debug_info > 1)
            printf("DEBUG2 - Run request with EVOLUTION_WAVE of %d steps of filename=%s\n", number_of_steps, filename);
        run_wave(filename, number_of_steps, number_of_steps_between_file_dumps, &argc, &argv, csv_output, debug_info);
        if (debug_info > 0)
            printf("DEBUG1 - run EVOLUTION_WAVE request - END\n");
    } else if (arg_action_to_take == RUN && evolution_type == EVOLUTION_WHITEBLACK) {
        if (debug_info > 1)
            printf("DEBUG2 - Run request with EVOLUTION_WHITEBLACK evolution of %d steps of filename=%s\n", number_of_steps, filename);
        run_whiteblack(filename, number_of_steps, number_of_steps_between_file_dumps, &argc, &argv, csv_output, debug_info);
        if (debug_info > 0)
            printf("DEBUG1 - run EVOLUTION_WHITEBLACK request - END\n");
    }

    if (debug_info > 1)
        printf("DEBUG2 - before END\n");
    if (is_defined_filename)
        free(filename);
    if (debug_info > 1)
        printf("DEBUG2 - END\n");
}

void strreplace(char *string, const char *find, const char *replaceWith) {
    if (strstr(string, find) != NULL) {
        char *temporaryString = malloc(strlen(strstr(string, find) + strlen(find)) + 1);
        strcpy(temporaryString, strstr(string, find) + strlen(find));    //Create a string with what's after the replaced part
        *strstr(string, find) = '\0';    //Take away the part to replace and the part after it in the initial string
        strcat(string, replaceWith);    //Concat the first part of the string with the part to replace with
        strcat(string, temporaryString);    //Concat the first part of the string with the part after the replaced part
        free(temporaryString);    //Free the memory to avoid memory leaks
    }
}

char *replace_char(char *str, char find, char replace) {
    char *current_pos = strchr(str, find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos, find);
    }
    return str;
}
