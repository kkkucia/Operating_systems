#include "header.h"

int sock;

int create_unix_connection(char *path, char *user) {
    struct sockaddr_un addr, bind_addr;
    memset(&addr, 0, sizeof(addr));
    bind_addr.sun_family = AF_UNIX;
    addr.sun_family = AF_UNIX;
    snprintf(bind_addr.sun_path, sizeof bind_addr.sun_path, "/tmp/%s%ld", user, time(NULL));
    strncpy(addr.sun_path, path, sizeof addr.sun_path);
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    bind(sock, (void *) &bind_addr, sizeof addr);
    connect(sock, (struct sockaddr *) &addr, sizeof addr);

    return sock;
}

int create_web_connection(char *ipv4, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipv4, &addr.sin_addr) <= 0) {
        puts("Invalid address\n");
        exit(0);
    }
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr *) &addr, sizeof addr);
    connect(sock, (struct sockaddr *) &addr, sizeof addr);

    return sock;
}

void create_connection(char **argv) {
    char *nick = argv[1];
    if (strcmp(argv[2], "unix") == 0) {
        sock = create_unix_connection(argv[3], nick);
    } else if (strcmp(argv[2], "web") == 0) {
        int port = atoi(argv[4]);
        sock = create_web_connection(argv[3], port);
    } else {
        puts("INVALID ARGUMENTS: [nick] [web|unix] [ip port|path]\n");
        exit(0);
    }

    message msg = {.type = CONNECT};
    strncpy(msg.nickname, nick, sizeof(msg.nickname));
    send(sock, &msg, sizeof(msg), 0);
}

int create_epoll() {
    int epoll_fd = epoll_create1(0);

    struct epoll_event stdin_event = {
            .events = EPOLLIN |
                      EPOLLPRI, //EPOLLIN (dane do odczytu dostępne) oraz EPOLLPRI (priorytetowe dane do odczytu dostępne)
            .data = {.fd = STDIN_FILENO}
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event);

    struct epoll_event socket_event = {
            .events = EPOLLIN | EPOLLPRI | EPOLLHUP,// EPOLLHUP - zakończenie połączenia na deskryptorze)
            .data = {.fd = sock}
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &socket_event);

    return epoll_fd;
}

void handle_message(struct epoll_event event) {
    message msg;
    recvfrom(sock, &msg, sizeof msg, 0, NULL, NULL);

    if (msg.type == NICK_TAKEN) {
        puts("This username is already taken\n");
        close(sock);
        exit(EXIT_SUCCESS);
    } else if (msg.type == FULL) {
        puts("Server is full\n");
        close(sock);
        exit(EXIT_SUCCESS);
    } else if (event.events & EPOLLHUP) {
        puts("Disconnected\n");
        kill(getpid(), SIGKILL);
        exit(0);
    } else if (msg.type == PING) {
        sendto(sock, &msg, sizeof msg, 0, NULL, sizeof(struct sockaddr_in));
    } else if (msg.type == STOP) {
        puts("Stopping\n");
        close(sock);
        exit(EXIT_SUCCESS);
    } else if (msg.type == GOOD_MSG) {
        puts(msg.text);
    }
}

void handle_input() {
    char buffer[MAX_BUFF_LENGTH] = {};
    int bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));
    buffer[bytesRead] = '\0';

    char *token;
    char *delimiters = " \t\n";
    int idx = 0;

    token = strtok(buffer, delimiters);

    message_type type = -1;
    char other_nickname[MAX_LENGTH] = {};
    char text[MAX_LENGTH] = {};

    int broke = 0;

    if (token == NULL)
        return;

    while (token != NULL) {
        switch (idx) {
            case 0:
                if (strcmp(token, "LIST") == 0) {
                    puts("Message LIST was sent from client\n");
                    type = LIST;
                } else if (strcmp(token, "2ALL") == 0) {
                    puts("Message 2ALL was sent from client\n");
                    type = ALL;
                } else if (strcmp(token, "2ONE") == 0) {
                    puts("Message ONE was sent from client\n");
                    type = ONE;
                } else if (strcmp(token, "STOP") == 0) {
                    puts("Message STOP was sent from client\n");
                    type = STOP;
                } else {
                    broke = 1;
                    puts("Invalid command. Type a command [LIST/2ALL/2ONE/STOP]:");
                    type = -1;
                }
                break;
            case 1:
                strncpy(text, token, sizeof(text));
                text[sizeof(text) - 1] = '\0';
                break;
            case 2:
                strncpy(other_nickname, token, sizeof(other_nickname));
                other_nickname[strlen(token)] = '\0';
                break;
            case 3:
                broke = 1;
                break;
        }

        if (broke)
            break;

        idx++;
        token = strtok(NULL, delimiters);
    }

    if (broke) {
        return;
    }


    message msg;
    msg.type = type;
    strncpy(msg.other_nickname, other_nickname, sizeof(msg.other_nickname) + 1);
    strncpy(msg.text, text, sizeof(msg.text));

    sendto(sock, &msg, sizeof msg, 0, NULL, sizeof(struct sockaddr_in));
}


void run() {
    int epoll_fd = create_epoll();
    struct epoll_event events[2];

    while (1) {
        int epoll_counter = epoll_wait(epoll_fd, events, 2, 1);
        for (int i = 0; i < epoll_counter; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                handle_input();
            } else {
                handle_message(events[i]);
            }
        }
    }
}


void on_SIGINT(int sig_no) {
    message msg = {.type = DISCONNECT};
    sendto(sock, &msg, sizeof msg, 0, NULL, sizeof(struct sockaddr_in));
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        puts("INVALID ARGUMENTS: [nick] [web|unix] [ip port|path]\n");
        exit(0);
    }

    create_connection(argv);
    signal(SIGINT, on_SIGINT);
    run();

    return 0;
}