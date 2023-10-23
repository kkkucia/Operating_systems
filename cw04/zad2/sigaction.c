#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define SIGUSR1 10
#define MAX_CALL_NUM 5

void info_handler(int signo, siginfo_t *info, void *ucontext) {
    printf("PID           : %d\n", info->si_pid);
    printf("UID           : %d\n", info->si_uid);
    printf("Signal number : %d\n", info->si_signo);
    printf("Signal code   : %d\n", info->si_code);
    printf("Signal status : %x\n", info->si_status);
}

void checkSIGINFO(struct sigaction sigAct) {
    sigAct.sa_sigaction = &info_handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_SIGINFO;

    if(sigaction(SIGUSR1, &sigAct, NULL) == -1) {
        perror("SIGACTION ERROR ");
        exit(EXIT_FAILURE);
    }
    printf("\nRise signal in parent : SIGUSR1 \n");
    raise(SIGUSR1);

    if(fork() == 0){
        printf("\nRise signal in child : SIGUSR1 \n");
        raise(SIGUSR1);
    }else{
        perror("ERROR - CANNOT CREATE CHILD PROCESS");
        exit(EXIT_FAILURE);
    }
}
int signal_call_num;
int signal_call_depth;

void nodefer_handler(int signo, siginfo_t *info, void *ucontext) {
    printf("START HANDLER: sig num: %d ; depth sig: %d\n", signal_call_num, signal_call_depth);
    signal_call_num ++;
    signal_call_depth ++;
    while (signal_call_num < MAX_CALL_NUM){
        raise(SIGUSR1);
        signal_call_num ++;
    }
    signal_call_depth --;

    printf("END   HANDLER: sig num: %d ; depth sig: %d\n", signal_call_num, signal_call_depth);
}


void checkNODEFER(struct sigaction sigAct){
    sigAct.sa_sigaction = &nodefer_handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_NODEFER;
    signal_call_num = 0;
    signal_call_depth = 0;

    if(sigaction(SIGUSR1, &sigAct, NULL) == -1) {
        perror("SIGACTION ERROR ");
        exit(EXIT_FAILURE);
    }
    raise(SIGUSR1);
}

void reset_handler(int signo, siginfo_t *info, void *ucontext) {
    printf("\nSygnał obsługiwany przez handler\n");
    printf("Signal number : %d\n", info->si_signo);
    printf("Signal code   : %d\n", info->si_code);
}

void checkRESETHAND(struct sigaction sigAct){
    sigAct.sa_sigaction = &reset_handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_RESETHAND;

    printf("\nsigaction() z flagą SA_RESETHAND\n");
    if(sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("SIGACTION ERROR ");
        exit(EXIT_FAILURE);
    }
    int sigCallNo = 1;
    printf("%d wywoałanie sygnału SIGINT\n", sigCallNo);
    sigCallNo ++;
    raise(SIGINT);

    printf("\nsigaction() z flagą SA_RESETHAND\n");
    if(sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("SIGACTION ERROR ");
        exit(EXIT_FAILURE);
    }
    printf("%d wywoałanie sygnału SIGINT\n", sigCallNo);
    sigCallNo ++;
    raise(SIGINT);
    printf("%d wywoałanie sygnału SIGINT\n", sigCallNo);
    raise(SIGINT);
}


int main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "ERROR - INVALID ARGUMENTS \n");
    }

    struct sigaction sigAct;
    //pozwala na przekazanie dodatkowych informacji o sygnale, takich jak
    // identyfikator procesu wysyłającego sygnał czy szczegóły dotyczące sygnału
    printf("\n \n---[SA_SIGINFO FLAG TEST]---\n");
    checkSIGINFO(sigAct);

    // wyłącza blokowanie sygnałów podczas ich obsługi przez procedurę obsługi sygnału.
    // Dzięki temu, w przypadku ponownego otrzymania sygnału w czasie jego obsługi,
    // procedura obsługi zostanie przerwana i uruchomiona ponownie.
    printf("\n \n---[SA_NODEFER FLAG TEST]---\n");
    checkNODEFER(sigAct);

    //jeśli proces otrzyma ten sam sygnał po raz kolejny, to zostanie on obsłużony
    // z użyciem domyślnej procedury obsługi, a nie zainstalowanej przez funkcję sigaction().
    printf("\n \n---[SA_RESETHAND FLAG TEST]---\n");
    checkRESETHAND(sigAct);

    return EXIT_SUCCESS;
}