#!/bin/bash
#SBATCH --no-requeue
# set the job name with "sbatch -J thejobname <script>.sh [arguments]
#SBATCH --job-name="dgemm_epyc"
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 128
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=dgemm_epyc_job_%j.out

#module load architecture/AMD
#module load openMPI/4.1.4/gnu/12.2.1
module load mkl
#module load openBLAS/0.3.21-omp
module load openBLAS/0.3.23-omp

#mpirun -np 1 make all

export OMP_PLACES=cores
export OMP_PROC_BIND=close
#export OMP_NUM_THREADS=64
#export BLIS_NUM_THREADS=64
export LD_LIBRARY_PATH=/u/dssc/mdepet00/assignment/exercise2/blis/lib:$LD_LIBRARY_PATH

SIZE=30000
#LIB=blis_optimized
#LIB=mkl_optimized
LIB=oblas_optimized
#oblas  mkl  blis

echo size scalability begin
now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=dgemm_"$LIB"_epyc_$(hostname)_$now.csv
echo "m,n,k,elapsed1,elapsed2,GFLOPS" >"$csvname"
for REP in {1..10}; do
  csvname=dgemm_"$LIB"_epyc_$(hostname)_$now.csv
  #increase the number of cores and analyse the scaling of the GEMM calculation for at least MKL and openblas.
  for CORES in {128..1}; do
    export OMP_NUM_THREADS="$CORES"
    export BLIS_NUM_THREADS="$CORES"
    echo dgemm_epyc_job_"$SLURM_JOB_ID".out$'\t'"$csvname"$'\t'"$(hostname)"$'\t'rep "$REP" size "$SIZE" OMP_PLACES=$OMP_PLACES OMP_PROC_BIND=$OMP_PROC_BIND OMP_NUM_THREADS="$OMP_NUM_THREADS" BLIS_NUM_THREADS="$BLIS_NUM_THREADS" srun -n1 --cpus-per-task="$CORES" ./dgemm_$LIB.x "$SIZE" "$SIZE" "$SIZE"'>>'"$csvname"
    srun -n1 --cpus-per-task="$CORES" ./dgemm_$LIB.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
    #srun -n1 --cpus-per-task=64 ./dgemm_oblas.x 2000 2000 2000
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
echo "$(hostname)" "$now" END
echo size scalability end
