#include "grid.h"
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

const int grid_width = 30;
const int grid_height = 30;
const int field_quantity = grid_width * grid_height;
pthread_t *threads;

char *create_grid() {
    return malloc(sizeof(char) * field_quantity);
}

void destroy_grid(char *grid) {
    free(grid);
}

void draw_grid(char *grid) {
    for (int i = 0; i < grid_height; ++i) {
        // Two characters for more uniform spaces (vertical vs horizontal)
        for (int j = 0; j < grid_width; ++j) {
            if (grid[i * grid_width + j]) {
                mvprintw(i, j * 2, "■");
                mvprintw(i, j * 2 + 1, " ");
            } else {
                mvprintw(i, j * 2, " ");
                mvprintw(i, j * 2 + 1, " ");
            }
        }
    }
    refresh();
}

void init_grid(char *grid) {
    for (int i = 0; i < field_quantity; ++i)
        grid[i] = rand() % 2 == 0;
}

bool is_alive(int row, int col, char *grid) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            int r = row + i;
            int c = col + j;
            if (r < 0 || r >= grid_height || c < 0 || c >= grid_width) {
                continue;
            }
            if (grid[grid_width * r + c]) {
                count++;
            }
        }
    }
    if (grid[row * grid_width + col]) {
        if (count == 2 || count == 3)
            return true;
        else
            return false;
    } else {
        if (count == 3)
            return true;
        else
            return false;
    }
}

void update_grid(char *src, char *dst) {
    for (int i = 0; i < grid_height; ++i) {
        for (int j = 0; j < grid_width; ++j) {
            dst[i * grid_width + j] = is_alive(i, j, src);
        }
    }
}

void update_grid_with_threads(char *src, char *dst) {
    for (int i = 0; i < field_quantity; ++i) {
        pthread_kill(threads[i], SIGUSR1);
    }
}

void *redraw_field(void *arg) {
    fields *field = (fields *) arg;
    int row_position, col_position;

    while (1) {
        row_position = (field->number) / grid_width;
        col_position = (field->number) % grid_width;

        field->dst[field->number] = is_alive(row_position, col_position, field->src);

        pause();

        char *tmp = field->src;
        field->src = field->dst;
        field->dst = tmp;
    }
    return NULL;
}


void ignore_handler(int sig, siginfo_t *info, void *ucontext) {};


void init_threads(char *src, char *dst) {
    threads = malloc(sizeof(pthread_t) * field_quantity);

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = ignore_handler;

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < field_quantity; i++) {
        fields *field = malloc(sizeof(fields));
        field->src = src;
        field->dst = dst;
        field->number = i;

        if (pthread_create(&threads[i], NULL, redraw_field, (void *) field) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        };
    }
}