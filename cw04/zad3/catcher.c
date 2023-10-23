#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define SIGUSR1 10

struct sigaction sigAct;
pid_t sender_pid;
int status;
int status_counter = 0;
int loop_time = 0;

void catcher_handler(int signo, siginfo_t *info, void *ucontext) {
    sender_pid = info->si_pid;
    status = info->si_status;
    if (status > 5 || status < 1){
        fprintf(stderr, "[CATCHER] Invalid status: %d\n", status);
    }else{
        printf("[CATCHER] Caught status: %d\n", status);
        status_counter ++;
    }
    kill(sender_pid, SIGUSR1);
}
void count_to_100(){
    status = 0;
    for (int i = 1; i <=100; i++){
        printf("[CATCHER] %d\n",i);
    }
}

void curren_time(){
    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    char time_str[50];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    printf("[CATCHER] Current time: %s\n", time_str);
}

void get_current_time(){
    status = 0;
    curren_time();
}

void get_status_counter(){
    status = 0;
    printf("[CATCHER] Number of requests to change status: %d\n", status_counter);
}

void get_loop_time(){
    while (loop_time) {
        time_t t = time(NULL);
        struct tm *local_time = localtime(&t);
        char time_str[50];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        printf("%s\n", time_str);
        sleep(1);
        if (loop_time && (status != 4)){
            loop_time = 0;
        }
    }
}

void set_loop_time(){
    if (!loop_time){
        loop_time = 1;
        get_loop_time();
    }
}

void finish_catcher(){
    status = 0;
    printf("[CATCHER] Finished work!\n");
    fflush(NULL);
    exit(EXIT_SUCCESS);
    return;
}

int main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "[CATCHER] ERROR - INVALID ARGUMENTS \n");
        return EXIT_FAILURE;
    }
    status = 0;

    sigAct.sa_sigaction = &catcher_handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_SIGINFO;

    if(sigaction(SIGUSR1, &sigAct, NULL) == -1) {
        perror("[CATCHER] SIGACTION ERROR ");
        exit(EXIT_FAILURE);
    }

    printf("[CATCHER] MY PID = %d\n", getpid());
    printf("[CATCHER] Waiting for signals...\nType: ./sender {PID} {status}\n");

    while(1){

        switch (status) {
            case 1: count_to_100(); break;
            case 2: get_current_time(); break;
            case 3: get_status_counter(); break;
            case 4: set_loop_time(); break;
            case 5: finish_catcher(); break;
            default: break;
        }
    }
    return EXIT_SUCCESS;
}