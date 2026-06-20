/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/socket_test.c
 * Purpose: Automated unit/integration tests for Socket Manager (TCP Echo/Chat - Single and Multi-client).
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
#include <sys/select.h>
#include <sys/time.h>
#include "socket_mgr.h"

#define TEST_PORT 9099

int read_from_pipe_with_timeout(int fd, char *buf, size_t max_len, int timeout_sec) {
    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;

    int ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
    if (ret <= 0) {
        return -1; // Timeout or error
    }
    ssize_t bytes_read = read(fd, buf, max_len - 1);
    if (bytes_read < 0) {
        return -1;
    }
    buf[bytes_read] = '\0';
    return (int)bytes_read;
}

void run_two_way_chat_test(void) {
    int server_in[2], server_out[2];
    int client_in[2], client_out[2];

    assert(pipe(server_in) == 0);
    assert(pipe(server_out) == 0);
    assert(pipe(client_in) == 0);
    assert(pipe(client_out) == 0);

    pid_t server_pid = fork();
    if (server_pid < 0) {
        perror("fork server failed");
        exit(1);
    }

    if (server_pid == 0) {
        // Child: Server
        dup2(server_in[0], STDIN_FILENO);
        dup2(server_out[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(server_in[1]);
        close(server_out[0]);
        close(client_in[0]);
        close(client_in[1]);
        close(client_out[0]);
        close(client_out[1]);

        socket_mgr_server_start(TEST_PORT);
        exit(0);
    }

    // Give server a moment to listen
    sleep(1);

    pid_t client_pid = fork();
    if (client_pid < 0) {
        perror("fork client failed");
        kill(server_pid, SIGKILL);
        exit(1);
    }

    if (client_pid == 0) {
        // Child: Client
        dup2(client_in[0], STDIN_FILENO);
        dup2(client_out[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(client_in[1]);
        close(client_out[0]);
        close(server_in[0]);
        close(server_in[1]);
        close(server_out[0]);
        close(server_out[1]);

        socket_mgr_client_start("127.0.0.1", TEST_PORT, "user");
        exit(0);
    }

    // Close unused ends in parent
    close(server_in[0]);
    close(server_out[1]);
    close(client_in[0]);
    close(client_out[1]);

    char buf[1024];

    // Verify Server listening banner
    int n = read_from_pipe_with_timeout(server_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "Server listening on port") != NULL);

    // Give client time to connect
    sleep(1);

    // Verify Server established header
    n = read_from_pipe_with_timeout(server_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "TCP Chat Server") != NULL);

    // Verify Client established header
    n = read_from_pipe_with_timeout(client_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "TCP Chat Client") != NULL);

    // Step 1: Server sends first
    const char *msg1 = "Hello from server\n";
    write(server_in[1], msg1, strlen(msg1));

    // Verify Server stdout shows "You\nHello from server"
    n = read_from_pipe_with_timeout(server_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "You") != NULL);
    assert(strstr(buf, "Hello from server") != NULL);

    // Verify Client stdout shows "Server\nHello from server"
    n = read_from_pipe_with_timeout(client_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "Server") != NULL);
    assert(strstr(buf, "Hello from server") != NULL);

    // Step 2: Client sends message
    const char *msg2 = "Hi from client\n";
    write(client_in[1], msg2, strlen(msg2));

    // Verify Client stdout shows "You\nHi from client"
    n = read_from_pipe_with_timeout(client_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "You") != NULL);
    assert(strstr(buf, "Hi from client") != NULL);

    // Verify Server stdout shows "Client\nHi from client"
    n = read_from_pipe_with_timeout(server_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "Client") != NULL);
    assert(strstr(buf, "Hi from client") != NULL);

    // Step 3: Client sends exit
    const char *exit_msg = "exit\n";
    write(client_in[1], exit_msg, strlen(exit_msg));

    // Verify Client stdout shows "You\nexit" and "Disconnected."
    n = read_from_pipe_with_timeout(client_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "You") != NULL);
    assert(strstr(buf, "exit") != NULL);
    assert(strstr(buf, "Disconnected.") != NULL);

    // Verify Server stdout shows "Client disconnected."
    n = read_from_pipe_with_timeout(server_out[0], buf, sizeof(buf), 2);
    assert(n > 0);
    assert(strstr(buf, "Client disconnected.") != NULL);

    // Reap child processes
    int status;
    waitpid(server_pid, &status, 0);
    waitpid(client_pid, &status, 0);

    close(server_in[1]);
    close(server_out[0]);
    close(client_in[1]);
    close(client_out[0]);

    printf("Two-Way Chat (Single-Client select-based) test passed successfully!\n\n");
}

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
    // 1. Two-Way Chat (Single-Client select-based) test
    printf("Starting Socket Manager - Sprint 10 (Two-Way Interactive TCP Chat) test program...\n\n");
    run_two_way_chat_test();

    // 2. Spawn Multi-client server
    printf("Starting Socket Manager - Sprint 2 (Multi-client TCP Echo) test program...\n\n");
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

    // 3. Spawn 3 concurrent clients
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

    // 4. Wait for all clients to finish
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

    // 5. Terminate server and reap it
    printf("\n[TEST PARENT] Terminating multi-client server...\n");
    kill(server_pid, SIGTERM);
    
    int server_status;
    waitpid(server_pid, &server_status, 0);
    printf("[TEST PARENT] Server reaped successfully.\n");

    assert(client_failures == 0);
    printf("\nSocket Manager Module multi-client tests completed successfully.\n");
    return 0;
}
