#ifndef ZAD1_SHARED_H
#define ZAD1_SHARED_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAX_LENGTH 2048

typedef struct message {
    long msgType;
    int senderID;
    int receiverID;
    char msgText[MAX_LENGTH];
    time_t timeSent;
} message;

typedef enum msg_type {
    STOP = 1,
    LIST = 2,
    ALL = 3,
    ONE = 4,
    INIT = 5
} msg_type;

const int MSG_SIZE = sizeof(message) - sizeof(long);

#endif