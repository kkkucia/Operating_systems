CC= gcc -Wall
TEST_DIR="../"

tests:
	make clean
	make overview
	./sizes_all $(TEST_DIR)

show_test_diff:
	make clean
	make overview
	./sizes_all $(TEST_DIR)
	find $(TEST_DIR) | xargs wc --bytes

overview: sizes_all.c
	$(CC) -c sizes_all.c -o sizes_all.o
	$(CC) sizes_all.o -o sizes_all


clean:
	rm -f *.o
	rm -f sizes_all