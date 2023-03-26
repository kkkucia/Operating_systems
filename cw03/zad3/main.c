#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_ARG_COUNT 3
#define MAX_ARG_SIZE 255

int file_handling(const char *full_path, const char *file_start) {
    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        perror("ERROR INVALID INPUT FILE");
        return EXIT_FAILURE;
    }
    size_t file_start_length = strlen(file_start);
    char buffer[file_start_length];

    size_t correctly_size = fread(buffer, sizeof(char), file_start_length, file);
    if (correctly_size != file_start_length) {
        perror("ERROR LOADING DATA");
        fclose(file);
        return EXIT_FAILURE;
    }
    buffer[file_start_length] = '\0';
    fclose(file);

    if (strcmp(file_start, buffer) == 0) {
        printf("PATH: %s ; PID: %d\n", full_path, getpid());
        fflush(NULL);
    }
    return EXIT_SUCCESS;
}


int dir_handling(const char *full_path, const char *name, const char *file_start) {
    DIR *dir = opendir(full_path);
    if (dir == NULL) {
        perror("ERROR CANNOT OPEN DIRECTORY");
        return EXIT_FAILURE;
    }
    struct dirent *dir_file = readdir(dir);

    while (dir_file != NULL) {
        if (strcmp(dir_file->d_name, ".") == 0 || strcmp(dir_file->d_name, "..") == 0) {
            dir_file = readdir(dir);
            continue;
        }
        char child_path[PATH_MAX];
        size_t child_path_size = strlen(full_path) + strlen("/") + strlen(dir_file->d_name);

        if ( child_path_size > PATH_MAX ) {
            fprintf(stderr, "ERROR - PATH IS TO LONG");
            closedir(dir);
            return -1;
        }

        strcpy(child_path, full_path);
        strcat(child_path, "/");
        strcat(child_path, dir_file->d_name);

        int child_pid = fork();
        if (child_pid == 0) {
            int execl_error = execl(name, name, child_path, file_start, NULL);
            if (execl_error == -1) {
                if (closedir(dir) == -1) {
                    perror("ERROR CANNOT CLOSE DIRECTORY");
                    return EXIT_FAILURE;
                }
            }
        } else if (child_pid == -1) {
            perror("ERROR - CANNOT MAKE A CHILD PROCESS");
            return EXIT_FAILURE;
        }
        dir_file = readdir(dir);
    }
    closedir(dir);
    return EXIT_SUCCESS;
}


int main(int argc, char **argv) {
    if (argc != MAX_ARG_COUNT) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 2 ARGUMENTS");
        return EXIT_FAILURE;
    }
    if (strlen(argv[2]) > MAX_ARG_SIZE) {
        fprintf(stderr, "INVALID ARGUMENTS - SECOND ARGUMENT CANNOT BE LONGER THAN 255 BYTES");
        return EXIT_FAILURE;
    }
    const char *path = argv[1];
    const char *file_start = argv[2];

    struct stat statistic;

    if (stat(path, &statistic) == -1) {
        perror("ERROR - CANNOT READ stat() INFORMATION");
        return EXIT_FAILURE;
    }

    if (!S_ISDIR(statistic.st_mode)) {
        file_handling(path, file_start);
    } else {
        dir_handling(path, argv[0], file_start);
    }
    while (wait(NULL) > 0);
    return EXIT_SUCCESS;
}