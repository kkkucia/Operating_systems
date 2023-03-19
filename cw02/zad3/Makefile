CC= gcc -Wall
TEST_DIR="../zad3"

tests:
	make clean
	make overview
	./sizes $(TEST_DIR)

show_test_diff:
	make clean
	make overview
	./sizes $(TEST_DIR)
	wc --bytes *

overview: sizes.c
	$(CC) -c sizes.c -o sizes.o
	$(CC) sizes.o -o sizes


clean:
	rm -f *.o
	rm -f sizes