#include "header.h"

pid_t pid = -1;
int clientID = -1;
int server_queue, client_queue;
key_t server_key, client_key;
int max_num_clients = 100;

char *get_message_text(char *rest) {
    char *msgText;
    if ((msgText = strtok(rest, "\n\0")) == NULL) {
        fprintf(stderr, "Cannot get message body. There are no more tokens in a string.\n");
        return NULL;
    }
    return msgText;
}

int get_receiverID(char *rest) {
    char *receiverID_str;
    char *writeHere = "";
    if ((receiverID_str = strtok_r(rest, " \0", &writeHere)) == NULL) {
        fprintf(stderr, "Cannot get receiver_id. There are no more tokens in a string.\n");
        return -1;
    }

    int receiverID = (int) strtol(receiverID_str, NULL, 10);
    if (receiverID < 0 || errno) {
        fprintf(stderr, "Invalid receiver id. \n");
        return -1;
    }

    strcpy(rest, writeHere);
    return receiverID;
}

int create_queues() {
    server_key = ftok(getenv("HOME"), 'S');

    server_queue = msgget(server_key, 0);
    if (server_queue == -1) {
        perror("Cannot found server queue!");
        return -1;
    }
    printf("Server queue with key: '%d' was found.\n", server_key);

    client_key = ftok(getenv("HOME"), getpid());
    client_queue = msgget(client_key, IPC_CREAT | 0666);
    if (client_queue == -1) {
        perror("Cannot create client queue!");
        return -1;
    }
    printf("Client queue with key: '%d' was created.\n", client_key);
    return 0;
}

int send_message(message msg) {
    if (msgsnd(server_queue, &msg, MSG_SIZE, 0) == -1) {
        perror("Cannot send message to the server");
        return -1;
    }
    return 0;
}

int send_INIT() {
    message msg = {.msgType = INIT,};
    sprintf(msg.msgText, "%d", client_key);

    if (send_message(msg) == -1) {
        return -1;
    }

    message recived_msg;

    if (msgrcv(client_queue, &recived_msg, MSG_SIZE, INIT, 0) == -1) {
        perror("Cannot get message from the client.");
        return -1;
    }

    clientID = (int) strtol(recived_msg.msgText, NULL, 10);
    printf("Client get id: '%d' \n", clientID);

    return 0;

}

int get_message(message msg) {
    time_t time_sent = msg.timeSent;
    struct tm *local_time = localtime(&time_sent);
    if (!local_time) {
        perror("Cannot get local time.");
        return -1;
    }

    printf("Sender id: %d\nMessage type: %ld\nTime of sending %d-%02d-%02d %02d:%02d:%02d\nMessage: %s\n",
           msg.senderID, msg.msgType,
           local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
           local_time->tm_hour, local_time->tm_min, local_time->tm_sec, msg.msgText
    );
    return 0;
}

int handle_client_queue() {
    message msg;

    if (msgrcv(client_queue, &msg, MSG_SIZE, 0, 0) == -1) { //najstarsza wiadomośc z kolejki
        perror("Cannot get message from client queue!");
        return -1;
    }

    switch (msg.msgType) {
        case ALL:
            printf("Received 2ALL message.\n");
            return get_message(msg);
        case ONE:
            printf("Received 2ONE message.\n");
            return get_message(msg);
        default:
            fprintf(stderr, "Unknown message type: '%ld'.\n", msg.msgType);
            return -1;
    }
}

int send_LIST() {
    puts("Sending LIST message...\n");
    message msg = {.senderID = clientID, .msgType = LIST};

    if (send_message(msg) == -1) {
        return -1;
    }

    message recived_msg;
    if (msgrcv(client_queue, &recived_msg, MSG_SIZE, LIST, 0) == -1) {
        perror("Cannot receive a list of active clients.");
        return -1;
    }

    printf("List of active clients:\n%s\n", recived_msg.msgText);
    return 0;
}

