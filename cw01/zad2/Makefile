CC = gcc -Wall

all:
	make static
	make shared
	make dynamic

static: library.c
	make clean
	$(CC) -c library.c
	ar rcs library.a library.o
	$(CC) -c main.c
	$(CC) main.o library.a  -o main
	./main

shared: library.c
	make clean
	$(CC) -fPIC -c library.c
	$(CC) -shared -o liblibrary.so library.o
	$(CC) main.c -o main_shared -L. -llibrary -Wl,-rpath=.
	./main_shared

dynamic: library.c
	make clean
	$(CC) -fPIC -c library.c
	$(CC) -shared library.o -o liblibrary.so
	$(CC) maindll.c -o main_dynamic -ldl
	./main_dynamic

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f main main_shared main_dynamic
