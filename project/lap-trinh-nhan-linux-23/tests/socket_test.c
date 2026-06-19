/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/socket_test.c
 * Purpose: Automated unit/integration tests for Socket Manager (TCP Echo - Single and Multi-client).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include "socket_mgr.h"

#define TEST_PORT 9099
#define TEST_MSG  "Hello POSIX Socket Echo!"

void run_test_client(int id) {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    char buffer[512] = {0};
    char msg1[128];
    char msg2[128];

    snprintf(msg1, sizeof(msg1), "Client %d Message 1", id);
    snprintf(msg2, sizeof(msg2), "Client %d Message 2", id);

    // Client setup
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket failed");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TEST_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("client inet_pton failed");
        close(sock_fd);
        exit(1);
    }

    // Connect to server
    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("client connect failed");
        close(sock_fd);
        exit(1);
    }

    printf("[CLIENT %d] Connected to server.\n", id);

    // Exchange 1
    if (send(sock_fd, msg1, strlen(msg1), 0) < 0) {
        perror("client send failed");
        close(sock_fd);
        exit(1);
    }
    ssize_t read_bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes < 0) {
        perror("client recv failed");
        close(sock_fd);
        exit(1);
    }
    buffer[read_bytes] = '\0';
    printf("[CLIENT %d] Echo 1: '%s'\n", id, buffer);
    assert(strcmp(buffer, msg1) == 0);

    // Short sleep to simulate some interval and verify concurrency
    usleep(100000); // 100ms

    // Exchange 2
    if (send(sock_fd, msg2, strlen(msg2), 0) < 0) {
        perror("client send failed");
        close(sock_fd);
        exit(1);
    }
    read_bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes < 0) {
        perror("client recv failed");
        close(sock_fd);
        exit(1);
    }
    buffer[read_bytes] = '\0';
    printf("[CLIENT %d] Echo 2: '%s'\n", id, buffer);
    assert(strcmp(buffer, msg2) == 0);

    close(sock_fd);
    printf("[CLIENT %d] Completed successfully and disconnected.\n", id);
    exit(0);
}

int main(void) {
    printf("Starting Socket Manager - Sprint 2 (Multi-client TCP Echo) test program...\n\n");

    // 1. Spawn Multi-client server
    pid_t server_pid = fork();
    if (server_pid < 0) {
        perror("fork server failed");
        return 1;
    }

    if (server_pid == 0) {
        // Child: Multi-client server
        socket_mgr_multi_server_start(TEST_PORT);
        exit(0); // If server_start returns
    }

    // Give server time to bind and listen
    sleep(1);

    // 2. Spawn 3 concurrent clients
    pid_t client_pids[3];
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork client failed");
            // Kill server and exit
            kill(server_pid, SIGTERM);
            return 1;
        }
        if (pid == 0) {
            // Child: Client session
            run_test_client(i + 1);
        } else {
            client_pids[i] = pid;
        }
    }

    // 3. Wait for all clients to finish
    int client_failures = 0;
    for (int i = 0; i < 3; i++) {
        int status;
        waitpid(client_pids[i], &status, 0);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) {
                printf("[TEST PARENT] Client child %d failed with code %d\n", i + 1, code);
                client_failures++;
            }
        } else {
            printf("[TEST PARENT] Client child %d exited abnormally\n", i + 1);
            client_failures++;
        }
    }

    // 4. Terminate server and reap it
    printf("\n[TEST PARENT] Terminating multi-client server...\n");
    kill(server_pid, SIGTERM);
    
    int server_status;
    waitpid(server_pid, &server_status, 0);
    printf("[TEST PARENT] Server reaped successfully.\n");

    assert(client_failures == 0);
    printf("\nSocket Manager Module multi-client tests completed successfully.\n");
    return 0;
}
