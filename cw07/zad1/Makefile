CC= gcc -Wall

all:
	make clean
	make create

create: main.c
	$(CC) main.c -o main

test:
	./main 2 1 1

clean:
	rm -f *.o main
