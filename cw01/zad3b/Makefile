CC = gcc -Wall

all:
	make O0
	make O
	make O1
	make O2
	make O3
	make Os

O0:
	make clean
	make static OPT="O0"
	make shared OPT="O0"
	make dynamic OPT="O0"

O:
	make clean
	make static OPT="O"
	make shared OPT="O"
	make dynamic OPT="O"

O1:
	make clean
	make static OPT="O1"
	make shared OPT="O1"
	make dynamic OPT="O1"

O2:
	make clean
	make static OPT="O2"
	make shared OPT="O2"
	make dynamic OPT="O2"

O3:
	make clean
	make static OPT="O3"
	make shared OPT="O3"
	make dynamic OPT="O3"

Os:
	make clean
	make static OPT="Os"
	make shared OPT="Os"
	make dynamic OPT="Os"


static: library.c
	$(CC) -$(OPT) -c library.c
	ar rcs library.a library.o
	$(CC) -$(OPT) -c main.c
	$(CC) -$(OPT) main.o library.a -o main
	cat tests | ./main > results_static_$(OPT).txt

shared: library.c
	$(CC) -$(OPT) -fPIC -c library.c
	$(CC) -$(OPT) -shared -o liblibrary.so library.o
	$(CC) -$(OPT) main.c -o main_shared -L. -llibrary -Wl,-rpath=.
	cat tests | ./main_shared > results_shared_$(OPT).txt

dynamic: library.c
	$(CC) -$(OPT) -fPIC -c library.c
	$(CC) -$(OPT) -shared library.o -o liblibrary.so
	$(CC) -$(OPT) maindll.c -o main_dynamic -ldl
	cat tests | ./main_dynamic > results_dynamic_$(OPT).txt


clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
	rm -f *.out
	rm -f main main_shared main_dynamic
