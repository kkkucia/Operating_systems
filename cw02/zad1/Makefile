CC= gcc -Wall
FILENAME_INPUT=test_from.txt
FILENAME_OUTPUT=test_to.txt
REPORT=pomiar_zad_1.txt

tests:
	make clean

	make diff_lib
	make diff_sys

diff_lib:
	make replace_lib
	./replace_lib a Y $(FILENAME_INPUT) $(FILENAME_OUTPUT) >> $(REPORT)
	tr a Y < $(FILENAME_INPUT) > tmp
	diff -s tmp $(FILENAME_OUTPUT) >> $(REPORT)

diff_sys:
	make replace_sys
	./replace_sys a X $(FILENAME_INPUT) $(FILENAME_OUTPUT) >> $(REPORT)
	tr a X < $(FILENAME_INPUT) > tmp
	diff -s tmp $(FILENAME_OUTPUT) >> $(REPORT)

replace_lib: replace_lib.c
	$(CC) -c replace_lib.c -o replace_lib.o
	$(CC) replace_lib.o -o replace_lib

replace_sys: replace_sys.c
	$(CC) -c replace_sys.c -o replace_sys.o
	$(CC) replace_sys.o -o replace_sys

clean:
	rm -f *.o
	rm -f *.out
	rm -f tmp
	rm -f replace_sys replace_lib
	rm -f test_to.txt