#include "common.h"
#include <sys/time.h>
#include <time.h>

struct ServerInfo {
    struct sockaddr_in addr;
    int active;
    struct Task current_task;
    time_t last_response;
};

struct ServerInfo servers[MAX_SERVERS];
int server_count = 0;

void discover_servers(int sock_fd) {
    struct sockaddr_in broadcast_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(BROADCAST_PORT),
        .sin_addr.s_addr = INADDR_BROADCAST
    };

    int broadcast_enable = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, 
               &broadcast_enable, sizeof(broadcast_enable));

    struct Message msg = { .type = DISCOVERY };
    sendto(sock_fd, &msg, sizeof(msg), 0,
           (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));

    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_fd, &readfds);

    while (select(sock_fd + 1, &readfds, NULL, NULL, &tv) > 0) {
        struct sockaddr_in server_addr;
        socklen_t server_len = sizeof(server_addr);
        
        if (recvfrom(sock_fd, &msg, sizeof(msg), 0,
                    (struct sockaddr*)&server_addr, &server_len) > 0) {
            if (msg.type == DISCOVERY_RESPONSE && server_count < MAX_SERVERS) {
                servers[server_count].addr = server_addr;
                servers[server_count].active = 1;
                server_count++;
                printf("Found server at %s:%d\n",
                       inet_ntoa(server_addr.sin_addr),
                       ntohs(server_addr.sin_port));
            }
        }
        
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
    }
}

void distribute_tasks(int sock_fd, double start, double end) {
    if (server_count == 0) {
        printf("No servers available\n");
        return;
    }

    double segment_size = (end - start) / server_count;
    struct Message msg = { .type = TASK };
    
    for (int i = 0; i < server_count; i++) {
        if (servers[i].active) {
            msg.data.task.start = start + i * segment_size;
            msg.data.task.end = msg.data.task.start + segment_size;
            msg.data.task.task_id = i;
            
            servers[i].current_task = msg.data.task;
            servers[i].last_response = time(NULL);

            sendto(sock_fd, &msg, sizeof(msg), 0,
                   (struct sockaddr*)&servers[i].addr, sizeof(servers[i].addr));
        }
    }
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    discover_servers(sock_fd);
    if (server_count == 0) {
        printf("No servers found\n");
        return 1;
    }

    double start = 0.0, end = 10.0;
    double total_result = 0.0;
    int completed_tasks = 0;

    distribute_tasks(sock_fd, start, end);

    while (completed_tasks < server_count) {
        struct Message msg;
        struct sockaddr_in server_addr;
        socklen_t server_len = sizeof(server_addr);

        struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);

        int select_result = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (select_result > 0) {
            if (recvfrom(sock_fd, &msg, sizeof(msg), 0,
                        (struct sockaddr*)&server_addr, &server_len) > 0) {
                if (msg.type == RESULT) {
                    int task_id = msg.data.result.task_id;
                    
                    if (servers[task_id].active) {
                        total_result += msg.data.result.result;
                        completed_tasks++;
                        servers[task_id].active = 0;
                    }
                }
            }
        } else if (select_result == 0) {
            for (int i = 0; i < server_count; i++) {
                if (servers[i].active && 
                    time(NULL) - servers[i].last_response > 5) {
                    printf("Server %d timeout\n", i);
                    servers[i].active = 0;
                    distribute_tasks(sock_fd, start, end);
                }
            }
        }
    }

    printf("Total result: %f\n", total_result);
    close(sock_fd);
    return 0;
}