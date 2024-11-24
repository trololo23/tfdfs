#include "common.h"

double function_to_integrate(double x) {
    return x * x;
}

double integrate(double start, double end, int steps) {
    double step = (end - start) / steps;
    double result = 0.0;
    
    for (int i = 0; i < steps; i++) {
        double x1 = start + i * step;
        double x2 = x1 + step;
        result += (function_to_integrate(x1) + function_to_integrate(x2)) * step / 2.0;
    }
    
    return result;
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(BROADCAST_PORT)
    };

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("Server started. Waiting for tasks...\n");

    while (1) {
        struct Message msg;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t recv_len = recvfrom(sock_fd, &msg, sizeof(msg), 0,
                                  (struct sockaddr*)&client_addr, &client_len);

        if (recv_len < 0) continue;

        if (msg.type == DISCOVERY) {
            msg.type = DISCOVERY_RESPONSE;
            sendto(sock_fd, &msg, sizeof(msg), 0,
                  (struct sockaddr*)&client_addr, client_len);
        }
        else if (msg.type == TASK) {
            double result = integrate(msg.data.task.start, 
                                   msg.data.task.end, 1000);
            
            msg.type = RESULT;
            msg.data.result.task_id = msg.data.task.task_id;
            msg.data.result.result = result;
            
            sendto(sock_fd, &msg, sizeof(msg), 0,
                  (struct sockaddr*)&client_addr, client_len);
        }
    }

    close(sock_fd);
    return 0;
} 