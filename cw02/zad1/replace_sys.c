#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>

struct tms start_time;
struct tms end_time;
clock_t real_start_time;
clock_t real_end_time;

long get_file_size(int fd) {
    long size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return size;
}

void replace(char **argv) {
    int fd_to_read = open(argv[3], O_RDONLY);
    if (fd_to_read == -1) {
        fprintf(stderr, "ERROR INVALID INPUT FILE\n");
        return;
    }
    int fd_to_write= open(argv[4], O_WRONLY | O_CREAT);
    if (fd_to_write == -1) {
        fprintf(stderr, "ERROR INVALID OUTPUT FILE\n");
        close(fd_to_read);
        return;
    }
    size_t file_size = get_file_size(fd_to_read);
    char *buffer = calloc(file_size, sizeof(char));

    read(fd_to_read, buffer, sizeof(char) * file_size);
    for (int i = 0; i < file_size; i++) {
        if (buffer[i] == argv[1][0]) {
            buffer[i] = argv[2][0];
        }
    }
    write(fd_to_write, buffer, sizeof(char) * file_size);

    close(fd_to_read);
    close(fd_to_write);
    free(buffer);
}

int check_sign(char *sign) {
    if (strlen(sign) == 1) {
        return 0;
    }
    return 1;
}

int check_arguments(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 4 ARGUMENTS: \n [Char 1][Char 2][File 1][File 2]");
        return 1;
    }
    if ((check_sign(argv[1]) == 1) || (check_sign(argv[2]) == 1)) {
        fprintf(stderr, "INVALID ARGUMENTS - FIST AND SECOND ARGUMENTS MUST BE A CHAR\n");
        return 1;
    }
    return 0;
}
void get_times(clock_t real_start_time, clock_t real_end_time, struct tms start_time, struct tms end_time) {
    double real_time = (double) (real_end_time - real_start_time) / CLOCKS_PER_SEC * 1000;
    double user_time = (double) (end_time.tms_utime - start_time.tms_utime) / (double) sysconf(_SC_CLK_TCK);
    double system_time = (double) (end_time.tms_stime - start_time.tms_stime) / (double) sysconf(_SC_CLK_TCK);

    printf("\nTIME SYS: Real: %.5f ; User: %.5f ; System: %.5f\n", real_time, user_time, system_time);
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
    if (check_arguments(argc, argv) == 0){
        start_timer();
        replace(argv);
        end_timer();
    }
    return 0;
}