#!/bin/bash
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --job-name=test_job
##SBATCH --ntasks=12
#SBATCH --nodes=1
#SBATCH --ntasks-per-node 64
#SBATCH --mem=1gb
#SBATCH --time=00:05:00
#SBATCH --output=test_job_%j.out

now=`date +"%Y-%m-%d_%H-%M-%S"`
#pwd; hostname; date
echo "`hostname` `pwd` `date`"
echo "Hello, world !"
echo "`hostname` `pwd` $now"

#--tasks-per-node=<#nodes> specify the number of tasks for each node
#-D , --chdir=<directory> set the working directory of the batch script to directory before it is executed, example of desired path SLURM_SUBMIT_DIR, available as enviroment variable
#--exclusive the job allocation can not share nodes with other running jobs
#-o, --output=<file_path>redirect stdout and stderr to the specified file, the default file name is "slurm-%j.out", where the "%j" is replaced by the job ID.
