CC = gcc -Wall

all:
	make static
	make shared

static: library.c
	make clean
	$(CC) -c library.c
	ar rcs library.a library.o

shared: library.c
	make clean
	$(CC) -fPIC -c library.c
	$(CC) -shared -o liblibrary.so library.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
