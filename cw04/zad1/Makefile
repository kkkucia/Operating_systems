CC= gcc -Wall
FORK= ./fork
EXEC= ./exec

all:
	make clean
	make signals_fork
	make test_fork
	make signals_exec
	make test_exec

signals_fork: fork.c
	$(CC) -c fork.c -o fork.o
	$(CC) fork.o -o fork

signals_exec: exec.c
	$(CC) -c exec.c -o exec.o
	$(CC) exec.o -o exec

test_fork:
	$(FORK) ignore
	$(FORK) handler
	$(FORK) mask
	$(FORK) pending

test_exec:
	$(EXEC) ignore
	$(EXEC) mask
	$(EXEC) pending

clean:
	rm -f *.o
	rm -f fork exec