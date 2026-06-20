/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/socket_launcher_test.c
 * Purpose: Verification tests for socket architecture modernization.
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
#include "terminal_launcher.h"
#include "repl.h"
#include "logger.h"

#define TEST_PORT 9091

// Access static functions or flags
extern const char* detect_terminal(void);

void test_terminal_detection(void) {
    printf("Testing terminal detection...\n");
    const char* term = detect_terminal();
    if (term) {
        printf("  Detected terminal: %s\n", term);
    } else {
        printf("  No terminal detected (could be in a headless environment).\n");
    }
}

void test_terminal_launching(void) {
    printf("Testing terminal launching api...\n");
    const char* term = detect_terminal();
    if (!term) {
        printf("  Skipping launcher test (no terminal emulator available).\n");
        return;
    }
    
    // Test terminal_open with a simple command that exits immediately
    int ret = terminal_open("Test Window", "true");
    printf("  terminal_open returned: %d\n", ret);
    assert(ret == 0);
}

void test_cli_dispatch_and_graceful_shutdown(void) {
    printf("Testing CLI dispatch, Host launch, Client launch, and graceful shutdown...\n");

    // Start Host server in a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child: execute server inline
        is_interactive = 0; // Run socket code inline in direct mode
        
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "socket server %d", TEST_PORT);
        
        printf("[Child Server] Starting server via CLI dispatch: %s\n", cmd);
        repl_execute_command(cmd);
        
        printf("[Child Server] Server exited cleanly.\n");
        exit(0);
    }

    // Parent: test connection
    sleep(1); // Wait for child to initialize and listen
    
    printf("[Parent Client] Connecting to port %d...\n", TEST_PORT);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(client_fd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TEST_PORT);
    assert(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) > 0);

    int conn_attempts = 0;
    while (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        if (++conn_attempts > 5) {
            perror("[Parent Client] Connection failed");
            kill(pid, SIGKILL);
            exit(1);
        }
        sleep(1);
    }
    printf("[Parent Client] Connected successfully!\n");

    // Send exit command to trigger graceful shutdown
    printf("[Parent Client] Sending 'exit' to server...\n");
    const char* exit_msg = "exit";
    assert(send(client_fd, exit_msg, strlen(exit_msg), 0) > 0);

    close(client_fd);

    // Wait for child process to terminate
    int status;
    printf("[Parent Client] Waiting for server process (PID %d) to exit...\n", pid);
    pid_t wpid = waitpid(pid, &status, 0);
    assert(wpid == pid);
    
    if (WIFEXITED(status)) {
        printf("[Parent Client] Server exited with status %d.\n", WEXITSTATUS(status));
        assert(WEXITSTATUS(status) == 0);
    } else {
        printf("[Parent Client] Server terminated abnormally.\n");
        assert(0);
    }
}

int main(void) {
    printf("=== Starting Socket Launcher Tests ===\n");
    
    test_terminal_detection();
    test_terminal_launching();
    test_cli_dispatch_and_graceful_shutdown();
    
    printf("=== All Socket Launcher Tests Passed! ===\n");
    return 0;
}
