#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define SIGUSR1 10

pid_t child_pid;
sigset_t mask, set;

void raise_signals() {
    int i = 0;
    while (i < 10) {
        sleep(1);
        raise(SIGUSR1);
        i++;
    }
}

// odpowiednio w procesie przodka ustawia ignorowanie
int func_ignore() {
    signal(SIGUSR1, SIG_IGN);

    int i = 0;
    while (i < 5) {
        printf("Proces rodzica działa (ignoruje sygnał SIGUSR1)\n");
        sleep(1);
        raise(SIGUSR1);
        i++;
    }

    child_pid = fork();
    if (child_pid < 0) {
        perror("ERROR - CANNOT CREATE CHILD PROCESS");
        return EXIT_FAILURE;
    } else if (child_pid == 0) {
        int j = 0;
        while (j < 10) {
            printf("Proces dziecka działa (ignoruje sygnał SIGUSR1)\n");
            sleep(1);
            raise(SIGUSR1);
            j++;
        }
        printf("Proces dziecka zakończył działanie.\n");
        return 0;
    } else {
        int k = 0;
        while (k < 5) {
            printf("Proces rodzica działa (ignoruje sygnał SIGUSR1)\n");
            sleep(1);
            raise(SIGUSR1);
            k++;
        }
        printf("Proces rodzica zakończył działanie.\n");
        return EXIT_SUCCESS;
    }
}

void handlerSIGUSR1(int signum) {
    printf("Otrzymano sygnał: %d ; PPID: %d ; PID %d\n", signum, getppid(), getpid());
}

// instaluje handler obsługujący sygnał wypisujący komunikat o jego otrzymaniu,
int func_handler() {
    signal(SIGUSR1, handlerSIGUSR1);
    raise_signals();

    child_pid = fork();
    if (child_pid < 0) {
        perror("ERROR - CANNOT CREATE CHILD PROCESS");
        return EXIT_FAILURE;
    } else if (child_pid == 0) {
        raise_signals();
        printf("Proces dziecka zakończył działanie.\n");
        return 0;
    } else {
        raise_signals();
        printf("Proces rodzica zakończył działanie.\n");
        return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
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

// maskuje ten sygnał oraz sprawdza (przy zamaskowaniu tego sygnału) czy wiszący/oczekujący sygnał jest widoczny w procesie
int func_mask() {
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        perror("ERROR - CANNOT MASK SIGNAL");
        return EXIT_FAILURE;
    }

    if (sigismember(&mask, SIGUSR1)) {
        printf("Sygnał SIGUSR1 jest w masce.\n");
    } else {
        printf("Sygnał SIGUSR1 nie jest w masce.\n");
    }

    check_blocked_signals();

    child_pid = fork();
    if (child_pid < 0) {
        perror("ERROR - CANNOT CREATE CHILD PROCESS");
        return EXIT_FAILURE;
    } else if (child_pid == 0) {

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
        return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
}

// testowane jest sprawdzenie, czy sygnał czekający w przodku jest widoczny w potomku
// wysyłam do przodka sygnał i sprawdzam czy jest widoczny w potomku
int func_pending() {
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
            perror("ERROR - CANNOT MASK SIGNAL");
            return EXIT_FAILURE;
        }
        raise(SIGUSR1);

        child_pid = fork();
        if (child_pid < 0) {
            perror("ERROR - CANNOT CREATE CHILD PROCESS");
            return EXIT_FAILURE;
        } else if (child_pid == 0) {
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

        } else {
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
            return EXIT_SUCCESS;
        }
        return EXIT_SUCCESS;
    }

//działanie związane z ignore/handler/mask/pending
//a następnie przy pomocy funkcji raise wysyła sygnał do samego siebie oraz wykonuje odpowiednie dla danej opcji działania,
//po czym tworzy potomka funkcją fork i ponownie przy pomocy funkcji raise potomek wysyła sygnał do samego siebie

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "INVALID NUMBER OF ARGUMENTS - EXPECTED 1 ARGUMENT \n");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "ignore") == 0) {
        func_ignore();

    } else if (strcmp(argv[1], "handler") == 0) {
        func_handler();

    } else if (strcmp(argv[1], "mask") == 0) {
        func_mask();

    } else if (strcmp(argv[1], "pending") == 0) {
        func_pending();

    } else {
        fprintf(stderr, "INVALID ARGUMENT - EXPECTED 1 ARGUMENT: [ignore/handler/mask/pending] \n ");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}