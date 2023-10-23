#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 4096
#define LINE_SIZE 1024

void handle_mail_sort(char *col_sort) {

    FILE *fp;
    if ((fp = popen("mail -H", "r")) == NULL) {
        perror("ERROR - CANNOT READ mail");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFF_SIZE];
    size_t size = fread(buffer, sizeof(char), BUFF_SIZE, fp);
    buffer[size] = 0;

    char command[LINE_SIZE] = "";
    strcat(command, " grep \"U \\\\| N\""); //unread and new
    strcat(command, col_sort);

    FILE *sort_fp = popen(command, "w");
    fwrite(buffer, sizeof(char), strlen(buffer), sort_fp);

    if (pclose(fp) != 0) {
        perror("ERROR - CANNOT CLOSE PIPE");
        exit(EXIT_FAILURE);
    }

    if (pclose(sort_fp) != 0) {
        perror("ERROR - CANNOT CLOSE PIPE");
        exit(EXIT_FAILURE);
    }
}

void handle_mail_send(char *email, char *title, char *content) {
    char command[LINE_SIZE];
    sprintf(command, "mail -s \"%s\" %s", title, email);

    FILE *fp = popen(command, "w");
    if (fp == NULL) {
        perror("ERROR - CANNOT RUN mail\n");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s", content);

    if (pclose(fp) != 0) {
        perror("ERROR - CANNOT CLOSE PIPE\n");
        exit(EXIT_FAILURE);
    }

    printf("Email has been sent!\n");
}


int main(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "nadawca") == 0) {
            handle_mail_sort(" | sort -k3");

        } else if (strcmp(argv[1], "data") == 0) {
            handle_mail_sort(""); //mail sam porządkuje w kolejności rosnącej daty :)

        } else {
            fprintf(stderr, "EXPECTED ARGUMENTS: 'nadawca' OR 'data' \n");
            exit(EXIT_FAILURE);
        }
    } else if (argc == 4) {
        handle_mail_send(argv[1], argv[2], argv[3]);

    } else {
        fprintf(stderr, "ERROR - Invalid number of arguments (expexted : <email> <title> <content>)\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
