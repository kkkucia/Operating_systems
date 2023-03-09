#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define MAX_COMMAND_SIZE 2048

typedef struct {
    int size;
    int current_size;
    char **array_of_blocks;
    int *array_of_used_blocks;
} Blocks;

Blocks *create_blocks(int size);

void count_words_and_lines(Blocks *blocks, char *filename);

char *get_block(Blocks *blocks, int block_index);

void remove_block(Blocks *blocks, int block_index);

void free_blocks(Blocks *blocks);

#endif //LAB1_LIBRARY_H
