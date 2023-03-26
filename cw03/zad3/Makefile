CC= gcc -Wall

PATH1="."
PATH2=".."
PATH3="../.."

ARG1="\#include"
ARG2="asdf"
ARG3="zadanie"


all:
	make clean
	make processes_tree
	make tests_PATH1
	make tests_PATH2
	make tests_PATH3


processes_tree: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main

tests_PATH1:
	./main $(PATH1) $(ARG3)
	./main $(PATH1) $(ARG2)
	./main $(PATH1) $(ARG1)

tests_PATH2:
	./main $(PATH2) $(ARG3)
	./main $(PATH2) $(ARG2)
	./main $(PATH2) $(ARG1)

tests_PATH3:
	./main $(PATH3) $(ARG3)
	./main $(PATH3) $(ARG2)
	./main $(PATH3) $(ARG1)


clean:
	rm -f *.o
	rm -f main