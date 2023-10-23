#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define REINDEER_QUANTITY 9
#define ELVES_QUANTITY 10
#define MAX_WAITING_ELVES 3
#define MAX_DELIVERED_GIFTS 3

pthread_t santa_thread;
pthread_t reindeer_threads[REINDEER_QUANTITY];
pthread_t elf_threads[ELVES_QUANTITY];

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;

pthread_t elves_queue[MAX_WAITING_ELVES];
int num_elves_waiting;
int num_reindeer_waiting;
int gift_deliveries;

void reindeer_holidays() {
    sleep(rand() % 6 + 5);
}

void elf_works() {
    sleep(rand() % 4 + 2);
}

void delivering_toys() {
    sleep(rand() % 5 + 2);
}

void helping_elves() {
    sleep(rand() % 3 + 1);
}


void *santa(void *arg) {
    while (gift_deliveries < MAX_DELIVERED_GIFTS) {
        pthread_mutex_lock(&santa_mutex);

        while (num_elves_waiting < MAX_WAITING_ELVES && num_reindeer_waiting < REINDEER_QUANTITY) {
            printf("Mikołaj  : Zasypiam\n");
            pthread_cond_wait(&santa_cond, &santa_mutex);
            printf("Mikołaj  : Wstaję\n");
        }

        if (num_elves_waiting == MAX_WAITING_ELVES) {
            pthread_mutex_unlock(&santa_mutex);
            printf("Elf      : Mikołaj rozwiązuje problem elfów, ID %lu, %lu, %lu\n", elves_queue[0], elves_queue[1],
                   elves_queue[2]);
            helping_elves();
            pthread_mutex_lock(&santa_mutex);
            num_elves_waiting = 0;
            pthread_cond_broadcast(&elves_cond);
        }

        if (num_reindeer_waiting == REINDEER_QUANTITY) {
            pthread_mutex_unlock(&santa_mutex);
            printf("Mikołaj  : Dostarczam prezenty %d raz\n", gift_deliveries + 1);
            delivering_toys();
            pthread_mutex_lock(&santa_mutex);
            num_reindeer_waiting = 0;
            gift_deliveries++;
            pthread_cond_broadcast(&reindeer_cond);
        }

        if (gift_deliveries == 3) {
            break;
        }
        pthread_mutex_unlock(&santa_mutex);
    }
    printf("Mikołaj  : Kończe pracę\n");
    pthread_exit(NULL);
}


void *elf(void *arg) {
    pthread_t elf_id = pthread_self();

    while (1) {
        elf_works();
        pthread_mutex_lock(&santa_mutex);

        if (num_elves_waiting < 3) {
            elves_queue[num_elves_waiting] = elf_id;
            num_elves_waiting++;
            printf("Elf      : Czeka %d elfów na Mikołaja, ID %lu\n", num_elves_waiting, elf_id);

            if (num_elves_waiting == 3) {
                printf("Elf      : Wybudzam Mikołaja, ID %lu\n", elf_id);
                pthread_cond_signal(&santa_cond);
            }
            pthread_cond_wait(&elves_cond, &santa_mutex);
        } else {
            printf("Elf      : Samodzielnie rozwiązuję swój problem, ID %lu\n", elf_id);
        }
        pthread_mutex_unlock(&santa_mutex);
    }
}


void *reindeer(void *arg) {
    pthread_t reindeer_id = pthread_self();

    while (1) {
        reindeer_holidays();
        pthread_mutex_lock(&santa_mutex);
        num_reindeer_waiting++;
        printf("Renifer  : Czeka %d reniferów na Mikołaja, ID %lu \n", num_reindeer_waiting, reindeer_id);

        if (num_reindeer_waiting == REINDEER_QUANTITY) {
            printf("Renifer  : Wybudzam Mikołaja, ID %lu\n", reindeer_id);
            pthread_cond_signal(&santa_cond);
        }

        while (num_elves_waiting > 0) {
            pthread_cond_wait(&elves_cond, &santa_mutex);
        }

        while (num_reindeer_waiting > 0) {
            pthread_cond_wait(&reindeer_cond, &santa_mutex);
        }
        pthread_mutex_unlock(&santa_mutex);
    }
}

void set_up_north_pole() {
    gift_deliveries = 0;
    num_elves_waiting = 0;
    num_reindeer_waiting = 0;

    if (pthread_create(&santa_thread, NULL, santa, NULL) != 0) {
        perror("pthread_create - santa");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ELVES_QUANTITY; i++) {
        if (pthread_create(&elf_threads[i], NULL, elf, NULL) != 0) {
            perror("pthread_create - elf");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < REINDEER_QUANTITY; i++) {
        if (pthread_create(&reindeer_threads[i], NULL, reindeer, NULL) != 0) {
            perror("pthread_create - reindeer");
            exit(EXIT_FAILURE);
        }
    }
}

void waiting_for_end_work() {
    pthread_join(santa_thread, NULL);

    for (int i = 0; i < ELVES_QUANTITY; i++) {
        pthread_cancel(elf_threads[i]);
    }

    for (int i = 0; i < REINDEER_QUANTITY; i++) {
        pthread_cancel(reindeer_threads[i]);
    }

    pthread_mutex_destroy(&santa_mutex);
    pthread_cond_destroy(&reindeer_cond);
    pthread_cond_destroy(&elves_cond);
    pthread_cond_destroy(&santa_cond);
}

int main() {
    srand(time(NULL));

    set_up_north_pole();
    waiting_for_end_work();

    return 0;
}