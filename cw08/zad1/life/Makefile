CC = gcc
CFLAGS = -Wall

all:
	make clean
	make life
	./life

life: grid.c life.c
	$(CC) $(CFLAGS) grid.c life.c -o life -lncursesw

test:
	./life

clean:
	rm -f life
