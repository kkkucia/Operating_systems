#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>
#include <unistd.h>

struct tms start_time;
struct tms end_time;
clock_t real_start_time;
clock_t real_end_time;

const int MAX_SIZE_OF_BLOCK = 1024;
const int SIZE_OF_BLOCK = 1;

long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void reverse_buffer(char *buffer) {
    int n = strlen(buffer);
    int temp;
    for (int i = 0; i < n / 2; i++) {
        temp = buffer[i];
        buffer[i] = buffer[n - i - 1];
        buffer[n - i - 1] = temp;
    }
}

void reverse_by_block(char **argv) {
    FILE *file_to_read = fopen(argv[1], "r");
    if (!file_to_read) {
        fprintf(stderr, "ERROR INVALID INPUT FILE\n");
        return;
    }
    FILE *file_to_write = fopen(argv[2], "a+");
    if (!file_to_write) {
        fprintf(stderr, "ERROR INVALID OUTPUT FILE\n");
        fclose(file_to_read);
        return;
    }
    size_t file_size = get_file_size(file_to_read);
    int number_of_blocks = file_size / MAX_SIZE_OF_BLOCK;
    int not_full_block = file_size % MAX_SIZE_OF_BLOCK;
    char *buffer;

    int index = 1;

    while (index <= number_of_blocks) {
        fseek(file_to_read, -index * MAX_SIZE_OF_BLOCK, SEEK_END);
        buffer = calloc(MAX_SIZE_OF_BLOCK, sizeof(char));

        int read_size = fread(buffer, sizeof(char), MAX_SIZE_OF_BLOCK, file_to_read);
        if (read_size != MAX_SIZE_OF_BLOCK) {
            fprintf(stderr, "ERROR LOADING DATA\n");
            fclose(file_to_read);
            fclose(file_to_write);
        }
        reverse_buffer(buffer);
        int write_size = fwrite(buffer, sizeof(char), MAX_SIZE_OF_BLOCK, file_to_write);
        if (write_size != MAX_SIZE_OF_BLOCK) {
            fprintf(stderr, "ERROR WRITING DATA\n");
        }
        free(buffer);
        index++;
    }
    buffer = calloc(not_full_block, sizeof(char));
    fseek(file_to_read, 0, SEEK_SET);
    int read_size = fread(buffer, sizeof(char), not_full_block, file_to_read);
    if (read_size != not_full_block) {
        fprintf(stderr, "ERROR LOADING DATA\n");
        fclose(file_to_read);
        fclose(file_to_write);
    }
    reverse_buffer(buffer);
    int write_size = fwrite(buffer, sizeof(char), not_full_block, file_to_write);
    if (write_size != not_full_block) {
        fprintf(stderr, "ERROR WRITING DATA\n");
    }
    free(buffer);
    fclose(file_to_read);
    fclose(file_to_write);
}

int check_sign(char *sign) {
    if (strlen(sign) == 1) {
        return 0;
    }
    return 1;
}

int check_arguments(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 2 ARGUMENTS: \n [File 1][File 2]");
        return 1;
    }
    return 0;
}

void get_times(clock_t real_start_time, clock_t real_end_time, struct tms start_time, struct tms end_time,
               int size_of_block) {
    double real_time = (double) (real_end_time - real_start_time) / CLOCKS_PER_SEC * 1000;
    double user_time = (double) (end_time.tms_utime - start_time.tms_utime) / (double) sysconf(_SC_CLK_TCK);
    double system_time = (double) (end_time.tms_stime - start_time.tms_stime) / (double) sysconf(_SC_CLK_TCK);

    printf("TIME for block size: %d : Real: %.5f ; User: %.5f ; System: %.5f\n", size_of_block, real_time,
           user_time, system_time);
}

void start_timer() {
    real_start_time = clock();
    times(&start_time);
}

void end_timer() {
    real_end_time = clock();
    times(&end_time);
}


int main(int argc, char **argv) {
    if (check_arguments(argc, argv) == 0) {
        start_timer();
        reverse_by_block(argv);
        end_timer();
        get_times(real_start_time, real_end_time, start_time, end_time, MAX_SIZE_OF_BLOCK);
    }
    return 0;
}
