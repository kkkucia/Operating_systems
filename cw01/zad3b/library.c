#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Blocks *create_blocks(int size) {
    Blocks *blocks = (Blocks *) malloc(sizeof(Blocks));
    blocks->size = size;
    blocks->current_size = 0;
    blocks->array_of_blocks = calloc(size, sizeof(char *));
    blocks->array_of_used_blocks = calloc(size, sizeof(int));
    memset(blocks->array_of_used_blocks, 0, size * sizeof(int));
    return blocks;
}

long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void count_words_and_lines(Blocks *blocks, char *filename) {
    char temp[] = "/tmp/library_XXXXXX";
    int tmp_file = mkstemp(temp);
    if (tmp_file == 0) {
        fprintf(stderr, "ERROR WHEN CREATE A TEMPORARY FILE\n");
        return;
    }
    char *command = calloc(MAX_COMMAND_SIZE, sizeof(char));
    snprintf(command, MAX_COMMAND_SIZE, "wc '%s' 1> '%s' 2>/dev/null", filename, temp);

    if (system(command) == 0) {
        FILE *temp_file = fopen(temp, "r");
        int size = (int) get_file_size(temp_file);
        char *buffer = calloc(size, sizeof(char));
        int correctly_load_size = fread(buffer, sizeof(char), size, temp_file);
        if (correctly_load_size != size){
            fprintf(stderr, "ERROR LOADING DATA\n");
        }
        fclose(temp_file);

        int buff_curr_size = strlen(buffer);
        buffer[buff_curr_size - 1] = '\0';
        buff_curr_size--;

        if (blocks->current_size < blocks->size){
            for (int i = 0; i < blocks->size; i++){
                if (blocks->array_of_used_blocks[i] == 0){
                    blocks->array_of_blocks[i] = calloc(buff_curr_size, sizeof(char));
                    blocks->array_of_blocks[i] = buffer;
                    blocks->array_of_used_blocks[i] = 1;
                    blocks->current_size++;
                    break;
                }
            }
        }else{
            fprintf(stderr, "MEMORY ERROR \n");
        }
    } else {
        fprintf(stderr, "COMMAND ERROR \n");
    }
}

char *get_block(Blocks *blocks, int block_index) {
    if ((block_index >= 0) && (block_index < blocks->size) && (blocks->array_of_used_blocks[block_index] == 1)) {
        return blocks->array_of_blocks[block_index];
    } else if (((block_index >= blocks->size) || (block_index < 0)) && (blocks != NULL)) {
        return "INDEX OUT OF RANGE ERROR";
    } else {
        return "NULL";
    }
}

void remove_block(Blocks *blocks, int block_index) {
    if ((block_index >= 0) && (block_index < blocks->size) && (blocks->array_of_used_blocks[block_index] == 1)) {
        free(blocks->array_of_blocks[block_index]);
        blocks->array_of_used_blocks[block_index] = 0;
        blocks->current_size--;
    }
    else if(blocks != NULL){
        fprintf(stderr, "NULL\n");
    } else {
        fprintf(stderr, "INDEX OUT OF RANGE ERROR\n");
    }
}

void free_blocks(Blocks *blocks) {
    if (blocks == NULL){
        fprintf(stderr, "ERROR NOTHING TO DESTROY\n");
        return;
    }
    for (int i = 0; i < blocks->size; i++) {
        if (blocks->array_of_used_blocks[i]) {
            free(blocks->array_of_blocks[i]);
            blocks->array_of_used_blocks[i] = 0;
        }
    }
    blocks->current_size = 0;
    free(blocks->array_of_blocks);
    free(blocks);
}





