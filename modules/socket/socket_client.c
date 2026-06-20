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
#include <termios.h>
#include "socket_mgr.h"
#include "logger.h"

static int client_socket_fd = -1;
static struct termios orig_termios;
static char input_buffer[512] = {0};
static int input_len = 0;
static pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
static int is_interactive = 0;
static int disconnected_printed = 0;

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
    int sock_fd = *(int*)arg;
    free(arg);
    char buffer[512];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t valread = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (valread == 0) {
            log_info("SOCKET", "Disconnect");
            pthread_mutex_lock(&input_mutex);
            if (!disconnected_printed) {
                if (is_interactive) {
                    printf("\r\x1b[KDisconnected.\n");
                } else {
                    printf("Disconnected.\n");
                }
                fflush(stdout);
                disconnected_printed = 1;
            }
            pthread_mutex_unlock(&input_mutex);
            break;
        } else if (valread < 0) {
            log_error("SOCKET", "Socket error: recv failed (errno %d)", errno);
            pthread_mutex_lock(&input_mutex);
            if (is_interactive) {
                printf("\r\x1b[KSocket receive error: %s\n", strerror(errno));
                fflush(stdout);
            } else {
                printf("\nSocket receive error.\n%s\n", strerror(errno));
                fflush(stdout);
            }
            pthread_mutex_unlock(&input_mutex);
            break;
        }

        buffer[valread] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        log_info("SOCKET", "Bytes received: %zd bytes", valread);
        log_info("SOCKET", "Server message: %s", buffer);

        if (strcmp(buffer, "exit") == 0) {
            pthread_mutex_lock(&input_mutex);
            if (!disconnected_printed) {
                if (is_interactive) {
                    printf("\r\x1b[KServer sent exit command.\n");
                } else {
                    printf("Server sent exit command.\n");
                    printf("Disconnected.\n");
                }
                fflush(stdout);
                disconnected_printed = 1;
            }
            pthread_mutex_unlock(&input_mutex);
            break;
        }

        pthread_mutex_lock(&input_mutex);
        if (is_interactive) {
            printf("\r\x1b[KServer > %s\n", buffer);
            printf("You > %s", input_buffer);
        } else {
            printf("Server\n%s\n", buffer);
        }
        fflush(stdout);
        pthread_mutex_unlock(&input_mutex);
    }

    log_info("SOCKET", "Terminal closed");
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    reset_terminal_mode();
    _exit(0);
    return NULL;
}

void socket_mgr_client_start(const char* server_ip, int port, const char* username) {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;

    (void)username;

    log_info("SOCKET", "Client connect requested to %s:%d", server_ip, port);

    is_interactive = isatty(0);

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
                    ssize_t valsent = send(sock_fd, input_buffer, input_len, 0);
                    if (valsent < 0) {
                        log_error("SOCKET", "Socket error: send failed (errno %d)", errno);
                        break;
                    }
                    log_info("SOCKET", "Bytes sent: %zd bytes", valsent);
                    log_info("SOCKET", "Client message: %s", input_buffer);

                    // Print client message securely
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
                    pthread_mutex_lock(&input_mutex);
                    if (!disconnected_printed) {
                        printf("\r\x1b[KDisconnected.\n");
                        fflush(stdout);
                        disconnected_printed = 1;
                    }
                    pthread_mutex_unlock(&input_mutex);
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
                pthread_mutex_lock(&input_mutex);
                if (!disconnected_printed) {
                    printf("Disconnected.\n");
                    fflush(stdout);
                    disconnected_printed = 1;
                }
                pthread_mutex_unlock(&input_mutex);
                break;
            }
        }
    }

    log_info("SOCKET", "Terminal closed");
    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    reset_terminal_mode();
    log_info("SOCKET", "Client connection closed cleanly");
}
