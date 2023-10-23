#include "header.h"

#define max_num_clients 10


int clients[max_num_clients] = {0};
int server_queue, client_queue;
key_t server_key, client_key;

void create_queue() {
    server_key = ftok(getenv("HOME"), 'S');
    if (server_key == -1) {
        perror("Cannot to generate a key.");
        exit(EXIT_FAILURE);
    }
    server_queue = msgget(server_key, IPC_CREAT | 0666); //utworzenie nowej kolejki komunikatow dla serwera
    if (server_queue == -1) {
        perror("Cannot create server queue!");
        exit(EXIT_FAILURE);
    }
    printf("Server queue with key: '%d' was created.\n", server_key);
}

int get_avaiable_clientID(void) {
    for (int i = 0; i < max_num_clients; i++) {
        if (!clients[i]) {
            return i;
        }
    }
    return -1;
}

int init_client(char *msgText) {
    int clientID = get_avaiable_clientID();
    if (clientID == -1) {
        fprintf(stderr, "The number of possible clients (%d) was exceeded. Cannot create a new client. \n",
                max_num_clients);
        return -1;
    }
    printf("New client id: '%d'.\n", clientID);
    key_t client_key = (key_t) strtol(msgText, NULL, 10); //text -> long int -> key_k

    clients[clientID] = msgget(client_key, 0);
    if (clients[clientID] == -1) {
        perror("Cannot create or find client queue.");
        return -1;
    }
    message msg = {.msgType = INIT};
    sprintf(msg.msgText, "%d", clientID);
    printf("Client queue with key '%d' was found.\n", client_key);

    if (msgsnd(clients[clientID], &msg, MSG_SIZE, 0) == -1) {
        perror("Cannot to send message to the client.");
        return -1;
    }
    return 0;
}


int list_all(int senderID) {
    printf("All active clients:\n");
    char active_clients_list[max_num_clients];
    char tmp[MAX_LENGTH];
    active_clients_list[0] = '\0';

    for (int clientID = 0; clientID < max_num_clients; clientID++) {
        if (clients[clientID] != 0) {
            printf("%d\n", clientID);

            sprintf(tmp, "%d\n", clientID);
            strcat(active_clients_list, tmp);
        }
    }
    message msg = {.msgType = LIST};
    strcpy(msg.msgText, active_clients_list);

    if (msgsnd(clients[senderID], &msg, MSG_SIZE, 0) == -1) {
        perror("Cannot to send message to the client.");
        return -1;
    }
    puts("LIST message was sent.");
    return 0;
}


int send_2one(int senderID, int receiverID, char *msgText, msg_type msgType) {
    if (!clients[receiverID]) {
        fprintf(stderr, "Cannot send a message to a client. There is no client with id '%d'.\n", receiverID);
        return -1;
    }
    message msg = {.msgType = msgType, .senderID = senderID, .receiverID = receiverID, .timeSent = time(NULL)};
    strcpy(msg.msgText, msgText);

    if (msgsnd(clients[receiverID], &msg, MSG_SIZE, 0) == -1) {
        perror("Cannot to send message to the client.");
        return -1;
    }
    printf("Message from '%d' to '%d' was sent.\n", senderID, receiverID);
    return 0;
}


int send_2all(int senderID, char *msgText, msg_type msgType) {
    for (int clientID = 0; clientID < max_num_clients; clientID++) {
        if (clients[clientID] && (clientID != senderID)) {
            if (send_2one(senderID, clientID, msgText, msgType) == -1) {
                return -1;
            }
        }
    }
    printf("Message from '%d' was sent to all active clients.\n", senderID);
    return 0;
}


int stop_client(int clientID) {
    if (!clients[clientID]) {
        fprintf(stderr, "Cannot send a message to a client. There is no client with id '%d'.\n", clientID);
        return -1;
    }
    clients[clientID] = 0;
    printf("STOP message was sent.\n");
    return 0;
}


int write_message_to_file(message msg) {
    FILE *file = fopen("messages.txt", "a");
    if (!file) {
        perror("Cannot to open a file.");
        return -1;
    }
    time_t currTime = time(NULL);
    struct tm *local_time = localtime(&currTime);
    if (!local_time) {
        perror("Cannot get a local time.\n");
        return -1;
    }
    if (fprintf(file, "Sender id: %d\nMessage type: %ld\nTime of sending %d-%02d-%02d %02d:%02d:%02d\nMessage: %s\n",
                msg.senderID, msg.msgType,
                local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
                local_time->tm_hour, local_time->tm_min, local_time->tm_sec, msg.msgText) < 0) {
        fprintf(stderr, "Cannot write data to a file.\n");
        return -1;
    }
    fclose(file);
    return 0;
}


int handle_message(message msg) {
    if (write_message_to_file(msg) == -1) {
        return -1;
    }
    switch (msg.msgType) {
        case INIT:
            printf("Message INIT from client %d.\n", msg.senderID);
            return init_client(msg.msgText);
        case LIST:
            printf("Message LIST from client %d.\n", msg.senderID);
            return list_all(msg.senderID);
        case ALL:
            printf("Message 2ALL from client %d.\n", msg.senderID);
            return send_2all(msg.senderID, msg.msgText, msg.msgType);
        case ONE:
            printf("Message 2ONE from %d client.\n", msg.senderID);
            return send_2one(msg.senderID, msg.receiverID, msg.msgText, msg.msgType);
        case STOP:
            printf("Message STOP from client %d.\n", msg.senderID);
            return stop_client(msg.senderID);
        default:
            fprintf(stderr, "Unknown message type: '%ld'.\n", msg.msgType);
            return -1;
    }
}

int get_message(int server_queue) {
    message recived_msg;

    if (msgrcv(server_queue, &recived_msg, MSG_SIZE, -(INIT + 1), 0) == -1) { //-(INIT+1) - piorytet
        perror("Cannot get message from client!");
        return -1;
    }
    return handle_message(recived_msg);
}


int run_server() {
    printf("Server start listening to clients...\n");
    while (1) {
        if (get_message(server_queue) == -1) {
            return -1;
        }
    }
    return 0;
}

void exit_handler() {
    if (msgctl(server_queue, IPC_RMID, NULL) == -1) { //usuwanie kolejki
        perror("Cannot remove server queue!");
        exit(1);
    } else {
        printf("Server queue with key: '%d' was deleted.\n", server_key);
    }
}

void SIGINT_Handler(int sig_no) {
    printf("\nReceived '%d' signal. Closing the server...\n", sig_no);
    exit(0);
}

int set_handler(int sig_no, int sa_flags, void *handler) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;
    sa.sa_handler = handler;

    if (sigaction(sig_no, &sa, NULL) == -1) {
        perror("sigaction()");
        return -1;
    }
    return 0;
}


int main() {
    if (atexit(exit_handler) == -1) {
        perror("Cannot set the exit handler.");
        exit(EXIT_FAILURE);
    }

    if (set_handler(SIGINT, 0, SIGINT_Handler) == -1) {
        exit(-1);
    }

    create_queue();
    run_server();
    return 0;
}
