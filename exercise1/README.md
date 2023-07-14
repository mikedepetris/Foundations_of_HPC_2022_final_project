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


#### Manually builng the executable
cd /u/dssc/mdepet00/assignment/exercise1  
salloc -n 1 -N1 -p EPYC --time=1:0:0  
	(remember to exit when done)  
	salloc -n 128 -N1 -p THIN --time=1:0:0  
		(salloc -n1 -t 1:00:00 -p THIN)  
module load architecture/AMD  
	(module load architecture/Intel)  
module load openMPI/4.1.4/gnu/12.2.1  
    ... edit the makefile  
srun -n1 make all  
	(srun -n1 make clean)  

#### Useful commands
alias cd1="cd /u/dssc/mdepet00/assignment/exercise1"  
alias cd2="cd /u/dssc/mdepet00/assignment/exercise2"  
chmod +x *.sh  
history|grep sinfo  
history|grep -- -k (to display past invocations with -k parameter)  
!<id> (to recall command line from history)  
salloc -n 128 -N1 -p EPYC --time=2:0:0  
salloc -n 24 -N1 -p THIN --time=2:0:0  
exit  
watch '<command>’  
sinfo -N --format="%.15N %.6D %.10P %.11T %.4c %.10z %.8m %.10e %.9O %.15C"'  
squeue --format="%.6i %.9P %.22j %.10u %.10M %.6D %.5C %.7m %.93R %.8T %l %L"  
w

#### Help printed by the program
Usage: /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x [options]  
 -i initialize a playground  
 -r run a playground (needs -f)  
 -k <num> playground size (default value 10000, minimum value 100)  
 -e [0|1|2|3] evolution type (0: ordered, 1: static, 2: wave, 3: white-black  
 -f <string> filename to be written (new_playground) or read (run)  
 -n <num> number of steps to be iterated (default value 100, max 99999)  
 -s <num> number of steps between two dumps (default 0: print only last state)  
 -h print help about program usage  
 -q print csv output  
 -D print debug informations  
 -D2 print advanced debug informations  
  
Examples:  
     to create initial conditions:  
         gameoflife.x -i -k 10000 -f initial_condition  
     this produces a file named initial_condition.pgm that contains a playground of size 10000x10000  
     to run a play ground:  
     gameoflife.x -r -f initial_condition.pgm -n 10000 -e 1 -s 1  
     this evolves the initial status of the file initial_condition.pgm for 10000 steps  
     with the static evolution mode, producing a snapshot at every time-step  
  
Parallel execution:  
     mpirun -np 1 gameoflife.x -i -k 100 -f pattern_random  
     mpirun -np 4 gameoflife.x -r -f pattern_random -n 3 -e 1 -s 0  

#### Examples of running in wsl
sudo apt install openmpi-bin libopenmpi-dev

patterns/  
    still_lifes/  
        pattern_block.pgm  
        pattern_loaf6.pgm  
    oscillators/  
        pattern_blinker.pgm  
        pattern_blinker32.pgm   
        pattern_pulsar32.pgm  
    spaceships/  
        pattern_ship32.pgm  

mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -i -k 5 -f pattern_random5  
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -i -k 6 -f pattern_random6  
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_block.pgm  -n 2000 -e 1 -s1  
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_loaf6.pgm  -n 2000 -e 1 -s1  
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_blinker5.pgm -n 2000 -e 1 -s1  
mpirun -np 1 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f patterns/oscillators/pattern_blinker.pgm -n 2000 -e 1 -s1  
mpirun -np 4 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_pulsar32.pgm -n 2000 -e 1 -s1  
mpirun -np 4 /mnt/c/dev/HPC/gameoflife/cmake-build-debug/gameoflife.x -r -f pattern_ship32.pgm -n 2000 -e 1 -s1  

#### Examples of running on ORFEO
sbatch -J scale_omp2_1_0_1000 scale_omp_2.sh 1 0 1000  
sbatch -J scale_omp4_1_0_1000 scale_omp_4.sh 1 0 1000  
sbatch -J scale_omp4_1_0_1000 scale_omp_4.sh 1 0 1000  
sbatch -J scale_omp3_1_0_1000 scale_omp_3.sh 1 0 1000  
sbatch -J scale_omp2thin_1_0_1000 scale_omp_2_thin.sh 1 0 1000  

#### Video animations creation from image sequences
Imagemagick command line tool:  
The tool can be installed as a package, and then run specifying the delay between frames in milliseconds, instructing to repeat the animation in a loop, taking all the “pgm” snapshot files as input, and writing to an output “ship.gif” animated gif:  
  
sudo apt install imagemagick-6.q16  
convert -delay 20 -loop 0 *.pgm ship.gif  
  
ffmpeg:  
ffmpeg tool can be installed and run, this time producing an MP4 video file. Using a video file codec, the result is a video with soft borders, that may give a more appreciable display effect, but that can hide the actual information about the alive cells.  
A slideshow video with one image per second can be produced by:  
  
ffmpeg -framerate 1 -pattern_type glob -i '*.pgm' -c:v libx264 -r   
30 -pix_fmt yuv420p out.mp4  