int send_STOP() {
    puts("Sending STOP message...\n");
    message msg = {.senderID = clientID, .msgType = STOP};

    if (pid > 0) {
        kill(pid, SIGKILL);
    }

    if (send_message(msg) == 0) {
        printf("Client was stopped.\n");
        exit(0);
    } else {
        fprintf(stderr, "Cannot stop a client.\n");
        exit(-1);
    }
}

int send_ONE(int receiverID, char *msgText) {
    puts("Sending 2ONE message...\n");
    message msg = {.senderID = clientID, .receiverID = receiverID, .msgType = ONE};
    strcpy(msg.msgText, msgText);
    return send_message(msg);
}

int sendALL(char *msgText) {
    puts("Sending 2ALL message...\n");
    message msg = {.senderID = clientID, .msgType = ALL};
    strcpy(msg.msgText, msgText);
    return send_message(msg);
}


char *get_command(char *msg) {
    printf("%s\n", msg);

    char *command = "";
    size_t length = 0;
    getline(&command, &length, stdin);
    command[strlen(command) - 1] = '\0';

    return command;
}

int listen_input() {
    char *input = get_command("Type a command [LIST/2ALL/2ONE/STOP]: ");

    while (!strlen(input)) {
        puts("Comand not recognized. Try again!");
        input = get_command("Type a command [LIST/2ALL/2ONE/STOP]: ");
    }

    char *rest;
    char *command = strtok_r(input, " \0", &rest);

    if (strcmp(command, "LIST") == 0) {
        return send_LIST();

    } else if (strcmp(command, "2ALL") == 0) {
        char *text = get_message_text(rest);
        if (!text) {
            fprintf(stderr, "Cannot send message to all receivers.\n");
            send_STOP();
            return -1;
        }
        return sendALL(rest);

    } else if (strcmp(command, "2ONE") == 0) {
        int receiverID;
        char *text;

        if ((receiverID = get_receiverID(rest) < 0 || !(text = get_message_text(rest)))) {
            fprintf(stderr, "Cannot send message to one receiver.\n");
            send_STOP();
            return -1;
        }
        printf("%d", receiverID);
        return send_ONE(receiverID, text);

    } else if (strcmp(command, "STOP") == 0) {
        return send_STOP();
    }

    fprintf(stderr, "Command '%s' is not recognized.\n", command);
    return send_STOP();
}

int run_client() {
    struct msqid_ds queue_Statistics;

    pid = fork();
    if (pid < 0) {
        perror("Cannot create child process.");
        return -1;
    }

    if (pid == 0) { //dziecko zajmie się obsługą kolejki
        while (1) {
            if (msgctl(client_queue, IPC_STAT, &queue_Statistics) == -1) {
                perror("Cannot get client queue information.");
                return -1;
            }
            if (queue_Statistics.msg_qnum) { //msg.qnum  - ilosc msg w kolejce
                if (handle_client_queue() == -1) {
                    exit(1);
                } else {
                    printf("Type a command:\n");
                }
            }
        }
    } else { //rodzic zajmie się obsługą inputu
        while (1) {
            if (listen_input() == -1) {
                return -1;
            }
        }
    }
}

void exit_handler() {
    if (msgctl(client_queue, IPC_RMID, NULL) == -1) { //IPC_RMID - usuwanie kolejki
        perror("Cannot remove client queue!");
        exit(1);
    } else {
        printf("Client queue with key: '%d' was deleted.\n", client_key);
    }
}

int set_handler(int sig_no, int sa_flags, void *handler) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror("sigation()");
        return -1;
    }
    return 0;
}


int main() {
    if (atexit(exit_handler) == -1) {
        perror("Cannot set the exit handler.");
        exit(EXIT_FAILURE);
    }

    if (create_queues() == -1) {
        exit(EXIT_FAILURE);
    }

    if (set_handler(SIGINT, 0, send_STOP) == -1) {
        exit(EXIT_FAILURE);
    }
    if (send_INIT() == -1) {
        fprintf(stderr, "Cannot INIT client.\n");
        exit(EXIT_FAILURE);
    }

    run_client();
    return 0;
}

