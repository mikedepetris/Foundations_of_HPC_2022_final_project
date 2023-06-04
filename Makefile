all: gameoflife.x

gameoflife.x: gameoflife.h gameoflife.c files.io.h files_io.c new_playground.c evolution_ordered.c evolution_static.c evolution_wave.c evolution_whiteblack.c
	mpicc -fopenmp gameoflife.h gameoflife.c read_write_pgm.h read_write_pgm.c initialize.c evolution_ordered.c evolution_static.c evolution_wave.c evolution_whiteblack.c -o gameoflife.x
#	cp gameoflife.x /home/mike/

clean:
	rm -rf gameoflife.x

#deploy:
#	cp gameoflife.x /home/mike/
