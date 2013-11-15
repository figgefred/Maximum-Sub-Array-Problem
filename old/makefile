all: max_sub
clean:
	rm bin/*

max_sub: src/maxsubarray.c 
	g++ src/maxsubarray.c -fopenmp -O -o bin/maxsubarray

max_sub_para: src/maxsubarray_para.c
	g++ src/maxsubarray_para.c -fopenmp -O -o bin/maxsubarray_para

