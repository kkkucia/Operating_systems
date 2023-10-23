#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MY_FIFO "/tmp/myfifo"
#define BUFFER_SIZE 4096

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
    if (argc != 4){
        printf("ERROR - Invalid arguments");
        exit(EXIT_FAILURE);
    }

    double a, b, h, result;
    a = atof(argv[1]);
    b = atof(argv[2]);
    h = atof(argv[3]);

    result = integral(a, b, h);

    char buffer[BUFFER_SIZE] = "";
    size_t size = snprintf(buffer, BUFFER_SIZE, "%lf\n", result);

    int fifo = open(MY_FIFO, O_WRONLY);
    if (fifo == -1) {
        perror("ERROR - open()");
        exit(EXIT_FAILURE);
    }

    if(write(fifo,buffer, size) == -1){
        perror("ERROR - CANNOT WRITE");
        exit(EXIT_FAILURE);
    }

    close(fifo);
    return EXIT_SUCCESS;
}