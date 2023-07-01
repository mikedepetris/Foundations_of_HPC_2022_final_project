#!/bin/bash
#SBATCH --no-requeue
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2/intel
#SBATCH --partition=THIN
#SBATCH --job-name=gemm_thin
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 12
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=gemm_mkl_thin_job_%j.out

#module load architecture/Intel
module load mkl
#module load openBLAS/0.3.21-omp
module load openBLAS/0.3.23-omp

export OMP_NUM_THREADS=12
export OMP_PLACES=cores
#export OMP_PLACES={0}:12:1
export OMP_PROC_BIND=close
export OMP_DISPLAY_ENV=true

# test single precision float

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=sgemm_mkl_thin_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=12  ./sgemm_mkl.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./sgemm_mkl.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=sgemm_mkl_thin_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=sgemm_oblas_thin_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./sgemm_oblas.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./sgemm_oblas.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done


now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=sgemm_oblas_thin_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done







# test double precision double

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=dgemm_mkl_thin_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./dgemm_mkl.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./dgemm_mkl.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=dgemm_mkl_thin_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=dgemm_oblas_thin_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./dgemm_oblas.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./dgemm_oblas.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done


now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=dgemm_oblas_thin_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=12"
srun -n1 --cpus-per-task=12  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {0..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=12 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
echo "`hostname` $now END"
