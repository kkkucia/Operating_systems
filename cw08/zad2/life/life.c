#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdlib.h>
#include <time.h>

char *foreground;
char *background;

void cleaning(){
    endwin(); // End curses mode
    destroy_grid(foreground);
    destroy_grid(background);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Invalid argument - expected one: [number of threads]\n");
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        printf("Invalid argument - number of threads has to be greater than zero!\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr(); // Start curses mode

    foreground = create_grid();
    background = create_grid();
    char *tmp;

    init_grid(foreground);
    if (init_threads(foreground, background, n) == -1){
        cleaning();
        return -1;
    };

    while (true) {
        draw_grid(foreground);
        usleep(500 * 1000);

        // Step simulation
        //update_grid(foreground, background);
        update_grid_with_threads(foreground, background);

        tmp = foreground;
        foreground = background;
        background = tmp;
    }
    cleaning();
    return 0;
}
