CC= gcc -Wall -g -lpthread

all:
	make clean
	make create
	make santa

create: northPole.c
	$(CC) northPole.c -o northPole

santa:
	./northPole

clean:
	rm -f *.o northPole
