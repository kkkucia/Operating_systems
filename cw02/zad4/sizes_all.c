#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>

long long total = 0;

int get_file_size(const char *file_path, const struct stat *statistic, int flag){
    if(flag == FTW_D){
        printf("DIR:  %s : IS A DIRECTORY\n", file_path);
    }else if(!S_ISDIR(statistic->st_mode)){
        printf("    %ld  %s\n", statistic->st_size, file_path);
        total += statistic->st_size;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "INVALID ARGUMENTS - EXPECTED 1 ARGUMENTS: [Directory]");
        return 1;
    }
    const char *dirname = argv[1];

    int func_ftw = ftw(dirname, get_file_size, 1);
    if( func_ftw == -1){
        fprintf(stderr, "ERROR");
    }

    printf("%lld  total \n", total);
    return 0;
}