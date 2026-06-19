/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_server.c
 * Purpose: Single-connection blocking TCP echo server.
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

void socket_mgr_server_start(int port) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[512];
    char input[512];

    log_info("SOCKET", "Server start requested on port %d", port);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("SOCKET", "Errors: socket creation failed (errno %d)", errno);
        perror("socket failed");
        return;
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_error("SOCKET", "Errors: setsockopt failed (errno %d)", errno);
        perror("setsockopt");
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        log_error("SOCKET", "Errors: bind failed (errno %d)", errno);
        perror("bind failed");
        close(server_fd);
        return;
    }

    // Listen
    if (listen(server_fd, 1) < 0) {
        log_error("SOCKET", "Errors: listen failed (errno %d)", errno);
        perror("listen");
        close(server_fd);
        return;
    }

    log_info("SOCKET", "Server started: listening on port %d", port);
    printf("Server listening on port %d...\n", port);
    fflush(stdout);

    // Accept one connection
    if ((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        log_error("SOCKET", "Errors: accept failed (errno %d)", errno);
        perror("accept");
        close(server_fd);
        return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
    log_info("SOCKET", "Connection established");
    
    printf("========================================\n");
    printf("TCP Chat Server\n");
    printf("========================================\n");
    fflush(stdout);
 
    int max_fd = (STDIN_FILENO > client_fd) ? STDIN_FILENO : client_fd;
    fd_set read_fds;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_fd, &read_fds);

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) continue;
            log_error("SOCKET", "select error: %s (errno %d)", strerror(errno), errno);
            printf("Select error.\n%s\n", strerror(errno));
            fflush(stdout);
            break;
        }

        // Check client socket
        if (FD_ISSET(client_fd, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t valread = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (valread == 0) {
                log_info("SOCKET", "Disconnect: Client closed connection");
                break;
            } else if (valread < 0) {
                log_error("SOCKET", "Socket error: %s (errno %d)", strerror(errno), errno);
                printf("Socket receive error.\n%s\n", strerror(errno));
                fflush(stdout);
                break;
            }

            buffer[valread] = '\0';
            // Trim trailing CR/LF
            buffer[strcspn(buffer, "\r\n")] = '\0';

            log_info("SOCKET", "Bytes received: %zd bytes", valread);
            log_info("SOCKET", "Client message: %s", buffer);

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            printf("Client\n%s\n", buffer);
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

            ssize_t valsent = send(client_fd, input, strlen(input), 0);
            if (valsent < 0) {
                log_error("SOCKET", "Socket error: send failed (errno %d)", errno);
                perror("send");
                break;
            }

            log_info("SOCKET", "Bytes sent: %zd bytes", valsent);
            log_info("SOCKET", "Server message: %s", input);

            printf("You\n%s\n", input);
            fflush(stdout);

            if (strcmp(input, "exit") == 0) {
                break;
            }
        }
    }
 
    printf("---\nClient disconnected.\n");
    fflush(stdout);
    close(client_fd);
    close(server_fd);
    log_info("SOCKET", "Server stopped cleanly");
}
