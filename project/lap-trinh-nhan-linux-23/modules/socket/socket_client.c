/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_client.c
 * Purpose: TCP echo/chat client.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
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
        log_error("SOCKET", "Socket error: socket creation failed (errno %d)", errno);
        perror("socket");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        log_error("SOCKET", "Socket error: invalid address / address not supported (errno %d)", errno);
        perror("inet_pton");
        close(sock_fd);
        return;
    }

    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        log_error("SOCKET", "Socket error: connection failed (errno %d)", errno);
        perror("connect");
        close(sock_fd);
        return;
    }

    log_info("SOCKET", "Connection established");

    printf("========================================\n");
    printf("TCP Chat Client\n");
    printf("========================================\n");
    fflush(stdout);

    int max_fd = (STDIN_FILENO > sock_fd) ? STDIN_FILENO : sock_fd;
    fd_set read_fds;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock_fd, &read_fds);

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) continue;
            log_error("SOCKET", "Socket error: select failed (errno %d)", errno);
            printf("Select error.\n%s\n", strerror(errno));
            fflush(stdout);
            break;
        }

        // Check server socket
        if (FD_ISSET(sock_fd, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t valread = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
            if (valread == 0) {
                log_info("SOCKET", "Disconnect");
                break;
            } else if (valread < 0) {
                log_error("SOCKET", "Socket error: recv failed (errno %d)", errno);
                printf("Socket receive error.\n%s\n", strerror(errno));
                fflush(stdout);
                break;
            }

            buffer[valread] = '\0';
            buffer[strcspn(buffer, "\r\n")] = '\0';

            log_info("SOCKET", "Bytes received: %zd bytes", valread);
            log_info("SOCKET", "Server message: %s", buffer);

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            printf("Server\n%s\n", buffer);
            fflush(stdout);
        }

        // Check stdin
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            memset(input, 0, sizeof(input));
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }

            input[strcspn(input, "\n")] = '\0';

            if (strlen(input) == 0) {
                continue;
            }

            ssize_t valsent = send(sock_fd, input, strlen(input), 0);
            if (valsent < 0) {
                log_error("SOCKET", "Socket error: send failed (errno %d)", errno);
                perror("send");
                break;
            }

            log_info("SOCKET", "Bytes sent: %zd bytes", valsent);
            log_info("SOCKET", "Client message: %s", input);

            printf("You\n%s\n", input);
            fflush(stdout);

            if (strcmp(input, "exit") == 0) {
                break;
            }
        }
    }

    printf("---\nDisconnected.\n");
    fflush(stdout);
    close(sock_fd);
    log_info("SOCKET", "Client connection closed cleanly");
}

