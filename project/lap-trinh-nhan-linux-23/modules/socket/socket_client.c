/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_client.c
 * Purpose: TCP echo client.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "socket_mgr.h"
#include "logger.h"

void socket_mgr_client_start(const char* server_ip, int port, const char* username) {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    char buffer[512];
    char input[512];

    (void)username;

    log_info("SOCKET", "Client connect requested to %s:%d", server_ip, port);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("SOCKET", "Errors: socket creation failed (errno %d)", errno);
        perror("socket");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        log_error("SOCKET", "Errors: invalid address / address not supported (errno %d)", errno);
        perror("inet_pton");
        close(sock_fd);
        return;
    }

    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        log_error("SOCKET", "Errors: connection failed (errno %d)", errno);
        perror("connect");
        close(sock_fd);
        return;
    }

    log_info("SOCKET", "Client connected: connected to server %s:%d", server_ip, port);
    printf("Connected to server %s:%d\n", server_ip, port);
    printf("Type messages below. Type 'exit' to quit.\n");
    fflush(stdout);

    while (1) {
        printf("client> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            log_info("SOCKET", "Disconnected: user exited");
            break;
        }

        if (strlen(input) == 0) {
            continue;
        }

        ssize_t valsent = send(sock_fd, input, strlen(input), 0);
        if (valsent < 0) {
            log_error("SOCKET", "Errors: send failed (errno %d)", errno);
            perror("send");
            break;
        }
        log_info("SOCKET", "Bytes sent: %zd bytes", valsent);

        memset(buffer, 0, sizeof(buffer));
        ssize_t valread = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (valread == 0) {
            log_info("SOCKET", "Disconnected: Server closed connection");
            printf("Server disconnected.\n");
            break;
        } else if (valread < 0) {
            log_error("SOCKET", "Errors: recv failed (errno %d)", errno);
            perror("recv");
            break;
        }
        log_info("SOCKET", "Bytes received: %zd bytes", valread);

        printf("server echoed: %s\n", buffer);
    }

    close(sock_fd);
    log_info("SOCKET", "Client connection closed cleanly");
}
