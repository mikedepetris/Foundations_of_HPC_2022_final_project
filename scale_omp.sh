#!/bin/bash
#SBATCH --no-requeue
# set the job name with "sbatch -J thejobname <script>.sh [arguments]
#SBATCH --job-name="omp_scale"
#SBATCH --get-user-env
#SBATCH --chdir=/u/dssc/mdepet00/assignment/exercise1
#SBATCH --partition=EPYC
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks-per-node 64
#SBATCH --mem=490G
#SBATCH --time=02:00:00
#SBATCH --output=scale_omp_epyc_job_%j.out

#SIZE=100
STEPS=100
TYPE="i"
if [ $# == 1 ]; then
  TYPE="$1"
fi
SNAPAT=0
if [ $# == 2 ]; then
  SNAPAT="$2"
fi
echo "Selected type of execution: $TYPE"

module load architecture/AMD
module load openMPI/4.1.4/gnu/12.2.1

mpirun -np 1 make all

#export OMP_NUM_THREADS=64
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# generate new playground with random values of given SIZE
#mpirun -np 1 gameoflife.x -i -k $SIZE -f pattern_random$SIZE

now=$(date +"%Y-%m-%d_%H-%M-%S")
csvname=scale_omp_epyc_$(hostname)_$now.csv
echo "$csvname $(hostname) $now"
echo "action,world_size,number_of_steps,number_of_steps_between_file_dumps,mpi_size,omp_get_max_threads,total_time,t_io,t_io_accumulator,t_io_accumulator_average" >"$csvname"

echo scalability begin

for REP in {1..5}; do
  for SIZE in 10000 1000 100; do

    if [ "$TYPE" == i ]; then
      for threads in {1..64}; do
        echo rep $REP scalability -i "$SIZE" "$threads"
        export OMP_NUM_THREADS=$threads
        mpirun -n 1 --map-by socket gameoflife.x -i -k $SIZE -f pattern_random$SIZE -q >>"$csvname"
      done
    else
      for threads in {1..64}; do
        echo scalability -e"$TYPE" "$SIZE" "$threads"
        export OMP_NUM_THREADS=$threads
        {
          mpirun -n 1 --map-by socket gameoflife.x -r -f pattern_random$SIZE.pgm -n $STEPS -e "$TYPE" -s "$SNAPAT" -q
          #      mpirun -n 1 --map-by node gameoflife.x -r -f pattern_random$SIZE.pgm -n $STEPS -e 0 -s 0 -q
          #      mpirun -n 1 --map-by node gameoflife.x -r -f pattern_random$SIZE.pgm -n $STEPS -e 1 -s 0 -q
          #      mpirun -n 1 --map-by node gameoflife.x -r -f pattern_random$SIZE.pgm -n $STEPS -e 2 -s 0 -q
          #      mpirun -n 1 --map-by node gameoflife.x -r -f pattern_random$SIZE.pgm -n $STEPS -e 3 -s 0 -q
        } >>"$csvname"
      done
    fi

  done
done

echo scalability end

#TODO: join image files into animation

#redo for SIZE 100 1000 10000
