#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int is_int(char *argument) {
    int length = strlen(argument);
    for (int i = 0; i < length; i++) {
        if (isdigit(argument[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

void check_processes(pid_t child_pid) {
    if (child_pid == -1) {
        perror("ERROR - CANNOT MAKE A CHILD PROCESS\n");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        printf("PROCESS: PARENT  PID: %d, CHILD PROCESS PID: %d\n", (int) getppid(), (int) getpid());
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 1 ARGUMENT: [Number of processes]");
        return EXIT_FAILURE;
    }

    if (is_int(argv[1]) == 0) {
        fprintf(stderr, "INVALID ARGUMENT - EXPECTED NUMBER");
        return EXIT_FAILURE;
    }

    int child_processes_number = atoi(argv[1]);
    pid_t child_pid;

    for (int i = 0; i < child_processes_number; i++) {
        child_pid = fork();
        check_processes(child_pid);
    }
    while (wait(NULL) > 0);

    printf("\nNUMBER OF CHILD PROCESSES: %d \n \n", child_processes_number);
    return EXIT_SUCCESS;
}