#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>


key_t SEMAPHORE_KEY;
int SEMAPHORE_ID;
struct sembuf sops[1];

int hairdressersQuantity; //m
int chairsQuantity; // n
int waitingRoomCapacity; //p

static int client_number = 0;

typedef union Semun {
    int val;                 /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array;   /* Array for GETALL, SETALL */
    struct seminfo *__buf;   /* Buffer for IPC_INFO */
} Semun;

Semun args;

void set_up_semaphore() {
    SEMAPHORE_KEY = ftok(getenv("HOME"), 1);
    if (SEMAPHORE_KEY == -1) {
        perror("FTOK - cannot create a key.");
    }

    SEMAPHORE_ID = semget(SEMAPHORE_KEY, 3, IPC_CREAT | 0666);
    if (SEMAPHORE_ID == -1) {
        perror("SEMGET - cannot create a semaphore.");
        exit(EXIT_FAILURE);
    }

    args.val = hairdressersQuantity;
    semctl(SEMAPHORE_ID, 0, SETVAL, args);

    args.val = chairsQuantity;
    semctl(SEMAPHORE_ID, 1, SETVAL, args);

    args.val = waitingRoomCapacity;
    semctl(SEMAPHORE_ID, 2, SETVAL, args);
}


void add_client_to_waiting_room(int client_number) {
    sops[0].sem_op = -1;
    sops[0].sem_num = 2;
    sops[0].sem_flg = IPC_NOWAIT;

    if (semop(SEMAPHORE_ID, sops, 1) != -1) {
        printf("Klient numer %d usiadł w poczekalni. \n", client_number);
        printf("Aktualne miejsca w poczekalni: %d \n", semctl(SEMAPHORE_ID, 2, GETVAL));
    }
}

void delete_client_from_waiting_room(int client_number) {
    sops[0].sem_op = 1;
    sops[0].sem_num = 2;
    sops[0].sem_flg = IPC_NOWAIT;

    if (semop(SEMAPHORE_ID, sops, 1) != -1) {
        printf("Klient numer %d opuścił poczekalnie. \n", client_number);
        printf("Aktualne miejsca w poczekalni: %d \n", semctl(SEMAPHORE_ID, 2, GETVAL));
    }
}


int is_waitting_place() {
    //sprawdz czy jest wolne miejsce w poczekalni if jest zwróć 0 else 1
    int places = semctl(SEMAPHORE_ID, 2, GETVAL);
    if ((places <= 0) || (places > waitingRoomCapacity)) {
        return 1;
    }
    return 0;
}

void making_haircut(int client_number) {
    printf("Klient numer %d rozpoczyna usługę.\n", client_number);
    sleep(rand() % 5 + 5);
    printf("Klient numer %d ma zrobioną fryzurę.\n", client_number);
}

void run_barber_salon() {
    int hairdressers, chairs, waitingPlaces; //0 1 2
    printf("Otwarcie salonu fryzjerskiego! \n");

    while (1) {
        srand(time(NULL));
        sleep(rand() % 3 + 1);

        hairdressers = semctl(SEMAPHORE_ID, 0, GETVAL);
        chairs = semctl(SEMAPHORE_ID, 1, GETVAL);
        waitingPlaces = semctl(SEMAPHORE_ID, 2, GETVAL);

        printf("\nFryzjerzy: %d | Fotele: %d | Miejsca w poczekalni: %d \n", hairdressers, chairs, waitingPlaces);

        pid_t client_pid = fork();
        client_number++;

        if (client_pid == 0) {
            signal(SIGINT, NULL);
            signal(SIGQUIT, NULL);

            printf("Klient numer %d wszedł do salonu. \n", client_number);

            if (is_waitting_place() == 1) {
                printf("Klient numer %d nie znalazł wolnego miejsca w poczekalni. Klient wyszedł. \n", client_number);
            } else {

                //szukanie fryzjera
                sops[0].sem_op = -1; //liczba śpiących fryzjerów o 1 mniej
                sops[0].sem_num = 0; //numer semafora
                sops[0].sem_flg = IPC_NOWAIT;

                //sprawdzenie czy jest wolny fryzjer
                if (semop(SEMAPHORE_ID, sops, 1) != -1) {
                    printf("Klient numer %d znalazł fryzjera. \n", client_number);

                    sops[0].sem_num = 1; //liczba foteli!
                    sops[0].sem_flg = 0; //czeka na zasoby fotelowe
                    semop(SEMAPHORE_ID, sops, 1);

                    printf("Klient numer %d znalazł fotel.\n", client_number);

                    making_haircut(client_number);

                } else {
                    printf("Klient numer %d siada w poczekalni. \n", client_number);

                    add_client_to_waiting_room(client_number);

                    sops[0].sem_num = 0;
                    sops[0].sem_flg = 0; //czeka na zasoby i przeprowadza operacje uzyj fryzjera
                    semop(SEMAPHORE_ID, sops, 1);
                    printf("Klient numer %d znalazł fryzjera. \n", client_number);

                    delete_client_from_waiting_room(client_number); //usuwa 1 klienta z poczekalni = poczekalnia o 1 krótsza

                    sops[0].sem_num = 1;
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0; //czeka na zasoby i przeprowadza operacje uzyj fotela
                    semop(SEMAPHORE_ID, sops, 1);
                    printf("Klient numer %d znalazł fotel.\n", client_number);

                    making_haircut(client_number);
                }
                sops[0].sem_num = 1;
                sops[0].sem_op = 1; // zwalnia fotel
                semop(SEMAPHORE_ID, sops, 1);

                sops[0].sem_num = 0;
                sops[0].sem_op = 1; // fryzjer wraca do spania
                semop(SEMAPHORE_ID, sops, 1);
            }
            exit(EXIT_SUCCESS);
        } else if (client_pid == -1) {
            perror("FORK - cannot create child process.\n");
            exit(EXIT_FAILURE);
        }
    }
}

void exit_handler(int sig_no) {
    printf("\nZamykanie salonu fryzjerskiego!\n");
    semctl(SEMAPHORE_ID, 0, IPC_RMID);
    exit(EXIT_SUCCESS);
}

void prepare_exit_barber_salon() {
    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
}


void validate_args(int argc, char **argv) {
    if (argc != 4) {
        printf("Invalid arguments. [Hairdressers quantity][Chairs quantity][Waiting room capacity]. \n");
        exit(EXIT_FAILURE);
    }

    hairdressersQuantity = atoi(argv[1]);
    chairsQuantity = atoi(argv[2]);
    waitingRoomCapacity = atoi(argv[3]);

    if (hairdressersQuantity < chairsQuantity) {
        printf("Invalid arguments. Hairdressers quantity has to be greater than chairs quantity. \n");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char **argv) {

    validate_args(argc, argv);

    set_up_semaphore();

    prepare_exit_barber_salon();

    run_barber_salon();

    return 0;
}