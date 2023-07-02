#!/bin/bash
#SBATCH --no-requeue
# set the job name with "sbatch -J thejobname <script>.sh [arguments]
#SBATCH --job-name="dgemm_mkl_epyc"
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=dgemm_mkl_epyc_job_%j.out

#module load architecture/AMD
#module load openMPI/4.1.4/gnu/12.2.1
module load mkl
#module load openBLAS/0.3.21-omp
module load openBLAS/0.3.23-omp

#mpirun -np 1 make all

#export OMP_NUM_THREADS=64
export OMP_PLACES=cores
export OMP_PROC_BIND=close

#hpl: export codes=/u/path_to/hpl-2.3/bin/epyc
#hpl: mpirun -np 128 --map-by core $codes/xhpl

echo size scalability begin

for LIB in oblas oblas_optimized mkl mkl_optimized; do
  now=$(date +"%Y-%m-%d_%H-%M-%S")
  csvname=dgemm_"$LIB"_epyc_$(hostname)_$now.csv
  #echo "$csvname $(hostname) $now"
  echo "$csvname"
  #2000,2000,2000,0.034462963,0.034463,464.266523
  echo "m,n,k,elapsed1,elapsed2,GFLOPS" >"$csvname"
  #srun -n1 --cpus-per-task=64 ./dgemm_oblas.x 2000 2000 200>discarded_first_result_"$csvname"
  #echo "$csvname $(hostname) $now">"$csvname"
  for REP in {1..10}; do
    #increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
    for SIZE in {2000..20000..500}; do
      echo rep "$REP" size "$SIZE" srun -n1 --cpus-per-task=64 ./dgemm_$LIB.x "$SIZE" "$SIZE" "$SIZE"'>>'"$csvname"
      srun -n1 --cpus-per-task=64 ./dgemm_$LIB.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
      #srun -n1 --cpus-per-task=64 ./dgemm_oblas.x 2000 2000 2000
    done
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
echo "$(hostname)" "$now" END
echo size scalability end
