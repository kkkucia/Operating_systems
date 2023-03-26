CC= gcc -Wall
ARG1="test1"
ARG2="test2"
ARG3="/bin"

all:
	make clean
	make main_ls
	make tests


main_ls: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main


tests:
	./main $(ARG3)
	./main $(ARG2)
	./main $(ARG1)


clean:
	rm -f *.o
	rm -f main