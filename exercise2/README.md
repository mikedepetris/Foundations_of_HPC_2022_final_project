# Foundations of HPC 2022 final project
## exercise 2 : Comparing MKL, OpenBLAS and BLIS on matrix-matrix multiplication 

Student: Michele De Petris IN2000137

This is the final assignment for the 2022/2023 Foundations of High Performance Computing course, part of the MSc in Data Science and Scientific Computing at University of Trieste (all related material can be found at https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).
It consists of two mandatory exercises.

### Structure of this github directory: 
````
.gitignore exclusions for git
CMakeLists.txt make file for CLion CMake
Makefile make file for mpicc

README.md explains all the files present in the directory and gives detailed information to reproduce the experiments

scale_mpi.sh batch script to run jobs for the execution used for the strong MPI scalability analysis on EPYC nodes
scale_mpi_thin.sh batch script to run jobs for the execution used for the strong MPI scalability analysis on THIN nodes
scale_omp.sh batch script to run jobs for the execution used for the OpenMP scalability analysis on a single socket of EPYC nodes
scalability_MPI.xlsx spreadsheet used to collect all output from the executions on the EPYC nodes for the strong MPI scalability to produce tables and graphic charts
scalability_MPI_thin.xlsx spreadsheet used to collect all output from the executions on the THIN nodes for the strong MPI scalability to produce tables and graphic charts
scalability_OMP.xlsx spreadsheet used to collect all output from the executions on the EPYC nodes for the OpenMP scalability to produce tables and graphic charts
scalability_OMP_thin.xlsx spreadsheet used to collect all output from the executions on the THIN nodes for the OpenMP scalability to produce tables and graphic charts
scalability_weak.xlsx spreadsheet used to collect all output from the executions for the weak scalability to produce tables and graphic charts
````

