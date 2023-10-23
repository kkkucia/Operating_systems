CC= gcc -Wall
CLEAN=rm -f /tmp/myfifo

all:
	make clean
	make counter
	make program
	make test

counter: counter.c
	$(CC) -c counter.c -o counter.o
	$(CC) counter.o -o counter

program: main.c
	$(CC) -c main.c -o main.o
	$(CC) main.o -o main

test:
	./main 0.0000001 1  >> results.txt
	$(CLEAN)
	./main 0.0000001 10  >> results.txt
	$(CLEAN)
	./main 0.0000001 100 >> results.txt
	$(CLEAN)
	./main 0.0000001 500 >> results.txt
	$(CLEAN)

	./main 0.00000001 1  >> results.txt
	$(CLEAN)
	./main 0.00000001 10 >> results.txt
	$(CLEAN)
	./main 0.00000001 100 >> results.txt
	$(CLEAN)
	./main 0.00000001 500 >> results.txt
	$(CLEAN)

	./main 0.000000001 1  >> results.txt
	$(CLEAN)
	./main 0.000000001 10 >> results.txt
	$(CLEAN)
	./main 0.000000001 100  >> results.txt
	$(CLEAN)
	./main 0.000000001 500  >> results.txt
	$(CLEAN)


clean:
	rm -f *.o
	rm -f main counter
	$(CLEAN)