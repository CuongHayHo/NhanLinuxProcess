/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_mgr.c
 * Purpose: Submenu TUI implementation for the Socket Manager.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket_mgr.h"
#include "logger.h"
#include "config.h"
#include <unistd.h>
#include "terminal_launcher.h"

static int read_socket_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    input_buf[strcspn(input_buf, "\n")] = '\0';
    if (strlen(input_buf) == 0) {
        return -1;
    }

    val = strtol(input_buf, &endptr, 10);
    if (*endptr != '\0') {
        return -1;
    }
    return (int)val;
}

static void socket_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void socket_mgr_run(void) {
    int choice;
    log_info("SOCKET", "Socket Manager submenu invoked.");

    while (1) {
        printf("\n========================================\n");
        printf("             Socket Manager\n");
        printf("========================================\n");
        printf("1. Run TCP Server\n");
        printf("2. Run TCP Client\n");
        printf("3. Multi Client Echo\n");
        printf("4. Socket Chat\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_socket_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 4) {
            printf("\nInvalid input. Please choose a number between 0 and 4.\n");
            socket_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("SOCKET", "Socket Manager submenu closed.");
            break;
        }

        const char* sysmgr_cmd = (access("./sysmgr", F_OK) == 0) ? "./sysmgr" : "sysmgr";

        if (choice == 1) {
            char port_buf[128];
            int port = DEFAULT_PORT;
            printf("Enter port to bind [%d]: ", DEFAULT_PORT);
            fflush(stdout);
            if (fgets(port_buf, sizeof(port_buf), stdin) != NULL) {
                port_buf[strcspn(port_buf, "\n")] = '\0';
                if (strlen(port_buf) > 0) {
                    port = atoi(port_buf);
                }
            }
            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket server %d", sysmgr_cmd, port);
            if (terminal_open("TCP Server", cmd_buf) != 0) {
                printf("\nFailed to launch TCP Server terminal.\n");
            }
        } else if (choice == 2) {
            char ip_buf[128];
            char port_buf[128];
            char server_ip[128];
            int port = DEFAULT_PORT;

            strcpy(server_ip, DEFAULT_IP);

            printf("Enter Server IP [%s]: ", DEFAULT_IP);
            fflush(stdout);
            if (fgets(ip_buf, sizeof(ip_buf), stdin) != NULL) {
                ip_buf[strcspn(ip_buf, "\n")] = '\0';
                if (strlen(ip_buf) > 0) {
                    strncpy(server_ip, ip_buf, sizeof(server_ip) - 1);
                    server_ip[sizeof(server_ip) - 1] = '\0';
                }
            }

            printf("Enter Server Port [%d]: ", DEFAULT_PORT);
            fflush(stdout);
            if (fgets(port_buf, sizeof(port_buf), stdin) != NULL) {
                port_buf[strcspn(port_buf, "\n")] = '\0';
                if (strlen(port_buf) > 0) {
                    port = atoi(port_buf);
                }
            }

            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket client %s %d", sysmgr_cmd, server_ip, port);
            if (terminal_open("TCP Client", cmd_buf) != 0) {
                printf("\nFailed to launch TCP Client terminal.\n");
            }
        } else if (choice == 3) {
            char port_buf[128];
            int port = DEFAULT_PORT;
            printf("Enter port to bind [%d]: ", DEFAULT_PORT);
            fflush(stdout);
            if (fgets(port_buf, sizeof(port_buf), stdin) != NULL) {
                port_buf[strcspn(port_buf, "\n")] = '\0';
                if (strlen(port_buf) > 0) {
                    port = atoi(port_buf);
                }
            }
            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket multi %d", sysmgr_cmd, port);
            if (terminal_open("Multi Client Server", cmd_buf) != 0) {
                printf("\nFailed to launch Multi Client Server terminal.\n");
            }
        } else if (choice == 4) {
            int chat_choice;
            while (1) {
                printf("\n========================================\n");
                printf("              Socket Chat\n");
                printf("========================================\n");
                printf("1. Host Chat\n");
                printf("2. Join Chat\n");
                printf("3. Open Host + Client Demo\n");
                printf("0. Return\n");
                printf("========================================\n");
                printf("Select option: ");
                fflush(stdout);

                chat_choice = read_socket_choice();
                if (chat_choice < 0) {
                    continue;
                }
                if (chat_choice > 3) {
                    printf("\nInvalid input. Please choose a number between 0 and 3.\n");
                    continue;
                }
                if (chat_choice == 0) {
                    break;
                }

                if (chat_choice == 1) {
                    char port_buf[128];
                    int port = DEFAULT_PORT;
                    printf("Enter port to bind [%d]: ", DEFAULT_PORT);
                    fflush(stdout);
                    if (fgets(port_buf, sizeof(port_buf), stdin) != NULL) {
                        port_buf[strcspn(port_buf, "\n")] = '\0';
                        if (strlen(port_buf) > 0) {
                            port = atoi(port_buf);
                        }
                    }
                    char cmd_buf[256];
                    snprintf(cmd_buf, sizeof(cmd_buf), "%s socket chat --host %d", sysmgr_cmd, port);
                    if (terminal_open("Chat Host", cmd_buf) != 0) {
                        printf("\nFailed to launch Chat Host terminal.\n");
                    }
                    break;
                } else if (chat_choice == 2) {
                    char ip_buf[128];
                    char port_buf[128];
                    char server_ip[128];
                    int port = DEFAULT_PORT;

                    strcpy(server_ip, DEFAULT_IP);

                    printf("Enter Server IP [%s]: ", DEFAULT_IP);
                    fflush(stdout);
                    if (fgets(ip_buf, sizeof(ip_buf), stdin) != NULL) {
                        ip_buf[strcspn(ip_buf, "\n")] = '\0';
                        if (strlen(ip_buf) > 0) {
                            strncpy(server_ip, ip_buf, sizeof(server_ip) - 1);
                            server_ip[sizeof(server_ip) - 1] = '\0';
                        }
                    }

                    printf("Enter Server Port [%d]: ", DEFAULT_PORT);
                    fflush(stdout);
                    if (fgets(port_buf, sizeof(port_buf), stdin) != NULL) {
                        port_buf[strcspn(port_buf, "\n")] = '\0';
                        if (strlen(port_buf) > 0) {
                            port = atoi(port_buf);
                        }
                    }

                    char cmd_buf[256];
                    snprintf(cmd_buf, sizeof(cmd_buf), "%s socket chat --client %s %d", sysmgr_cmd, server_ip, port);
                    if (terminal_open("Chat Client", cmd_buf) != 0) {
                        printf("\nFailed to launch Chat Client terminal.\n");
                    }
                    break;
                } else if (chat_choice == 3) {
                    log_info("SOCKET", "Demo launched");
                    printf("Launching Host Chat in a new terminal...\n");
                    char cmd_host[256], cmd_client[256];
                    snprintf(cmd_host, sizeof(cmd_host), "%s socket chat --host 8080", sysmgr_cmd);
                    snprintf(cmd_client, sizeof(cmd_client), "%s socket chat --client 127.0.0.1 8080", sysmgr_cmd);
                    if (terminal_open("Chat Host", cmd_host) == 0) {
                        printf("Waiting for Host Chat to initialize...\n");
                        sleep(2);
                        printf("Launching Client Chat in a new terminal...\n");
                        if (terminal_open("Chat Client", cmd_client) != 0) {
                            printf("Failed to launch demo Client Chat.\n");
                        }
                    } else {
                        printf("Failed to launch demo Host Chat.\n");
                    }
                    break;
                }
            }
        }
    }
}
