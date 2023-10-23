CC= gcc -Wall

all:
	make clean
	make test

test: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main

clean:
	rm -f *.o
	rm -f main