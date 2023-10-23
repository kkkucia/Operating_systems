#include "header.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int epoll_fd;


void send_message(client *current_client, message_type type, char text[MAX_LENGTH]) {
    message msg;
    msg.type = type;
    memcpy(&msg.text, text, MAX_LENGTH * sizeof(char));
    write(current_client->fd, &msg, sizeof(msg));
}

void delete_client(client *current_client) {
    printf("Deleting client: %s\n", current_client->nickname);
    current_client->state = EMPTY;
    current_client->nickname[0] = 0;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_client->fd, NULL);
    close(current_client->fd);
}

client *create_client(int client_fd) {
    pthread_mutex_lock(&mutex);
    int i;
    for (i = 0; i < MAX_CLIENT_NUM; i++) {
        if (clients[i].state == EMPTY) {
            break;
        }
    }
    if (i == MAX_CLIENT_NUM) {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    client *new_client = &clients[i];

    new_client->fd = client_fd;
    new_client->state = CREATE;
    new_client->responding = 1;
    pthread_mutex_unlock(&mutex);
    return new_client;
}

void handle_create_state(client *client) {
    int nick_size = read(client->fd, client->nickname, sizeof(client->nickname) - 1);
    int j = client - clients;

    pthread_mutex_lock(&mutex);

    int i;
    for (i = 0; i < MAX_CLIENT_NUM; i++) {
        if (i != j && strncmp(client->nickname, clients[i].nickname, sizeof(clients->nickname)) == 0) {
            break;
        }
    }

    if (i == MAX_CLIENT_NUM) {
        client->nickname[nick_size] = '\0';
        client->state = READY;
        printf("New client: %s\n", client->nickname);
    } else {
        message msg = {.type = NICK_TAKEN};
        printf("Nickname %s already taken\n", client->nickname);
        write(client->fd, &msg, sizeof(msg));
        strcpy(client->nickname, "new client");
        delete_client(client); // username taken
    }

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

void handle_ready_state(client *current_client) {
    message msg;
    read(current_client->fd, &msg, sizeof(msg));

    printf("Server handle message: %s\n", process_message_type(msg.type));

    switch (msg.type) {
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
            strcat(out, msg.text);

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
            strcat(out, msg.text);

            for (int i = 0; i < MAX_CLIENT_NUM; i++) {
                if (clients[i].state != EMPTY && strcmp(clients[i].nickname, msg.other_nickname) == 0) {
                    send_message(clients + i, GOOD_MSG, out);
                }
            }
            break;
        }
        default:
            break;
    }
}


void handle_client_message(client *client) {
    if (client->state == CREATE) {
        handle_create_state(client);
    } else {
        handle_ready_state(client);
    }
}

void create_socket(int socket, void *addr, int addr_size) {
    bind(socket, (struct sockaddr *) addr, addr_size);
    listen(socket, MAX_CLIENT_NUM);
    struct epoll_event event = {.events = EPOLLIN | EPOLLPRI};
    event_data *event_data = event.data.ptr = malloc(sizeof *event_data);
    event_data->type = SOCKET_TYPE;
    event_data->payload.socket = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
}

void *ping_clients(void *arg) {
    static message ping_msg = {.type = PING};
    while (1) {
        sleep(PING_SLEEP_TIME);
        pthread_mutex_lock(&mutex);
        printf("Pinging clients\n");
        for (int i = 0; i < MAX_CLIENT_NUM; i++) {
            if (clients[i].state != EMPTY) {
                if (clients[i].responding) {
                    clients[i].responding = 0;
                    write(clients[i].fd, &ping_msg, sizeof ping_msg);
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
    strncpy(local_addr.sun_path, socket_path, sizeof local_addr.sun_path);

    struct sockaddr_in web_addr = {
            .sin_family = AF_INET, .sin_port = htons(port),
            .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    unlink(socket_path);
    int local_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    create_socket(local_sock, &local_addr, sizeof local_addr);

    int web_sock = socket(AF_INET, SOCK_STREAM, 0);
    create_socket(web_sock, &web_addr, sizeof web_addr);

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);

    printf("Server listening on *:%d and '%s'\n", port, socket_path);
}


void handle_new_connection(event_data *data) {
    int client_fd = accept(data->payload.socket, NULL, NULL);
    client *client = create_client(client_fd);
    if (client == NULL) {
        printf("Server is full\n");
        message msg = {.type = FULL};
        write(client_fd, &msg, sizeof msg);
        close(client_fd);
        return;
    }

    event_data *event_data = malloc(sizeof(event_data));
    event_data->type = CLIENT_TYPE;
    event_data->payload.client = client;
    struct epoll_event event = {.events = EPOLLIN | EPOLLET | EPOLLHUP, .data = {event_data}};

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
}

void handle_client_event(event_data *data, uint32_t events) {
    if (events & EPOLLHUP) {
        pthread_mutex_lock(&mutex);
        delete_client(data->payload.client);
        pthread_mutex_unlock(&mutex);
    } else {
        handle_client_message(data->payload.client);
    }
}

void run_server() {
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int epoll_counter = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < epoll_counter; i++) {
            event_data *data = events[i].data.ptr;
            if (data->type == SOCKET_TYPE) {
                handle_new_connection(data);
            } else if (data->type == CLIENT_TYPE) {
                handle_client_event(data, events[i].events);
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