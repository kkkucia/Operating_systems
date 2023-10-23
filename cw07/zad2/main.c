#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SEMAPHORE_HAIRDRESSERS "/hairdresser_sem"
#define SEMAPHORE_CHAIRS       "/chairs_sem"
#define SEMAPHORE_WAITING      "/waiting_sem"
#define SHARED_MEMORY_NAME     "/shared_memory"

typedef struct {
    int hairdressersQuantity; //m
    int chairsQuantity; //n
    int waitingRoomCapacity; //p
} salon_t;


sem_t *hairdressers_sem, *chairs_sem, *waiting_sem, *client_sem;
int shared_memory_fd;
salon_t *salon;
static int client_number = 0;


void set_up_shared_memory() {
    shared_memory_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shared_memory_fd == -1) {
        perror("shm_open - cannot open shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_fd, sizeof(salon_t)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    salon = mmap(NULL, sizeof(salon_t), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if (salon == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
}


void set_up_semaphores() {
    hairdressers_sem = sem_open(SEMAPHORE_HAIRDRESSERS, O_CREAT, 0666, salon->hairdressersQuantity);
    if (hairdressers_sem == SEM_FAILED) {
        perror("sem_open hairdressers");
        exit(EXIT_FAILURE);
    }

    chairs_sem = sem_open(SEMAPHORE_CHAIRS, O_CREAT, 0666, salon->chairsQuantity);
    if (chairs_sem == SEM_FAILED) {
        perror("sem_open chairs");
        exit(EXIT_FAILURE);
    }

    waiting_sem = sem_open(SEMAPHORE_WAITING, O_CREAT, 0666, salon->waitingRoomCapacity);
    if (waiting_sem == SEM_FAILED) {
        perror("sem_open waiting rooms");
        exit(EXIT_FAILURE);
    }
}

void add_client_to_waiting_room(int client_number) {
    int *places = malloc(sizeof(int));
    if (sem_trywait(waiting_sem) != -1) {
        sem_getvalue(waiting_sem, places);
        printf("Klient numer %d usiadł w poczekalni. \n", client_number);
        printf("Aktualne miejsca w poczekalni: %d \n", *places);
    }
    free(places);
}

void delete_client_from_waiting_room(int client_number) {
    int *places = malloc(sizeof(int));
    if (sem_post(waiting_sem) != -1) {
        sem_getvalue(waiting_sem, places);
        printf("Klient numer %d opuścił poczekalnie. \n", client_number);
        printf("Aktualne miejsca w poczekalni: %d \n", *places);
    }
    free(places);
}


int is_waitting_place() {
    //sprawdz czy jest wolne miejsce w poczekalni if jest zwróć 0 else 1
    int *places = malloc(sizeof(int));
    sem_getvalue(waiting_sem, places);
    if ((*places <= 0) || (*places > salon->waitingRoomCapacity)) {
        free(places);
        return 1;
    }
    free(places);
    return 0;
}

void making_haircut(int client_number) {
    printf("Klient numer %d rozpoczyna usługę.\n", client_number);
    sleep(rand() % 5 + 5);
    printf("Klient numer %d ma zrobioną fryzurę.\n", client_number);
}

void run_barber_salon() {
    printf("Otwarcie salonu fryzjerskiego! \n");
    int *hairdressers = malloc(sizeof(int));
    int *chairs = malloc(sizeof(int));
    int *waitingPlaces = malloc(sizeof(int));

    while (1) {
        srand(time(NULL));
        sleep(rand() % 3 + 1);

        sem_getvalue(hairdressers_sem, hairdressers);
        sem_getvalue(chairs_sem, chairs);
        sem_getvalue(waiting_sem, waitingPlaces);

        printf("\nFryzjerzy: %d | Fotele: %d | Miejsca w poczekalni: %d \n", *hairdressers, *chairs, *waitingPlaces);

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
                if (sem_trywait(hairdressers_sem) != -1) {
                    printf("Klient numer %d znalazł fryzjera. \n", client_number);

                    //szukanie fotela
                    sem_wait(chairs_sem);

                    printf("Klient numer %d znalazł fotel.\n", client_number);

                    making_haircut(client_number);
                } else {
                    printf("Klient numer %d siada w poczekalni. \n", client_number);

                    add_client_to_waiting_room(client_number);

                    sem_wait(hairdressers_sem); //czeka na zasoby i przeprowadza operacje uzyj fryzjera

                    printf("Klient numer %d znalazł fryzjera. \n", client_number);

                    delete_client_from_waiting_room(client_number); //usuwa 1 klienta z poczekalni = poczekalnia o 1 krótsza

                    sem_wait(chairs_sem); //czeka na zasoby i przeprowadza operacje uzyj fotela
                    printf("Klient numer %d znalazł fotel.\n", client_number);

                    making_haircut(client_number);
                }
                sem_post(hairdressers_sem); //fryzjer wraca do spania
                sem_post(chairs_sem); //zwalnianie fotela
            }
            exit(EXIT_SUCCESS);
        } else if (client_pid == -1) {
            perror("FORK - cannot create child process.\n");
            exit(EXIT_FAILURE);
        }
    }
    free(hairdressers);
    free(chairs);
    free(waitingPlaces);
}


void validate_args(int argc, char **argv) {
    if (argc != 4) {
        printf("Invalid arguments. [Hairdressers quantity][Chairs quantity][waiting room capacity]. \n");
        exit(EXIT_FAILURE);
    }

    set_up_shared_memory();

    salon->hairdressersQuantity = atoi(argv[1]);
    salon->chairsQuantity = atoi(argv[2]);
    salon->waitingRoomCapacity = atoi(argv[3]);

    if (salon->hairdressersQuantity < salon->chairsQuantity) {
        printf("Invalid arguments. Hairdressers quantity has to be greater than chairs quantity. \n");
        exit(EXIT_FAILURE);
    }
}

void exit_handler(int sig_no) {
    printf("\nZamykanie salonu fryzjerskiego!\n");
    sem_unlink(SEMAPHORE_HAIRDRESSERS);
    sem_unlink(SEMAPHORE_CHAIRS);
    sem_unlink(SEMAPHORE_WAITING);
    sem_unlink(SHARED_MEMORY_NAME);
    munmap(salon, sizeof(salon_t));
    sem_close(hairdressers_sem);
    sem_close(chairs_sem);
    sem_close(waiting_sem);
    exit(EXIT_SUCCESS);
}

void prepare_exit_barber_salon() {
    signal(SIGINT, exit_handler);
    signal(SIGQUIT, exit_handler);
}


int main(int argc, char **argv) {

    validate_args(argc, argv);

    set_up_semaphores();

    prepare_exit_barber_salon();

    run_barber_salon();

    return 0;
}