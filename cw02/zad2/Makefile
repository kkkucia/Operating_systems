CC= gcc -Wall
FILENAME_INPUT=test_from.txt
REPORT=pomiar_zad_2.txt

tests:
	make clean
	make reverse_1
	make reverse_1024


reverse_1024:
	$(CC) -c reverse_1024.c -o reverse_1024.o
	$(CC) reverse_1024.o -o reverse_1024
	./reverse_1024 $(FILENAME_INPUT) test_to_1024.txt  >> $(REPORT)

reverse_1:
	$(CC) -c reverse_1.c -o reverse_1.o
	$(CC) reverse_1.o -o reverse_1
	./reverse_1 $(FILENAME_INPUT) test_to_1.txt  >> $(REPORT)

clean:
	rm -f *.o
	rm -f *.out
	rm -f reverse_1 reverse_1024 tmp
