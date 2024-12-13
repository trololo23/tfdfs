#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define BROADCAST_PORT 8888
#define SERVER_PORT 8889
#define BUFFER_SIZE 1024
#define MAX_SERVERS 10

// Типы сообщений
enum MessageType {
    DISCOVERY = 1,
    DISCOVERY_RESPONSE = 2,
    TASK = 3,
    RESULT = 4
};

// Структура задачи
struct Task {
    double start;
    double end;
};

// Структура результата
struct Result {
    int task_id;
    double result;
};

// Структура сообщения
struct Message {
    enum MessageType type;
    union {
        struct Task task;
        struct Result result;
    } data;
};

#endif 