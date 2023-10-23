all:
	make clean
	make create

create: main.c
	gcc -Wall -c main.c
	gcc main.o -o main -l pthread -l rt

test:
	./main 2 1 1

clean:
	rm -f *.o main
