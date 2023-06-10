#!/bin/bash
#SBATCH --no-requeue
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --job-name=policy_gemm_epyc
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=policy_gemm_epyc_job_%j.out

module load architecture/AMD
module load mkl
module load openBLAS/0.3.21-omp

nt=64
pl=cores
pb=close
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

nt=128
pl=cores
pb=close
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

nt=32
pl=cores
pb=close
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

nt=64
pl=cores
pb=spread
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

nt=64
pl=cores
pb=master
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

# Use 64 cores, and use only 2 cores for each CCX (each CCX has 4 cores).
#export OMP_PLACES={0}:64:2 #64 places, stride 2
nt=64
pl={0}:64:2
pb=master
export OMP_NUM_THREADS=$nt
export OMP_PLACES=$pl
export OMP_PROC_BIND=$pb

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_mkl_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=policy_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` $now export OMP_NUM_THREADS=$nt OMP_PLACES=$pl OMP_PROC_BIND=$pb"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 2000 2000 2000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for i in {18..18}
do
	let matrixsize=$((2000+1000*$i))
	for j in {1..4}
	do
		srun -n1 --cpus-per-task=64 ./dgemm_oblas_optimized.x $matrixsize $matrixsize $matrixsize >>$csvname
	done
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
echo "`hostname` $now END"
