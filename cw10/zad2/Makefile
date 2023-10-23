SHELL := /bin/bash #ustawiam powłokę (shell) na /bin/bash
CC=gcc -Wall

all:
	make client
	make server

server: server.c
	$(CC) server.c -o server -lpthread

client: client.c
	$(CC) client.c -o client -lpthread

clean:
	rm -f client server

test_server:
	./server 8080 /tmp/socket_path

test_client1:
	./client Client1 web 127.0.0.1 8080

test_client2:
	./client Client2 unix /tmp/socket_path