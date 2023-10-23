#pragma once

#include <stdbool.h>
#include <signal.h>

typedef struct {
    char *src;
    char *dst;
    int number;
} fields;


char *create_grid();

void destroy_grid(char *grid);

void draw_grid(char *grid);

void init_grid(char *grid);

bool is_alive(int row, int col, char *grid);

void init_threads(char *src, char *dst);

void update_grid_with_threads(char *src, char *dst);

void *redraw_field(void *arg);

void ignore_handler(int sig, siginfo_t *info, void *ucontext);

void update_grid(char *src, char *dst);


