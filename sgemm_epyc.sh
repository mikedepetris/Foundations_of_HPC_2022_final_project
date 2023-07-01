#!/bin/bash
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise2
#SBATCH --partition=EPYC
#SBATCH --job-name=sgemm_mkl_epyc
#SBATCH --nodes=1
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=sgemm_mkl_epyc_job_%j.out

#module load architecture/AMD
#module load openMPI/4.1.4/gnu (needed?)
module load mkl
#module load openBLAS/0.3.21-omp
module load openBLAS/0.3.23-omp

#hpl: export codes=/u/path_to/hpl-2.3/bin/epyc
#hpl: mpirun -np 128 --map-by core $codes/xhpl

now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=sgemm_mkl_epyc_$now.csv
echo "$csvname $(hostname) $now"
#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=64 ./sgemm_mkl.x 2000 2000 200 >discarded_first_result_"$csvname"
echo "$csvname $(hostname) $now" >"$csvname"
for REP in {1..10}; do
  for SIZE in {2000..20000..500}; do
    echo rep "$REP" size "$SIZE"
    srun -n1 --cpus-per-task=64 ./sgemm_mkl.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
    #srun -n1 --cpus-per-task=64 ./sgemm_mkl.x 2000 2000 2000
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=sgemm_mkl_epyc_optimized_$now.csv
echo "$csvname $(hostname) $now"
#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x 2000 2000 2000 >discarded_first_result_"$csvname"
echo "$csvname $(hostname) $now" >"$csvname"
for REP in {1..10}; do
  for SIZE in {2000..20000..500}; do
    echo rep "$REP" size "$SIZE"
    srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
    #srun -n1 --cpus-per-task=64 ./sgemm_mkl_optimized.x 2000 2000 2000
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=sgemm_oblas_epyc_$now.csv
echo "$csvname $(hostname) $now"

#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=64 ./sgemm_oblas.x 2000 2000 200 >discarded_first_result_"$csvname"
echo "$csvname $(hostname) $now" >"$csvname"
for REP in {1..10}; do
  for SIZE in {2000..20000..500}; do
    echo rep "$REP" size "$SIZE"
    srun -n1 --cpus-per-task=64 ./sgemm_oblas.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
    #  srun -n1 --cpus-per-task=64 ./sgemm_oblas.x 2000 2000 2000
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=sgemm_oblas_epyc_optimized_$now.csv
echo "$csvname $(hostname) $now"

#increase the size of matrices size from 2000x2000 to 20000x20000 (single precision) and analyse the scaling of the GEMM calculation for at least MKL and openblas.
srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x 2000 2000 200 >discarded_first_result_"$csvname"
echo "$csvname $(hostname) $now" >"$csvname"
for REP in {1..10}; do
  for SIZE in {2000..20000..500}; do
    echo rep "$REP" size "$SIZE"
    srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x "$SIZE" "$SIZE" "$SIZE" >>"$csvname"
    #  srun -n1 --cpus-per-task=64 ./sgemm_oblas_optimized.x 2000 2000 2000
  done
done

now=$(date +"%Y-%m-%d_%H-%M-%S")
echo "$(hostname) $now END"
