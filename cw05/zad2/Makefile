CC= gcc -Wall

all:
	make clean
	make integral
	make test

integral: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main

test:
	./main 0.0000001 1  >> results.txt
	./main 0.0000001 10 >> results.txt
	./main 0.0000001 100  >> results.txt
	./main 0.0000001 500 >> results.txt

	./main 0.00000001 1  >> results.txt
	./main 0.00000001 10 >> results.txt
	./main 0.00000001 100  >> results.txt
	./main 0.00000001 500  >> results.txt

	./main 0.000000001 1  >> results.txt
	./main 0.000000001 10  >> results.txt
	./main 0.000000001 100  >> results.txt
	./main 0.000000001 500  >> results.txt


clean:
	rm -f *.o
	rm -f main