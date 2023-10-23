#include "header.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int epoll_fd;

void send_message(client *current_client, message_type type, char text[MAX_LENGTH]) {
    message msg;
    msg.type = type;
    memcpy(&msg.text, text, MAX_LENGTH * sizeof(char));
    sendto(current_client->sock, &msg, sizeof msg, 0, (sa) &current_client->addr, current_client->addrlen);
}

void delete_client(client *current_client) {
    printf("Deleting client %s\n", current_client->nickname);

    message msg = {.type = DISCONNECT};
    sendto(current_client->sock, &msg, sizeof msg, 0, (sa) &current_client->addr, current_client->addrlen);
    memset(&current_client->addr, 0, sizeof current_client->addr);
    current_client->sock = 0;
    current_client->state = EMPTY;
    current_client->nickname[0] = 0;
}

void create_client(union addr *addr, socklen_t addrlen, int sock, char *nickname) {
    pthread_mutex_lock(&mutex);
    int empty_idx = -1;

    for (int i = 0; i < MAX_CLIENT_NUM; i++) {
        if (clients[i].state == EMPTY) {
            empty_idx = i;
        } else if (strncmp(nickname, clients[i].nickname, sizeof clients->nickname) == 0) {
            pthread_mutex_unlock(&mutex);
            message msg = {.type = NICK_TAKEN};
            printf("Nickname %s is taken\n", nickname);
            sendto(sock, &msg, sizeof msg, 0, (sa) addr, addrlen);
            return;
        }
    }
    if (empty_idx == -1) {
        pthread_mutex_unlock(&mutex);
        printf("Server is full\n");
        message msg = {.type = FULL};
        sendto(sock, &msg, sizeof msg, 0, (sa) addr, addrlen);
        return;
    }
    printf("New client %s\n", nickname);
    client *new_client = &clients[empty_idx];
    memcpy(&new_client->addr, addr, addrlen);
    new_client->addrlen = addrlen;
    new_client->state = CREATE;
    new_client->responding = 1;
    new_client->sock = sock;

    memset(new_client->nickname, 0, sizeof new_client->nickname);
    strncpy(new_client->nickname, nickname, sizeof(new_client->nickname) - 1);

    pthread_mutex_unlock(&mutex);
}


char *process_message_type(message_type type) {
    switch (type) {
        case WAIT:
            return "WAIT";
        case LIST:
            return "LIST";
        case ONE:
            return "ONE";
        case ALL:
            return "ALL";
        case STOP:
            return "STOP";
        case PING:
            return "PING";
        case FULL:
            return "FULL";
        case DISCONNECT:
            return "DISCONNECT";
        case NICK_TAKEN:
            return "NICK_TAKEN";
        case GOOD_MSG:
            return "GOOD_MSG";
        default:
            return "UNKNOWN";
    }
}


void handle_client_message(client *current_client, message *msg) {

    printf("Server handle message: %s\n", process_message_type(msg->type));

    switch (msg->type) {
        case PING: {
            pthread_mutex_lock(&mutex);
            printf("Ping: %s\n", current_client->nickname);
            current_client->responding = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
        case DISCONNECT:
        case STOP: {
            pthread_mutex_lock(&mutex);
            delete_client(current_client);
            pthread_mutex_unlock(&mutex);
            break;
        }
        case ALL: {
            char out[MAX_LENGTH] = "";
            strcat(out, current_client->nickname);
            strcat(out, ": ");
            strcat(out, msg->text);

            for (int i = 0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].state != EMPTY)
                    send_message(clients + i, GOOD_MSG, out);
            }
            break;
        }
        case LIST: {
            for (int i = 0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].state != EMPTY)
                    send_message(current_client, GOOD_MSG, clients[i].nickname);
            }
            break;
        }
        case ONE: {
            char out[MAX_LENGTH] = "";
            strcat(out, current_client->nickname);
            strcat(out, ": ");
            strcat(out, msg->text);

            for (int i = 0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].state != EMPTY && strcmp(clients[i].nickname, msg->other_nickname) == 0) {
                    send_message(clients + i, GOOD_MSG, out);
                }
            }
            break;
        }
        default:
            break;
    }
}


void create_socket(int socket, void *addr, int addr_size) {
    bind(socket, (struct sockaddr *) addr, addr_size);
    struct epoll_event event = {
            .events = EPOLLIN | EPOLLPRI,
            .data = {.fd = socket}
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
}

void *ping_clients(void *arg) {
    const static message msg = {.type = PING};
    while (1) {
        sleep(PING_SLEEP_TIME);
        pthread_mutex_lock(&mutex);
        printf("Pinging clients\n");
        for (int i = 0; i < MAX_CLIENT_NUM; i++) {
            if (clients[i].state != EMPTY) {
                if (clients[i].responding) {
                    clients[i].responding = 0;
                    sendto(clients[i].sock, &msg, sizeof msg, 0, (sa) &clients[i].addr, clients[i].addrlen);
                } else {
                    delete_client(&clients[i]);
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
}

int create_epoll() {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll");
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

void setup_socket(int port, char *socket_path) {
    epoll_fd = create_epoll();

    struct sockaddr_un local_addr = {.sun_family = AF_UNIX};
    strncpy(local_addr.sun_path, socket_path, sizeof(local_addr.sun_path));

    struct sockaddr_in web_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    unlink(socket_path);
    int local_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    create_socket(local_sock, &local_addr, sizeof local_addr);

    int web_sock = socket(AF_INET, SOCK_DGRAM, 0);
    create_socket(web_sock, &web_addr, sizeof web_addr);

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);

    printf("Server listening on *:%d and '%s'\n", port, socket_path);
}


void run_server() {
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int epoll_counter = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < epoll_counter; i++) {
            int sock = events[i].data.fd;
            message msg;
            union addr addr;
            socklen_t addrlen = sizeof addr;
            recvfrom(sock, &msg, sizeof msg, 0, (sa) &addr, &addrlen);
            if (msg.type == CONNECT) {
                create_client(&addr, addrlen, sock, msg.nickname);
            } else {
                int i;
                for (i = 0; i < MAX_CLIENT_NUM; i++) {
                    if (memcmp(&clients[i].addr, &addr, addrlen) == 0) {
                        handle_client_message(&clients[i], &msg);
                        break;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("INVALID ARGUMENTS: ./server [port] [path]\n");
        exit(EXIT_SUCCESS);
    }

    setup_socket(atoi(argv[1]), argv[2]);
    run_server();
    return 0;
}