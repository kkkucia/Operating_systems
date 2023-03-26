CC= gcc -Wall
ARG=5

test:
	make clean
	make check_processes

check_processes: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main
	./main $(ARG)

clean:
	rm -f *.o
	rm -f main