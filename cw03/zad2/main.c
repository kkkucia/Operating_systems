#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 1 ARGUMENT: [Directory path]");
        return EXIT_FAILURE;
    }
    printf("%s", argv[1]);
    fflush(stdout); //wypisać zawartość bufora tuż przed wywołaniem funkcji execl, używając funkcji fflush(stdout).
    execl("/bin/ls", "ls", argv[1], NULL);

    return EXIT_SUCCESS;
}