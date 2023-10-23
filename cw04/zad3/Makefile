CC= gcc -Wall

all:
	make clean
	make catcher
	make sender

catcher: catcher.c
	$(CC) -c catcher.c -o catcher.o
	$(CC) catcher.o -o catcher

sender: sender.c
	$(CC) -c sender.c -o sender.o
	$(CC) sender.o -o sender

clean:
	rm -f *.o
	rm -f sender catcher