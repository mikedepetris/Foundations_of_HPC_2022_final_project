#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name=test
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise1
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=test_epyc_job_%j.out

module load architecture/AMD
module load openMPI/4.1.4/gnu/12.2.1

export OMP_NUM_THREADS=64
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export BLIS_NUM_THREADS=64
export LD_LIBRARY_PATH=/u/dssc/mdepet00/mikeblis/lib:$LD_LIBRARY_PATH

# test 

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=test_epyc_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=64"
#increase the size of ...
#srun -n1 --cpus-per-task=64  ./sgemm_mkl.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {1..128}
do
#	srun -n1 --cpus-per-task=$i ./sgemm_mkl.x $matrixsize $matrixsize $matrixsize >>$csvname
	mpirun -np $i gameoflife.x -r -f pattern_random1000.pgm -n1 -e2 -s0 -q
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
echo "`hostname` $now END"
