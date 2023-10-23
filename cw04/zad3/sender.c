#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SIGUSR1 10


int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "[SENDER] ERROR - INVALID ARGUMENTS \n");
        return EXIT_FAILURE;
    }

    sigval_t sig_val;
    sig_val.sival_int = atoi(argv[2]);
    pid_t catcher_pid = atoi(argv[1]);

    printf("[SENDER] Sent state: %d\n", sig_val.sival_int);
    if (sigqueue(catcher_pid, SIGUSR1, sig_val) == -1) {
        perror("[SENDER] sigqueue() ERROR ");
        exit(EXIT_FAILURE);
    }else{
        printf("[SENDER] Sender confirmation!\n");
    }

    return EXIT_SUCCESS;
}
