#!/bin/bash
#SBATCH --no-requeue
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --job-name="scale_gemm"
#SBATCH --nodes=1
#SBATCH --exclusive
##SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=scale_gemm_epyc_job_%j.out

module load architecture/AMD
#module load openMPI/4.1.4/gnu/12.2.1
module load mkl
module load openBLAS/0.3.21-omp

#make

#export OMP_NUM_THREADS=64 we scale the number of threads=cores
export OMP_PLACES=cores
export OMP_PROC_BIND=close

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=scale_sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` OMP_NUM_THREADS=1-->64"
srun -n1 --cpus-per-task=64  ./sgemm_mkl_optimized.x 12000 12000 12000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for j in {1..64}
do
    export OMP_NUM_THREADS=$j
#   mpirun -n 1 --map-by node ./main.x -r -f init100.pbm -n $STEPS -s 0 >> omp_scalability.csv
#	srun -n1 --cpus-per-task=$j ./sgemm_mkl_optimized.x 12000 12000 12000 >>$csvname
	srun -n1 ./sgemm_mkl_optimized.x 12000 12000 12000 >>$csvname
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=scale_sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` OMP_NUM_THREADS=1-->64"
srun -n1 --cpus-per-task=64  ./sgemm_oblas_optimized.x 12000 12000 12000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for j in {1..64}
do
    export OMP_NUM_THREADS=$j
#   mpirun -n 1 --map-by node ./main.x -r -f init100.pbm -n $STEPS -s 0 >> omp_scalability.csv
#	srun -n1 --cpus-per-task=$j ./sgemm_oblas_optimized.x 12000 12000 12000 >>$csvname
	srun -n1 ./sgemm_oblas_optimized.x 12000 12000 12000 >>$csvname
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=scale_dgemm_mkl_epyc_optimized_$now.csv
echo "$csvname `hostname` OMP_NUM_THREADS=1-->64"
srun -n1 --cpus-per-task=64  ./dgemm_mkl_optimized.x 12000 12000 12000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for j in {1..64}
do
    export OMP_NUM_THREADS=$j
#   mpirun -n 1 --map-by node ./main.x -r -f init100.pbm -n $STEPS -s 0 >> omp_scalability.csv
#	srun -n1 --cpus-per-task=$j ./dgemm_mkl_optimized.x 12000 12000 12000 >>$csvname
	srun -n1 ./dgemm_mkl_optimized.x 12000 12000 12000 >>$csvname
done

now=`date +"%Y-%m-%d_%H-%M-%S"`
csvname=scale_dgemm_oblas_epyc_optimized_$now.csv
echo "$csvname `hostname` OMP_NUM_THREADS=1-->64"
srun -n1 --cpus-per-task=64  ./dgemm_oblas_optimized.x 12000 12000 12000 >discarded_first_result_$csvname
echo "$csvname `hostname` $now" >$csvname
for j in {1..64}
do
    export OMP_NUM_THREADS=$j
#   mpirun -n 1 --map-by node ./main.x -r -f init100.pbm -n $STEPS -s 0 >> omp_scalability.csv
#	srun -n1 --cpus-per-task=$j ./dgemm_oblas_optimized.x 12000 12000 12000 >>$csvname
	srun -n1 ./dgemm_oblas_optimized.x 12000 12000 12000 >>$csvname
done

#make image
#make clean
