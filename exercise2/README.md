# Foundations of HPC 2022 final project
## exercise 2 : Comparing MKL, OpenBLAS and BLIS on matrix-matrix multiplication 

Student: Michele De Petris IN2000137

This is the final assignment for the 2022/2023 Foundations of High Performance Computing course, part of the MSc in Data Science and Scientific Computing at University of Trieste (all related material can be found at https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).
It consists of two mandatory exercises.

### Structure of this github directory: 
file                    |description
------------------------|-------------------------------------------------------------------------------------------------------------------------
results_discarded\		|all the output files that have been discarded due to execution or configuration errors
results_used\			|all the output files that have been used and collected in the spreadsheets
.gitignore				|exclusions for git
CMakeLists.txt			|make file for CLion CMake
Makefile				|make file for the compiler
README.md				|explains all the files present in the directory and gives detailed information to reproduce the experiments
dgemm.c					|original source code for the assgniment
gemm.c					|modified version of the program used for all the experiments
dgemm_epyc128.sh		|batch script to run jobs for the execution used for size scalability analysis on EPYC nodes up to 128 in double precision
dgemm_epyc64.sh			|batch script to run jobs for the execution used for size scalability analysis on EPYC nodes up to 64 in double precision
dgemm_thin12.sh			|batch script to run jobs for the execution used for size scalability analysis on THIN nodes up to 12 in double precision
dgemm_thin24.sh			|batch script to run jobs for the execution used for size scalability analysis on THIN nodes up to 24 in double precision
sgemm_epyc128.sh		|batch script to run jobs for the execution used for size scalability analysis on EPYC nodes up to 128 in single precision
sgemm_epyc64.sh			|batch script to run jobs for the execution used for size scalability analysis on EPYC nodes up to 64 in single precision
sgemm_thin12.sh			|batch script to run jobs for the execution used for size scalability analysis on THIN nodes up to 12 in single precision
sgemm_thin24.sh			|batch script to run jobs for the execution used for size scalability analysis on THIN nodes up to 24 in single precision
strong_dgemm_epyc128.sh	|batch script to run jobs for the execution used for cores scalability analysis on EPYC nodes up to 128 in double precision
strong_dgemm_epyc64.sh	|batch script to run jobs for the execution used for cores scalability analysis on EPYC nodes up to 64 in double precision
strong_dgemm_thin12.sh	|batch script to run jobs for the execution used for cores scalability analysis on THIN nodes up to 12 in double precision
strong_dgemm_thin24.sh	|batch script to run jobs for the execution used for cores scalability analysis on THIN nodes up to 24 in double precision
strong_sgemm_epyc128.sh	|batch script to run jobs for the execution used for cores scalability analysis on EPYC nodes up to 128 in single precision
strong_sgemm_epyc64.sh	|batch script to run jobs for the execution used for cores scalability analysis on EPYC nodes up to 64 in single precision
strong_sgemm_thin12.sh	|batch script to run jobs for the execution used for cores scalability analysis on THIN nodes up to 12 in single precision
strong_sgemm_thin24.sh	|batch script to run jobs for the execution used for cores scalability analysis on THIN nodes up to 24 in single precision
epyc.txt				|output of command srun lscpu>epyc.txt
thin.txt				|output of command srun lscpu>thin.txt
gemm_cores.xlsm			|spreadsheet used to collect all output from the executions for the cores scalability to produce tables and graphic charts
gemm_size.xlsm			|spreadsheet used to collect all output from the executions for the size scalability to produce tables and graphic charts

#### Examples of running on ORFEO
sbatch strong_dgemm_epyc128.sh  
sbatch strong_sgemm_epyc128.sh  
sbatch strong_dgemm_thin24.sh   
sbatch strong_sgemm_thin24.sh   
sbatch strong_dgemm_epyc128.sh  
sbatch strong_sgemm_epyc128.sh  
sbatch strong_dgemm_thin24.sh   
watch 'squeue --format="%.8i %.9P %.22j %.10u %.9M %.6D %.5C %.7m %.92R %.8T %l %L"'  
watch 'sinfo -N --format="%.15N %.6D %.10P %.11T %.4c %.10z %.8m %.10e %.9O %.15C"'  
