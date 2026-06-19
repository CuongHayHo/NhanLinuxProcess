/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/socket_test.c
 * Purpose: Automated unit/integration tests for Socket Manager (TCP Echo).
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
#include "socket_mgr.h"

#define TEST_PORT 9099
#define TEST_MSG  "Hello POSIX Socket Echo!"

void run_test_client(void) {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    char buffer[512] = {0};

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

    printf("[TEST CLIENT] Connected to server.\n");

    // Send message
    ssize_t sent = send(sock_fd, TEST_MSG, strlen(TEST_MSG), 0);
    if (sent < 0) {
        perror("client send failed");
        close(sock_fd);
        exit(1);
    }
    printf("[TEST CLIENT] Sent %zd bytes: '%s'\n", sent, TEST_MSG);

    // Receive echo
    ssize_t read_bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes < 0) {
        perror("client recv failed");
        close(sock_fd);
        exit(1);
    }
    buffer[read_bytes] = '\0';
    printf("[TEST CLIENT] Received %zd bytes: '%s'\n", read_bytes, buffer);

    // Assert echo matches
    assert(strcmp(buffer, TEST_MSG) == 0);
    printf("[TEST CLIENT] Echo verification passed!\n");

    close(sock_fd);
    printf("[TEST CLIENT] Disconnected cleanly.\n");
    exit(0);
}

int main(void) {
    printf("Starting Socket Manager - Sprint 1 (TCP Echo) test program...\n\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process: Client
        // Sleep a short duration to ensure server is listening
        sleep(1);
        run_test_client();
    } else {
        // Parent process: Server
        printf("[TEST SERVER] Starting server on port %d...\n", TEST_PORT);
        socket_mgr_server_start(TEST_PORT);

        // Wait for child to exit
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("\n[TEST SERVER] Client child exited with status: %d\n", exit_code);
            assert(exit_code == 0);
        } else {
            printf("\n[TEST SERVER] Client child exited abnormally!\n");
            return 1;
        }
    }

    printf("\nSocket Manager Module tests completed successfully.\n");
    return 0;
}
