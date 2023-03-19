#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 1 ARGUMENTS: [Directory]");
        return 1;
    }
    const char *dirname = argv[1];

    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        fprintf(stderr, "ERROR CANNOT OPEN DIRECTORY\n");
        return 1;
    }

    long long total = 0;
    struct dirent *dir_file = readdir(dir);
    struct stat statistic;

    while (dir_file != NULL) {
        stat(dir_file->d_name, &statistic);

        if (!S_ISDIR(statistic.st_mode)) {
            printf("%ld  %s\n", statistic.st_size, dir_file->d_name);
            total += statistic.st_size;
        }
        dir_file = readdir(dir);
    }
    printf("%lld  total \n", total);

    if (closedir(dir) == -1) {
        fprintf(stderr, "ERROR CANNOT CLOSE DIRECTORY\n");
        return 1;
    }
    return 0;
}