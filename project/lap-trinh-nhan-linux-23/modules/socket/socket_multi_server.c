/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_multi_server.c
 * Purpose: Multi-threaded TCP echo server.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "socket_mgr.h"
#include "logger.h"

typedef struct {
    int client_fd;
    struct sockaddr_in address;
} session_t;

// Protocol level logic: handles the echo loop for one connection
static void session_handle_protocol(int client_fd) {
    char buffer[512];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t valread = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (valread == 0) {
            log_info("SOCKET", "Disconnect: Client closed connection");
            printf("[SERVER] Client disconnected.\n");
            break;
        } else if (valread < 0) {
            log_error("SOCKET", "Errors: recv failed (errno %d)", errno);
            perror("[SERVER] recv failed");
            break;
        }

        log_info("SOCKET", "Bytes received: %zd bytes", valread);

        // Echo back the message
        ssize_t valsent = send(client_fd, buffer, valread, 0);
        if (valsent < 0) {
            log_error("SOCKET", "Errors: send failed (errno %d)", errno);
            perror("[SERVER] send failed");
            break;
        }
        log_info("SOCKET", "Bytes sent: %zd bytes", valsent);
    }
}

// Session level logic: manages the connection lifecycle and thread resources
static void* session_worker(void* arg) {
    session_t* session = (session_t*)arg;
    int client_fd = session->client_fd;
    char client_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &session->address.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(session->address.sin_port);

    log_info("SOCKET", "Thread created: Handling client %s:%d", client_ip, client_port);
    printf("[SERVER] Thread created for client %s:%d\n", client_ip, client_port);
    fflush(stdout);

    // Call protocol handler
    session_handle_protocol(client_fd);

    // Clean up connection
    close(client_fd);
    free(session);

    log_info("SOCKET", "Thread exited: Finished client %s:%d", client_ip, client_port);
    printf("[SERVER] Thread exited for client %s:%d\n", client_ip, client_port);
    fflush(stdout);

    pthread_exit(NULL);
}

// Server level logic: sets up listener and handles accept loop
void socket_mgr_multi_server_start(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    log_info("SOCKET", "Multi-client Server start requested on port %d", port);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("SOCKET", "Errors: socket creation failed (errno %d)", errno);
        perror("socket failed");
        return;
    }

    // Reuse port
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
    if (listen(server_fd, 10) < 0) {
        log_error("SOCKET", "Errors: listen failed (errno %d)", errno);
        perror("listen");
        close(server_fd);
        return;
    }

    log_info("SOCKET", "Server started: Multi-threaded server listening on port %d", port);
    printf("Multi-client TCP Server listening on port %d...\n", port);
    printf("Press Ctrl+C to terminate server.\n");
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);

        if (client_fd < 0) {
            log_error("SOCKET", "Errors: accept failed (errno %d)", errno);
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        log_info("SOCKET", "Client connected: %s:%d", client_ip, ntohs(client_addr.sin_port));
        printf("[SERVER] Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        fflush(stdout);

        // Allocate session context
        session_t* session = malloc(sizeof(session_t));
        if (session == NULL) {
            log_error("SOCKET", "Errors: malloc failed");
            close(client_fd);
            continue;
        }
        session->client_fd = client_fd;
        session->address = client_addr;

        // Spawn session thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, session_worker, session) != 0) {
            log_error("SOCKET", "Errors: thread creation failed");
            close(client_fd);
            free(session);
            continue;
        }

        // Detach thread to avoid memory leaks
        pthread_detach(thread_id);
    }

    close(server_fd);
}
