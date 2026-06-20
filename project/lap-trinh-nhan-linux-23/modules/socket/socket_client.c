/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_client.c
 * Purpose: TCP client with threaded receiver.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "socket_mgr.h"
#include "logger.h"

static int client_socket_fd = -1;

static void* receiver_thread(void* arg) {
    int sock_fd = *(int*)arg;
    free(arg);
    char buffer[512];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t valread = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (valread == 0) {
            log_info("SOCKET", "Disconnect");
            printf("\n---\nDisconnected.\n");
            fflush(stdout);
            break;
        } else if (valread < 0) {
            log_error("SOCKET", "Socket error: recv failed (errno %d)", errno);
            printf("\nSocket receive error.\n%s\n", strerror(errno));
            fflush(stdout);
            break;
        }

        buffer[valread] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        log_info("SOCKET", "Bytes received: %zd bytes", valread);
        log_info("SOCKET", "Server message: %s", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("\nServer sent exit command.\n");
            fflush(stdout);
            break;
        }

        printf("Server\n%s\n", buffer);
        fflush(stdout);
    }

    log_info("SOCKET", "Terminal closed");
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    exit(0);
    return NULL;
}

void socket_mgr_client_start(const char* server_ip, int port, const char* username) {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    char input[512];

    (void)username;

    log_info("SOCKET", "Client connect requested to %s:%d", server_ip, port);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("SOCKET", "Socket error: socket creation failed (errno %d)", errno);
        perror("socket");
        return;
    }
    client_socket_fd = sock_fd;

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

    log_info("SOCKET", "TCP Client launched");
    log_info("SOCKET", "Connection established");
    log_info("SOCKET", "Socket process started");

    printf("========================================\n");
    printf("TCP Chat Client\n");
    printf("========================================\n");
    fflush(stdout);

    pthread_t recv_tid;
    int* p_sock_fd = malloc(sizeof(int));
    *p_sock_fd = sock_fd;
    if (pthread_create(&recv_tid, NULL, receiver_thread, p_sock_fd) != 0) {
        log_error("SOCKET", "Errors: pthread_create failed (errno %d)", errno);
        perror("pthread_create failed");
        free(p_sock_fd);
        close(sock_fd);
        return;
    }
    pthread_detach(recv_tid);

    while (1) {
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

    log_info("SOCKET", "Terminal closed");
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    log_info("SOCKET", "Client connection closed cleanly");
}
