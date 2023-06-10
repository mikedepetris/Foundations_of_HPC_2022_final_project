#!/bin/bash
#SBATCH --no-requeue
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --job-name=gemm_testblis_epyc
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=gemm_testblis_epyc_job_%j.out

module load architecture/AMD
module load mkl
module load openBLAS/0.3.21-omp

export OMP_NUM_THREADS=64
export OMP_PLACES=cores
export OMP_PROC_BIND=close
export BLIS_NUM_THREADS=64
export LD_LIBRARY_PATH=/u/dssc/mdepet00/mikeblis/lib:$LD_LIBRARY_PATH

# test single precision float

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=sgemm_blis_epyc_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=64"
#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=64  ./sgemm_blis.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {1..1}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_blis.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done


now=`date +"%Y-%m-%d_%H-%M-%S"`
echo "`hostname` $now END"
