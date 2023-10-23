#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_LENGTH 256
#define MAX_BUFF_LENGTH 512
#define MAX_NICKNAME_LENGTH 32
#define MAX_CLIENT_NUM 16
#define PING_SLEEP_TIME 15
#define MAX_EVENTS 10

typedef enum {
    WAIT,
    LIST,
    ONE,
    ALL,
    STOP,
    PING,
    FULL,
    DISCONNECT,
    CONNECT,
    NICK_TAKEN,
    GOOD_MSG
} message_type;

typedef struct {
    message_type type;
    char text[MAX_LENGTH];
    char other_nickname[MAX_LENGTH];
    char nickname[MAX_LENGTH];
} message;

typedef enum {
    EMPTY = 0,
    CREATE,
    READY
} client_state;

typedef enum {
    SOCKET_TYPE,
    CLIENT_TYPE
} event_type;

union addr {
    struct sockaddr_un uni;
    struct sockaddr_in web;
};
typedef struct sockaddr *sa;


struct client {
    union addr addr;
    int sock, addrlen;
    client_state state;
    char nickname[MAX_NICKNAME_LENGTH];
    struct client* peer;
    char symbol;
    int responding;
} clients[MAX_CLIENT_NUM], *waiting_client = NULL;
typedef struct client client;


typedef struct event_data {
    event_type type;
    union payload {
        client *client;
        int socket;
    } payload;
} event_data;