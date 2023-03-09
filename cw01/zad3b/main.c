#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>

#include "library.h"


struct tms start_time;
struct tms end_time;
clock_t real_start_time;
clock_t real_end_time;


void printHelp(void) {
    fprintf(stderr, "Available commands: \n");
    fprintf(stderr, "help \n");
    fprintf(stderr, "init [size] - create array of size n \n");
    fprintf(stderr, "count [file]  - count words and lines from [file] \n");
    fprintf(stderr, "show [index] - show block from [index] in array \n");
    fprintf(stderr, "delete [index] - delete block from [index] in array \n");
    fprintf(stderr, "destroy - destroy memory of block \n");
    fprintf(stderr, "exit - close \n");
}

int is_int(char *argument) {
    int length = strlen(argument);
    for (int i = 0; i < length; i++) {
        if (isdigit(argument[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

Blocks *block_init(char *result) {
    result = strtok(NULL, " ");
    if ((result != NULL) && is_int(result) == 1) {
        int size = atoi(result);
        return create_blocks(size);
    } else {
        fprintf(stderr, "INVALID ARGUMENT. TYPE help FOR MORE INFORMATION \n");
        return NULL;
    }
}

void block_count(Blocks *blocks, char *result) {
    result = strtok(NULL, " ");

    if (result != NULL) {
        count_words_and_lines(blocks, result);
    } else {
        fprintf(stderr, "INVALID ARGUMENT. TYPE help FOR MORE INFORMATION \n");
    }
}

void block_show(Blocks *blocks, char *result) {
    result = strtok(NULL, " ");

    if ((result != NULL) && is_int(result) == 1) {
        int index = atoi(result);
        char *block_to_show = get_block(blocks, index);
        printf("%s\n", block_to_show);
    } else {
        fprintf(stderr, "INVALID ARGUMENT. TYPE help FOR MORE INFORMATION \n");
    }
}

void block_delete(Blocks *blocks, char *result) {
    result = strtok(NULL, " ");

    if ((result != NULL) && is_int(result)) {
        int index = atoi(result);
        remove_block(blocks, index);
    } else {
        fprintf(stderr, "INVALID COMMAND. TYPE help FOR MORE INFORMATION \n");
    }
}

void get_times(clock_t real_start_time, clock_t real_end_time, struct tms start_time, struct tms end_time) {
    double real_time = (double) (real_end_time - real_start_time) / CLOCKS_PER_SEC * 1000;
    double user_time = (double) (end_time.tms_utime - start_time.tms_utime) /
                       (double) sysconf(_SC_CLK_TCK);
    double system_time = (double) (end_time.tms_stime - start_time.tms_stime) / (double) sysconf(_SC_CLK_TCK);

    printf("TIME: Real: %.5f ; User: %.5f ; System: %.5f\n", real_time, user_time, system_time);
}

void start_timer() {
    real_start_time = clock();
    times(&start_time);
}

void end_timer() {
    real_end_time = clock();
    times(&end_time);
    get_times(real_start_time, real_end_time, start_time, end_time);
}

int main(int argc, char **argv) {
    char *result = calloc(MAX_COMMAND_SIZE, sizeof(char));
    Blocks *blocks = NULL;

    while (fgets(result, MAX_COMMAND_SIZE, stdin)) {
        int result_length = strlen(result);
        if (result[result_length - 1] == '\n') {
            result[result_length - 1] = '\0';
        }
        char *argument = strtok(result, " ");

        if (strcmp(argument, "help") == 0) {
            printHelp();
        } else if (strcmp(argument, "init") == 0) {
            start_timer();
            blocks = block_init(result);
            end_timer();
        } else if (strcmp(argument, "count") == 0) {
            start_timer();
            block_count(blocks, result);
            end_timer();
        } else if (strcmp(argument, "show") == 0) {
            start_timer();
            block_show(blocks, result);
            end_timer();
        } else if (strcmp(argument, "delete") == 0) {
            start_timer();
            block_delete(blocks, result);
            end_timer();
        } else if (strcmp(argument, "destroy") == 0) {
            start_timer();
            free_blocks(blocks);
            end_timer();
        } else if (strcmp(argument, "exit") == 0) {
            break;
        } else {
            fprintf(stderr, "INVALID COMMAND. TYPE help FOR MORE INFORMATION \n");
        }
        fflush(NULL);
    }
    return 0;
}
