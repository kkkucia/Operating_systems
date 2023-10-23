#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define MY_FIFO "/tmp/myfifo"
#define BUFFER_SIZE 4096


void get_info(clock_t start_time, clock_t end_time, double result, double h, int n) {
    double time = (double) (end_time - start_time) / CLOCKS_PER_SEC;

    printf("Result: %.17f ; Width: %.17f ; Number of programs: %d ; Time: %.17f \n", result, h, n, time);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "ERROR - Invalid number of arguments\nEXPECTED ARGUMENTS: [width - precision] [number of programs]\n");
        return EXIT_FAILURE;
    }

    double a = 0.0;
    double b = 1.0;
    double h = atof(argv[1]);
    int programs_num = atoi(argv[2]);
    double interval_size = (b - a) / programs_num;
    clock_t start_time, end_time;

    start_time = clock();

    if (mkfifo(MY_FIFO, 0666) == -1) { //0666 - write and read to pipe
        perror("ERROR - mkfifo()");
        exit(EXIT_FAILURE);
    }

    double curr_a, curr_b;

    for (int i = 0; i < programs_num; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            curr_a = a + i * interval_size;
            curr_b = a + (i + 1) * interval_size;
            char bufferA[BUFFER_SIZE];
            char bufferB[BUFFER_SIZE];
            snprintf(bufferA, BUFFER_SIZE, "%lf", curr_a);
            snprintf(bufferB, BUFFER_SIZE, "%lf", curr_b);

            execl("./counter", "counter", bufferA, bufferB, argv[1], NULL);
        }
    }

    int fifo = open(MY_FIFO, O_RDONLY);
    if (fifo == -1) {
        perror("ERROR - open()");
        exit(EXIT_FAILURE);
    }

    double all_result = 0.0;
    char buffer[BUFFER_SIZE] = "";
    int i = 0;
    char *separator = "\n";

    while (i < programs_num) {
        size_t size = read(fifo, buffer, BUFFER_SIZE);
        buffer[size] = 0;
        char *curr_result;
        curr_result = strtok(buffer, separator);

        while (curr_result != NULL) {
            all_result += atof(curr_result);
            curr_result = strtok(NULL, separator);
            i += 1;
        }
    }
    close(fifo);
    remove(fifo);

    end_time = clock();
    get_info(start_time, end_time, all_result, h, programs_num);

    return EXIT_SUCCESS;
}
