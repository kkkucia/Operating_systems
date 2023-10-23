#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define SIGUSR1 10

pid_t child_pid;
sigset_t mask, set;
int parent_process;

void raise_signals() {
    int i = 0;
    while (i < 10) {
        sleep(1);
        raise(SIGUSR1);
        i++;
    }
}

int func_ignore() {
    if (parent_process) {
        signal(SIGUSR1, SIG_IGN);

        int i = 0;
        while (i < 5) {
            printf("Proces rodzica działa (ignoruje sygnał SIGUSR1)\n");
            sleep(1);
            raise(SIGUSR1);
            i++;
        }
        printf("Proces rodzica zakończył działanie.\n");
        int execl_error = execl("./exec", "./exec", "ignore", "child_process", NULL);
        if (execl_error == -1) {
            perror("ERROR EXECL FUNCTION");
            return EXIT_FAILURE;
        }
    } else {
        int j = 0;
        while (j < 10) {
            printf("Proces dziecka działa (ignoruje sygnał SIGUSR1)\n");
            sleep(1);
            raise(SIGUSR1);
            j++;
        }
        printf("Proces dziecka zakończył działanie.\n");
        return 0;
    }
    return EXIT_SUCCESS;
}


void handlerSIGUSR1(int signum) {
    printf("Otrzymano sygnał: %d ; PPID: %d ; PID %d\n", signum, getppid(), getpid());
}


void check_blocked_signals() {
    if (sigpending(&set) < 0) {
        perror("SIGPENDING ERROR");
        exit(EXIT_FAILURE);
    }

    if (sigismember(&set, SIGUSR1) == 1) {
        printf("[Proces rodzica - Przed wysłaniem sygnału] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
    } else {
        printf("[Proces rodzica - Przed wysłaniem sygnału] Sygnał SIGUSR1 nie oczekuje na oblokowanie\n");
    }
    raise(SIGUSR1);

    if (sigpending(&set) < 0) {
        perror("SIGPENDING ERROR");
        exit(EXIT_FAILURE);
    }

    if (sigismember(&set, SIGUSR1) == 1) {
        printf("[Proces rodzica - Po wysłaniu sygnału] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
    } else {
        printf("[Proces rodzica - Po wysłaniu sygnału] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
    }
    return;
}


int func_mask() {
    if (parent_process) {
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);

        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
            perror("ERROR - CANNOT MASK SIGNAL");
            return EXIT_FAILURE;
        }
        printf("Proces rodzica \n");

        if (sigismember(&mask, SIGUSR1)) {
            printf("Sygnał SIGUSR1 jest w masce.\n");
        } else {
            printf("Sygnał SIGUSR1 nie jest w masce.\n");
        }

        check_blocked_signals();

        printf("Proces rodzica zakończył działanie.\n");
        int execl_error = execl("./exec", "./exec", "mask", "child_process", NULL);
        if (execl_error == -1) {
            perror("ERROR EXECL FUNCTION");
            return EXIT_FAILURE;
        }
    } else {
        printf("Proces dziecka \n");

        if (sigpending(&set) < 0) {
            perror("SIGPENDING ERROR");
            return EXIT_FAILURE;
        }

        if (sigismember(&set, SIGUSR1) == 1) {
            printf("[Process dziecka] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
        } else {
            printf("[Process dziecka] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
        }
        printf("Proces dziecka zakończył działanie.\n");
        return 0;
    }
    return EXIT_SUCCESS;
}

int func_pending() {
    if (parent_process) {
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
            perror("ERROR - CANNOT MASK SIGNAL");
            return EXIT_FAILURE;
        }
        raise(SIGUSR1);

        if (sigpending(&set) < 0) {
            perror("SIGPENDING ERROR");
            exit(EXIT_FAILURE);
        }

        if (sigismember(&set, SIGUSR1) == 1) {
            printf("[Proces rodzica] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
        } else {
            printf("[Proces rodzica] Sygnał SIGUSR1 nie oczekuje na oblokowanie\n");
        }

        printf("Proces rodzica zakończył działanie.\n");
        int execl_error = execl("./exec", "./exec", "pending", "child_process", NULL);
        if (execl_error == -1) {
            perror("ERROR EXECL FUNCTION");
            return EXIT_FAILURE;
        }
    } else {
        if (sigpending(&set) < 0) {
            perror("SIGPENDING ERROR");
            exit(EXIT_FAILURE);
        }

        if (sigismember(&set, SIGUSR1) == 1) {
            printf("[Proces dziecka] Sygnał SIGUSR1 oczekuje na oblokowanie\n");
        } else {
            printf("[Proces dziecka] Sygnał SIGUSR1 nie oczekuje na oblokowanie\n");
        }

        printf("Proces dziecka zakończył działanie.\n");
        return 0;
    }
    return EXIT_SUCCESS;
}


int main(int argc, const char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "ERROR - INVALID NUMBER OF ARGUMENTS \n");
        return EXIT_FAILURE;
    }
    if (argc == 3 && strcmp(argv[2], "child_process") != 0) {
        fprintf(stderr, "ERROR - argv[3] CANNOT BE SET MANUALLY\n");
        return EXIT_FAILURE;
    }

    if (argc == 3 && strcmp(argv[2], "child_process") == 0) {
        parent_process = 0;
    } else {
        parent_process = 1;
    }

    if (strcmp(argv[1], "ignore") == 0) {
        func_ignore();

    } else if (strcmp(argv[1], "mask") == 0) {
        func_mask();

    } else if (strcmp(argv[1], "pending") == 0) {
        func_pending();

    } else {
        fprintf(stderr, "INVALID ARGUMENT - EXPECTED 1 ARGUMENT: [ignore/mask/pending] \n ");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}