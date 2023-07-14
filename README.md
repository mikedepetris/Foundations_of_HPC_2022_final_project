# Foundations of HPC 2022 final project

This is the final project for the Foundations of High Performance Computing course, part of the MSc in Data Science and Scientific Computing at University of Trieste (all related material can be found at https://github.com/Foundations-of-HPC/Foundations_of_HPC_2022/tree/main/Assignment).

Structure of this github directory:
README.md 
report.pdf
exercise1/README.md 
exercise1/all the files for exercise 1 
exercise2/README.md 
exercise2/all the files for exercise 2

where:
report.pdf contains a detailed report of all the exercises in this documents.
README.md a file in markdown language that describes briefly what was done.
exercise1/README.MD explains all the files present in the directory and give detailed information on which software stack should be used to compile the codes and run all the programs.
a short description of scripts you write and use to complete the exercise
a short description of the datasets available (in .csv file are commma separated values) to produce any kind of figures you provided
exercise2/README.MD should explain all the files present in the exercise 2 directory
exercise3/README.MD should explain all the files present in the exercise 3 directory

What was the assignment about
Exercise 1
In this exercise we were asked to implement a parallel version of the famous Conway's Game of Life ("GOL") and to measure its scalability in different conditions. The parallelism had to be a hybrid MPI/openMP, i.e. we had to work with a distributed memory approach and a shared memory approach on the same code.

In practice, we implemented an MPI version of GOL in which the workload was equally distributed among the processes; then, each MPI process would parallelize its own work spawning a number of openMP processes. To study the scalability we compiled and ran the program on ORFEO (the cluster hosted at Area Science Park (Trieste)).

For details about this assignment see this document in the original course repository.

Exercise 2
In this second exercise we were asked to compare the performance of three HPC math libraries: MKL, openBLAS and BLIS (the last one had to be downloaded and compiled by the student on his own working area on ORFEO).

In particular, we had to compare the performance of a level 3 BLAS function called gemm on matrix-matrix multiplications, both for increasing matrix size (at fixed number of CPUs) and for incresing number of CPUs (at fixed matrix size), both on EPYC and THIN nodes of ORFEO, both for single and double point precision floating point numbers and with different threads allocation policies (we chose to use close cores and spread cores).

For details about this assignment see this document in the original course repository.
