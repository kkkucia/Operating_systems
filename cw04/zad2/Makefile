CC= gcc -Wall
SIGACT= ./sigaction


all:
	make clean
	make sigaction
	$(SIGACT)

sigaction: sigaction.c
	$(CC) -c sigaction.c -o sigaction.o
	$(CC) sigaction.o -o sigaction

clean:
	rm -f *.o
	rm -f sigaction