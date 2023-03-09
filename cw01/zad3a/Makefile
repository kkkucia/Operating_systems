CC = gcc -Wall

all:
	make clean
	make test_static
	make test_shared
	make test_dynamic

static: library.c
	$(CC) -c library.c
	ar rcs library.a library.o
	$(CC) -c main.c
	$(CC) main.o library.a -o main

shared: library.c
	$(CC) -fPIC -c library.c
	$(CC) -shared -o liblibrary.so library.o
	$(CC) main.c -o main_shared -L. -llibrary -Wl,-rpath=.

dynamic: library.c
	$(CC) -fPIC -c library.c
	$(CC) -shared library.o -o liblibrary.so
	$(CC) maindll.c -o main_dynamic -ldl

test_static:
	make static
	cat tests | ./main > results_static.txt

test_shared:
	make shared
	cat tests | ./main_shared > results_shared.txt

test_dynamic:
	make dynamic
	cat tests | ./main_dynamic > results_dynamic.txt

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f main main_shared main_dynamic
