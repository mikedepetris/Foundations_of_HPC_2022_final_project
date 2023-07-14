# Foundations of HPC 2022 final project

This is the final assigmment for the 2022/2023 Foundations of High Performance Computing course, part of the MSc in Data Science and Scientific Computing at University of Trieste (all related material can be found at https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).
It consists of two mandatory exercises.

### Structure of this github directory: 

````
[README.md](README.md)
[report.pdf](README.md)
exercise1/README.md 
exercise1/all the files for exercise 1 
exercise2/README.md 
exercise2/all the files for exercise 2
````

where: 

- report.pdf            contains a detailed report of all the exercises in this documents.
- README.md             is a file in markdown language that describes briefly what was done. 
- exercise1/README.MD   explains all the files present in the directory and gives detailed information on
  - which software stack should be used to compile the codes and run all the programs.
  - a short description of scripts used to complete the exercise.
  - a short description of the datasets available (in .csv file are commma separated values) to produce any kind of provided figures
 - exercise2/README.MD should explain all the files present in the exercise 2 directory

## exercise 1:  parallel  programming  
Implementation of a parallel version of the famous Conway's Game of Life ("GOL") and scalability study in different conditions. The parallelism is implemented as a hybrid MPI/openMP, that means that the same code is designed with both a distributed memory approach and a shared memory approach.

The MPI version of GOL is implemented in a way in which the workload is equally distributed among the processes; then, each MPI process further parallelizes its own work spawning a number of openMP processes. To study the scalability the program had been compiled and ran on ORFEO (the cluster hosted at Area Science Park (Trieste)).

For details about this assignment see this document in the original course repository.

## exercise 2 : Comparing MKL, OpenBLAS and BLIS on matrix-matrix multiplication 

Performance comparison of math libraries available on HPC: MKL, OpenBLAS and BLIS.
The comparison has been performed focusing on the level 3 BLAS function called *gemm*. Such function comes in different flavours, for double precision (dgemm), single precision (sgemm). The calls the function to execute matrix-matrix multiplications, both for increasing matrix size (at fixed number of CPUs) and for incresing number of CPUs (at fixed matrix size), both on EPYC and THIN nodes of ORFEO, both for single and double precision floating point numbers and with different threads allocation policies (close cores and spread cores).

For details about this assignment see this document in the original course repository.
