all: gameoflife.x

gameoflife.x: gameoflife.h gameoflife.c files.io.h files_io.c new_playground.c run_ordered.c run_static.c run_wave.c run_whiteblack.c
	mpicc -fopenmp gameoflife.h gameoflife.c read_write_pgm.h read_write_pgm.c initialize.c iterate_ordered.c iterate_static.c iterate_wave.c iterate_whiteblack.c -o gameoflife.x
#	cp gameoflife.x /home/mike/

clean:
	rm -rf gameoflife.x

#deploy:
#	cp gameoflife.x /home/mike/
