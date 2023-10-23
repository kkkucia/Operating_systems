CC= gcc -Wall

all:
	make clean
	make create

create:
	$(CC) client.c -o client
	$(CC) server.c -o server

clean:
	rm -f *.o client server