/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_server.c
 * Purpose: Single-connection TCP server with non-blocking terminal redraw.
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
#include <termios.h>
#include "socket_mgr.h"
#include "logger.h"

static int server_socket_fd = -1;
static int client_socket_fd = -1;

static struct termios orig_termios;
static char input_buffer[512] = {0};
static int input_len = 0;
static pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
static int is_interactive = 0;

static void set_conio_terminal_mode(void) {
    if (is_interactive) {
        struct termios new_termios;
        tcgetattr(0, &orig_termios);
        memcpy(&new_termios, &orig_termios, sizeof(new_termios));
        new_termios.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &new_termios);
    }
}

static void reset_terminal_mode(void) {
    if (is_interactive) {
        tcsetattr(0, TCSANOW, &orig_termios);
    }
}

static void* receiver_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[512];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t valread = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (valread == 0) {
            log_info("SOCKET", "Disconnect: Client closed connection");
            if (is_interactive) {
                pthread_mutex_lock(&input_mutex);
                printf("\r\x1b[KClient disconnected.\n");
                fflush(stdout);
                pthread_mutex_unlock(&input_mutex);
            } else {
                printf("Client disconnected.\n");
                fflush(stdout);
            }
            break;
        } else if (valread < 0) {
            log_error("SOCKET", "Socket error: %s (errno %d)", strerror(errno), errno);
            if (is_interactive) {
                pthread_mutex_lock(&input_mutex);
                printf("\r\x1b[KSocket receive error: %s\n", strerror(errno));
                fflush(stdout);
                pthread_mutex_unlock(&input_mutex);
            } else {
                printf("Socket receive error: %s\n", strerror(errno));
                fflush(stdout);
            }
            break;
        }

        buffer[valread] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        log_info("SOCKET", "Bytes received: %zd bytes", valread);
        log_info("SOCKET", "Client message: %s", buffer);

        if (strcmp(buffer, "exit") == 0) {
            if (is_interactive) {
                pthread_mutex_lock(&input_mutex);
                printf("\r\x1b[KClient sent exit command.\n");
                fflush(stdout);
                pthread_mutex_unlock(&input_mutex);
            } else {
                printf("Client sent exit command.\n");
                printf("Client disconnected.\n");
                fflush(stdout);
            }
            break;
        }

        if (is_interactive) {
            pthread_mutex_lock(&input_mutex);
            printf("\r\x1b[KClient > %s\n", buffer);
            printf("You > %s", input_buffer);
            fflush(stdout);
            pthread_mutex_unlock(&input_mutex);
        } else {
            printf("Client\n%s\n", buffer);
            fflush(stdout);
        }
    }

    log_info("SOCKET", "Terminal closed");
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    if (server_socket_fd != -1) {
        shutdown(server_socket_fd, SHUT_RDWR);
        close(server_socket_fd);
    }
    reset_terminal_mode();
    _exit(0);
    return NULL;
}

void socket_mgr_server_start(int port) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    log_info("SOCKET", "Server start requested on port %d", port);

    is_interactive = isatty(0);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error("SOCKET", "Errors: socket creation failed (errno %d)", errno);
        perror("socket failed");
        return;
    }
    server_socket_fd = server_fd;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_error("SOCKET", "Errors: setsockopt failed (errno %d)", errno);
        perror("setsockopt");
        close(server_fd);
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        log_error("SOCKET", "Errors: bind failed (errno %d)", errno);
        perror("bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0) {
        log_error("SOCKET", "Errors: listen failed (errno %d)", errno);
        perror("listen");
        close(server_fd);
        return;
    }

    log_info("SOCKET", "TCP Server launched");
    printf("Server listening on port %d...\n", port);
    fflush(stdout);

    if ((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        log_error("SOCKET", "Errors: accept failed (errno %d)", errno);
        perror("accept");
        close(server_fd);
        return;
    }
    client_socket_fd = client_fd;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
    log_info("SOCKET", "Connection established");
    log_info("SOCKET", "Socket process started");

    printf("========================================\n");
    printf("TCP Chat Server\n");
    printf("========================================\n");
    fflush(stdout);

    pthread_t recv_tid;
    int* p_client_fd = malloc(sizeof(int));
    *p_client_fd = client_fd;
    if (pthread_create(&recv_tid, NULL, receiver_thread, p_client_fd) != 0) {
        log_error("SOCKET", "Errors: pthread_create failed (errno %d)", errno);
        perror("pthread_create failed");
        free(p_client_fd);
        close(client_fd);
        close(server_fd);
        return;
    }
    pthread_detach(recv_tid);

    if (is_interactive) {
        set_conio_terminal_mode();

        pthread_mutex_lock(&input_mutex);
        printf("You > ");
        fflush(stdout);
        pthread_mutex_unlock(&input_mutex);

        while (1) {
            char c;
            if (read(0, &c, 1) <= 0) {
                break;
            }

            pthread_mutex_lock(&input_mutex);
            if (c == '\n' || c == '\r') {
                input_buffer[input_len] = '\0';
                pthread_mutex_unlock(&input_mutex);

                if (input_len > 0) {
                    ssize_t valsent = send(client_fd, input_buffer, input_len, 0);
                    if (valsent < 0) {
                        log_error("SOCKET", "Socket error: send failed (errno %d)", errno);
                        break;
                    }
                    log_info("SOCKET", "Bytes sent: %zd bytes", valsent);
                    log_info("SOCKET", "Server message: %s", input_buffer);

                    // Print server message securely
                    pthread_mutex_lock(&input_mutex);
                    printf("\r\x1b[KYou > %s\n", input_buffer);
                    fflush(stdout);
                    pthread_mutex_unlock(&input_mutex);
                }

                int check_exit = (strcmp(input_buffer, "exit") == 0);

                pthread_mutex_lock(&input_mutex);
                input_len = 0;
                input_buffer[0] = '\0';
                if (!check_exit) {
                    printf("You > ");
                    fflush(stdout);
                }
                pthread_mutex_unlock(&input_mutex);

                if (check_exit) {
                    break;
                }
            } else if (c == 127 || c == 8) {
                if (input_len > 0) {
                    input_len--;
                    input_buffer[input_len] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
                pthread_mutex_unlock(&input_mutex);
            } else if (c >= 32 && c < 127 && input_len < (int)sizeof(input_buffer) - 1) {
                input_buffer[input_len++] = c;
                input_buffer[input_len] = '\0';
                putchar(c);
                fflush(stdout);
                pthread_mutex_unlock(&input_mutex);
            } else {
                pthread_mutex_unlock(&input_mutex);
            }
        }
    } else {
        char input[512];
        while (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\r\n")] = '\0';
            if (strlen(input) == 0) continue;

            ssize_t valsent = send(client_fd, input, strlen(input), 0);
            if (valsent < 0) {
                log_error("SOCKET", "Socket error: send failed (errno %d)", errno);
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

    log_info("SOCKET", "Terminal closed");
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    reset_terminal_mode();
    log_info("SOCKET", "Server stopped cleanly");
}
