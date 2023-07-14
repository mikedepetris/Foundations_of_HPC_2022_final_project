# Foundations of HPC 2022 final project
## exercise 1:  parallel  programming: Game of Life

Student: Michele De Petris IN2000137

This is the final assignment for the 2022/2023 Foundations of High Performance Computing course, part of the MSc in Data Science and Scientific Computing at University of Trieste (all related material can be found at https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).
It consists of two mandatory exercises.

### Structure of this github directory: 
file                                    |description
----------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------
patterns\								|several pmg image files of well known patterns that generate interesting evolutions
results_discarded\						|all the output files that have been discarded due to execution or configuration errors
results_used\							|all the output files that have been used and collected in the spreadsheets
.gitignore								|exclusions for git
CMakeLists.txt							|make file for CLion CMake
Makefile								|make file for mpicc
evolution_ordered.c						|-e0 ordered evolution
evolution_static.c						|-e1 static evolution
evolution_wave.c						|-e2 wave evolution
evolution_whiteblack.c					|-e3 white-black evolution
files_io.c								|file reading and writing
files_io.h								|header with shared definitions for files module
gameoflife.c							|main code that parses the input and executes the operations
gameoflife.h							|header with shared definitions
new_playground.c						|initialization of a new playground with random values
README.md								|explains all the files present in the directory and gives detailed information to reproduce the experiments
merge_images.sh							|examples of using ffmpeg to create videos from image sequences qith varous settings
scale_mpi.sh							|batch script to run jobs for the execution used for the strong MPI scalability analysis on EPYC nodes
scale_mpi_thin.sh						|batch script to run jobs for the execution used for the strong MPI scalability analysis on THIN nodes
scale_omp.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on a single socket of EPYC nodes
scale_omp_2.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on two sockets of EPYC nodes
scale_omp_2_thin.sh						|batch script to run jobs for the execution used for the OpenMP scalability analysis on two sockets of THIN nodes
scale_omp_3.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on three sockets of EPYC nodes
scale_omp_4.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on four sockets of EPYC nodes
scale_omp_5.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on five sockets of EPYC nodes
scale_omp_6.sh							|batch script to run jobs for the execution used for the OpenMP scalability analysis on six sockets of THIN nodes
scale_omp_thin.sh						|batch script to run jobs for the execution used for the OpenMP scalability analysis on THIN nodes
scale_weak.sh							|batch script to run jobs for the execution used for the weak scalability analysis
calc double size squared matrix.xlsx	|spreadsheet used to compute the rows and columns numbers to obtain a linear increment of matrix sizes
omp_debug.xlsx							|spreadsheet used to inspect behaviviour of measuremens doing the OpenMP scalability test
scalability_MPI.xlsx					|spreadsheet used to collect all output from the executions on the EPYC nodes for the strong MPI scalability to produce tables and graphic charts
scalability_MPI_thin.xlsx				|spreadsheet used to collect all output from the executions on the THIN nodes for the strong MPI scalability to produce tables and graphic charts
scalability_OMP.xlsx					|spreadsheet used to collect all output from the executions on the EPYC nodes for the OpenMP scalability to produce tables and graphic charts
scalability_OMP_thin.xlsx				|spreadsheet used to collect all output from the executions on the THIN nodes for the OpenMP scalability to produce tables and graphic charts
scalability_weak.xlsx					|spreadsheet used to collect all output from the executions for the weak scalability to produce tables and graphic charts

#### Examples of running in wsl:
patterns\
    still_lifes\
        pattern_block.pgm 
		pattern_loaf6.pgm
	oscillators\
		pattern_blinker.pgm
		pattern_blinker32.pgm 
		pattern_pulsar32.pgm
	spaceships\
		pattern_ship32.pgm

mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -i -k 5 -f pattern_random5
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -i -k 6 -f pattern_random6
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_block.pgm  -n 2000 -e 1 -s1
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_loaf6.pgm  -n 2000 -e 1 -s1
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_blinker5.pgm -n 2000 -e 1 -s1
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f patterns/oscillators/pattern_blinker.pgm -n 2000 -e 1 -s1
mpirun -np 4 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_pulsar32.pgm -n 2000 -e 1 -s1
mpirun -np 4 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_ship32.pgm -n 2000 -e 1 -s1
