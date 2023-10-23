#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

clock_t start_time, end_time;

void get_info(clock_t start_time, clock_t end_time, double result, double h, int n) {
    double time = (double) (end_time - start_time) / CLOCKS_PER_SEC;

    printf("Result: %.17f ; Width: %.17f ; Number of processes: %d ; Time: %.17f \n", result, h, n, time);
}

double f(double x){
    return 4.0 / (x*x + 1);
}

double integral(double a, double b, double h) {
    double result = 0.0;
    for (double x = a; x < b; x += h) {
        result += f(x) * h;
    }
    return result;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "ERROR - Invalid number of arguments\nEXPECTED ARGUMENTS: [width - precision] [number of processes]\n");
        return EXIT_FAILURE;
    }
    double a = 0.0;
    double b = 1.0;
    double h = atof(argv[1]);
    int processes_num = atoi(argv[2]);
    double interval_size = (b - a) / processes_num;
    int fd[processes_num][2]; //[read][write]

    start_time = clock();

    for (int i = 0; i < processes_num; i++) {
        if (pipe(fd[i]) == -1) {
            perror("ERROR - CANNOT CREATE PIPE");
            exit(EXIT_FAILURE);
        }
    }
    double curr_a, curr_b, curr_result;

    for (int j = 0; j < processes_num; j++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("ERROR - CANNOT FORK ");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            curr_a = a + j * interval_size;
            curr_b = a + (j+1) * interval_size;
            curr_result = integral(curr_a, curr_b, h);

            close(fd[j][0]); // close read
            write(fd[j][1], &curr_result, sizeof(double)); // write current result
            close(fd[j][1]); // close write
            exit(EXIT_SUCCESS);
        }
    }

    double all_result = 0;
    for (int i = 0; i < processes_num; i++) {
        close(fd[i][1]); // close write
        read(fd[i][0], &curr_result, sizeof(double)); // read current result
        all_result += curr_result;
        close(fd[i][0]); // close read
    }

    end_time = clock();
    get_info(start_time, end_time, all_result, h, processes_num);

    return EXIT_SUCCESS;
}
