/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/repl.c
 * Purpose: Modern interactive CLI REPL and command dispatcher.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "repl.h"
#include "parser.h"
#include "palette.h"
#include "history.h"
#include "autocomplete.h"
#include "ui.h"
#include "linenoise.h"

/* Backend module headers */
#include "logger.h"
#include "file_mgr.h"
#include "process_mgr.h"
#include "network_mgr.h"
#include "package_mgr.h"
#include "socket_mgr.h"
#include "kernel_mgr.h"
#include "shell_mgr.h"
#include "terminal_launcher.h"

/* Process demo externs (from modules/process/demo/process_demo.h) */
extern void fork_demo_run(void);
extern void exec_demo_run(void);
extern void wait_demo_run(void);
extern void zombie_demo_run(void);
extern void orphan_demo_run(void);
extern void daemon_demo_run(void);

#define MAX_CONTEXT_DEPTH 8
static char context_stack[MAX_CONTEXT_DEPTH][64] = { "main" };
static int context_depth = 0;

const char* repl_get_current_context(void) {
    return context_stack[context_depth];
}

static void context_push(const char* context) {
    if (context_depth < MAX_CONTEXT_DEPTH - 1) {
        context_depth++;
        strncpy(context_stack[context_depth], context, 63);
        context_stack[context_depth][63] = '\0';
    }
}

static void context_pop(void) {
    if (context_depth > 0) {
        context_depth--;
    }
}

/* Helper to execute a standalone system command in child process */
static int execute_system_command(char* const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        ui_print_error("Error: fork failed to execute system command.\n");
        return -1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        exit(127);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}

/* Helper to prompt for missing arguments if not supplied inline */
static int get_arg_or_prompt(char* dest, size_t dest_len, int arg_idx, int argc, char** argv, const char* prompt_msg) {
    if (arg_idx < argc) {
        strncpy(dest, argv[arg_idx], dest_len - 1);
        dest[dest_len - 1] = '\0';
        return 0;
    }
    if (is_interactive) {
        char* input = linenoise(prompt_msg);
        if (input == NULL) {
            dest[0] = '\0';
            return -1;
        }
        if (strcmp(input, "\x1b") == 0) {
            dest[0] = '\0';
            linenoiseFree(input);
            return -2; /* ESC canceled */
        }
        strncpy(dest, input, dest_len - 1);
        dest[dest_len - 1] = '\0';
        linenoiseFree(input);
        if (dest[0] == '\0') {
            return -1;
        }
        return 0;
    } else {
        printf("%s", prompt_msg);
        fflush(stdout);
        if (fgets(dest, dest_len, stdin) == NULL) {
            dest[0] = '\0';
            return -1;
        }
        dest[strcspn(dest, "\n")] = '\0';
        if (dest[0] == '\0') {
            return -1;
        }
        return 0;
    }
}

/* Dispatches commands under 'file' context */
static void dispatch_file(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    char path[256] = "";
    char dest[256] = "";
    char mode[64] = "";
    char data[1024] = "";

    if (strcmp(cmd, "create") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path to create: ") == 0) {
            file_mgr_create(path);
        }
    } else if (strcmp(cmd, "read") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path to read: ") == 0) {
            memset(data, 0, sizeof(data));
            int read_bytes = file_mgr_read(path, data, sizeof(data) - 1);
            if (read_bytes >= 0) {
                printf("\nFile Content:\n%s\n", data);
            }
        }
    } else if (strcmp(cmd, "write") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path to write to: ") == 0) {
            if (get_arg_or_prompt(data, sizeof(data), 2, argc, argv, "Enter content to write: ") == 0) {
                file_mgr_write(path, data, strlen(data));
            }
        }
    } else if (strcmp(cmd, "delete") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path to delete: ") == 0) {
            file_mgr_delete(path);
        }
    } else if (strcmp(cmd, "copy") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter source path: ") == 0) {
            if (get_arg_or_prompt(dest, sizeof(dest), 2, argc, argv, "Enter destination path: ") == 0) {
                file_mgr_copy(path, dest);
            }
        }
    } else if (strcmp(cmd, "move") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter source path: ") == 0) {
            if (get_arg_or_prompt(dest, sizeof(dest), 2, argc, argv, "Enter destination path: ") == 0) {
                file_mgr_move(path, dest);
            }
        }
    } else if (strcmp(cmd, "rename") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter old path: ") == 0) {
            if (get_arg_or_prompt(dest, sizeof(dest), 2, argc, argv, "Enter new path: ") == 0) {
                file_mgr_rename(path, dest);
            }
        }
    } else if (strcmp(cmd, "info") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path to query info: ") == 0) {
            struct stat st;
            file_mgr_get_info(path, &st);
        }
    } else if (strcmp(cmd, "list") == 0) {
        /* Default to "." if no arg */
        if (argc > 1) {
            strncpy(path, argv[1], sizeof(path) - 1);
        } else {
            strcpy(path, ".");
        }
        file_mgr_list_dir(path);
    } else if (strcmp(cmd, "mkdir") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter directory path to create: ") == 0) {
            file_mgr_mkdir(path);
        }
    } else if (strcmp(cmd, "chmod") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter file path: ") == 0) {
            if (get_arg_or_prompt(mode, sizeof(mode), 2, argc, argv, "Enter permission mode (e.g. 755, 644, +x): ") == 0) {
                file_mgr_chmod(path, mode);
            }
        }
    } else if (strcmp(cmd, "search") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter start directory: ") == 0) {
            if (get_arg_or_prompt(dest, sizeof(dest), 2, argc, argv, "Enter search pattern (e.g. *.txt): ") == 0) {
                file_mgr_search(path, dest);
            }
        }
    } else if (strcmp(cmd, "archive") == 0) {
        if (get_arg_or_prompt(path, sizeof(path), 1, argc, argv, "Enter archive output path (e.g. out.tar.gz): ") == 0) {
            if (get_arg_or_prompt(dest, sizeof(dest), 2, argc, argv, "Enter source directory to archive: ") == 0) {
                file_mgr_archive(path, dest);
            }
        }
    } else {
        ui_print_error("Error: Unknown file command '%s'. Type / for options.\n", cmd);
    }
}

/* Dispatches commands under 'process' context */
static void dispatch_process(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    char pid_str[64] = "";
    char sig_str[64] = "";

    if (strcmp(cmd, "list") == 0) {
        process_mgr_list();
    } else if (strcmp(cmd, "info") == 0) {
        if (get_arg_or_prompt(pid_str, sizeof(pid_str), 1, argc, argv, "Enter process name or PID to search: ") == 0) {
            process_mgr_search(pid_str);
        }
    } else if (strcmp(cmd, "kill") == 0) {
        if (get_arg_or_prompt(pid_str, sizeof(pid_str), 1, argc, argv, "Enter PID to kill: ") == 0) {
            pid_t pid = (pid_t)atoi(pid_str);
            process_mgr_send_signal(pid, 9); /* SIGKILL */
        }
    } else if (strcmp(cmd, "signal") == 0) {
        if (get_arg_or_prompt(pid_str, sizeof(pid_str), 1, argc, argv, "Enter PID: ") == 0) {
            if (get_arg_or_prompt(sig_str, sizeof(sig_str), 2, argc, argv, "Enter signal number (e.g. 15 for SIGTERM): ") == 0) {
                pid_t pid = (pid_t)atoi(pid_str);
                int sig = atoi(sig_str);
                process_mgr_send_signal(pid, sig);
            }
        }
    } else if (strcmp(cmd, "nice") == 0) {
        if (get_arg_or_prompt(pid_str, sizeof(pid_str), 1, argc, argv, "Enter PID: ") == 0) {
            if (get_arg_or_prompt(sig_str, sizeof(sig_str), 2, argc, argv, "Enter nice value (-20 to 19): ") == 0) {
                pid_t pid = (pid_t)atoi(pid_str);
                int nice_val = atoi(sig_str);
                process_mgr_set_priority(pid, nice_val);
            }
        }
    } else if (strcmp(cmd, "fork") == 0) {
        fork_demo_run();
    } else if (strcmp(cmd, "exec") == 0) {
        exec_demo_run();
    } else if (strcmp(cmd, "wait") == 0) {
        wait_demo_run();
    } else if (strcmp(cmd, "zombie") == 0) {
        zombie_demo_run();
    } else if (strcmp(cmd, "orphan") == 0) {
        orphan_demo_run();
    } else if (strcmp(cmd, "daemon") == 0) {
        daemon_demo_run();
    } else if (strcmp(cmd, "siginit") == 0) {
        signal_mgr_init();
    } else if (strcmp(cmd, "sigreset") == 0) {
        signal_mgr_restore_defaults();
    } else {
        ui_print_error("Error: Unknown process command '%s'. Type / for options.\n", cmd);
    }
}

/* Dispatches commands under 'network' context */
static void dispatch_network(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    if (strcmp(cmd, "list") == 0) {
        network_list_interfaces_clean();
    } else if (strcmp(cmd, "info") == 0) {
        network_interface_info();
    } else if (strcmp(cmd, "config") == 0) {
        network_configure_interface_learning();
    } else if (strcmp(cmd, "up") == 0) {
        network_bring_interface_state("up");
    } else if (strcmp(cmd, "down") == 0) {
        network_bring_interface_state("down");
    } else if (strcmp(cmd, "route") == 0) {
        network_show_routes_ip();
    } else if (strcmp(cmd, "ping") == 0) {
        network_ping_host();
    } else if (strcmp(cmd, "dns") == 0) {
        network_dns_lookup();
    } else if (strcmp(cmd, "stats") == 0) {
        network_socket_statistics();
    } else {
        ui_print_error("Error: Unknown network command '%s'. Type / for options.\n", cmd);
    }
}

int is_interactive = 1;

/* Dispatches commands under 'socket' context */
static void dispatch_socket(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    char ip[128] = "";
    char port_str[64] = "";
    const char* sysmgr_cmd = (access("./sysmgr", F_OK) == 0) ? "./sysmgr" : "sysmgr";

    if (strcmp(cmd, "server") == 0) {
        if (is_interactive) {
            if (get_arg_or_prompt(port_str, sizeof(port_str), 1, argc, argv, "Enter port to bind [8080]: ") == -2) {
                return;
            }
            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket server %s", sysmgr_cmd, port_str[0] ? port_str : "8080");
            if (terminal_open("TCP Server", cmd_buf) != 0) {
                ui_print_error("Failed to launch socket server in a new terminal.\n");
            }
        } else {
            int port = 8080;
            if (argc > 1) {
                port = atoi(argv[1]);
            }
            socket_mgr_server_start(port);
        }
    } else if (strcmp(cmd, "client") == 0) {
        if (is_interactive) {
            if (get_arg_or_prompt(ip, sizeof(ip), 1, argc, argv, "Enter server IP [127.0.0.1]: ") == -2) {
                return;
            }
            if (get_arg_or_prompt(port_str, sizeof(port_str), 2, argc, argv, "Enter server port [8080]: ") == -2) {
                return;
            }
            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket client %s %s", sysmgr_cmd, ip[0] ? ip : "127.0.0.1", port_str[0] ? port_str : "8080");
            if (terminal_open("TCP Client", cmd_buf) != 0) {
                ui_print_error("Failed to launch socket client in a new terminal.\n");
            }
        } else {
            const char* server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
            int port = (argc > 2) ? atoi(argv[2]) : 8080;
            socket_mgr_client_start(server_ip, port, "user");
        }
    } else if (strcmp(cmd, "multi") == 0) {
        if (is_interactive) {
            if (get_arg_or_prompt(port_str, sizeof(port_str), 1, argc, argv, "Enter port to bind [8080]: ") == -2) {
                return;
            }
            char cmd_buf[256];
            snprintf(cmd_buf, sizeof(cmd_buf), "%s socket multi %s", sysmgr_cmd, port_str[0] ? port_str : "8080");
            if (terminal_open("Multi Client Server", cmd_buf) != 0) {
                ui_print_error("Failed to launch socket multi-server in a new terminal.\n");
            }
        } else {
            int port = 8080;
            if (argc > 1) {
                port = atoi(argv[1]);
            }
            log_info("SOCKET", "Socket process started");
            log_info("SOCKET", "Multi Client Server launched");
            socket_mgr_multi_server_start(port);
            log_info("SOCKET", "Terminal closed");
        }
    } else if (strcmp(cmd, "chat") == 0) {
        if (is_interactive) {
            char sub_cmd[64] = "";
            if (argc > 1) {
                strncpy(sub_cmd, argv[1], sizeof(sub_cmd) - 1);
            } else {
                const char* chat_options[] = {
                    "Host Chat (Launch a TCP Chat Host Server)",
                    "Join Chat (Connect to an active Chat Host)",
                    "Open Host + Client Demo (Launch local demo)",
                    "Cancel"
                };
                int sel = ui_select_menu("Socket Chat Options", chat_options, 4);
                if (sel == 0) {
                    strcpy(sub_cmd, "host");
                } else if (sel == 1) {
                    strcpy(sub_cmd, "client");
                } else if (sel == 2) {
                    strcpy(sub_cmd, "demo");
                } else {
                    return;
                }
            }
            
            if (strcmp(sub_cmd, "host") == 0) {
                if (get_arg_or_prompt(port_str, sizeof(port_str), 2, argc, argv, "Enter port to bind [8080]: ") == -2) {
                    return;
                }
                char cmd_buf[256];
                snprintf(cmd_buf, sizeof(cmd_buf), "%s socket chat --host %s", sysmgr_cmd, port_str[0] ? port_str : "8080");
                if (terminal_open("Chat Host", cmd_buf) != 0) {
                    ui_print_error("Failed to launch socket chat host in a new terminal.\n");
                }
            } else if (strcmp(sub_cmd, "client") == 0) {
                if (get_arg_or_prompt(ip, sizeof(ip), 2, argc, argv, "Enter host IP [127.0.0.1]: ") == -2) {
                    return;
                }
                if (get_arg_or_prompt(port_str, sizeof(port_str), 3, argc, argv, "Enter port [8080]: ") == -2) {
                    return;
                }
                char cmd_buf[256];
                snprintf(cmd_buf, sizeof(cmd_buf), "%s socket chat --client %s %s", sysmgr_cmd, ip[0] ? ip : "127.0.0.1", port_str[0] ? port_str : "8080");
                if (terminal_open("Chat Client", cmd_buf) != 0) {
                    ui_print_error("Failed to launch socket chat client in a new terminal.\n");
                }
            } else if (strcmp(sub_cmd, "demo") == 0) {
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
                        ui_print_error("Failed to launch demo Client Chat.\n");
                    }
                } else {
                    ui_print_error("Failed to launch demo Host Chat.\n");
                }
            } else {
                ui_print_error("Error: Invalid chat option. Choose host, client, or demo.\n");
            }
        } else {
            if (argc > 1) {
                if (strcmp(argv[1], "--host") == 0) {
                    int port = 8080;
                    if (argc > 2) {
                        port = atoi(argv[2]);
                    }
                    log_info("SOCKET", "Chat Host launched");
                    socket_mgr_server_start(port);
                } else if (strcmp(argv[1], "--client") == 0) {
                    const char* server_ip = (argc > 2) ? argv[2] : "127.0.0.1";
                    int port = (argc > 3) ? atoi(argv[3]) : 8080;
                    log_info("SOCKET", "Chat Client launched");
                    socket_mgr_client_start(server_ip, port, "client_user");
                } else {
                    ui_print_error("Error: Invalid chat parameters.\n");
                }
            } else {
                ui_print_error("Error: Missing chat mode (--host or --client).\n");
            }
        }
    } else if (strcmp(cmd, "status") == 0) {
        char* ss_argv[] = {"ss", "-t", "-a", NULL};
        printf("\n--- Active TCP Connections ---\n");
        execute_system_command(ss_argv);
    } else {
        ui_print_error("Error: Unknown socket command '%s'. Type / for options.\n", cmd);
    }
}

/* Dispatches commands under 'package' context */
static void dispatch_package(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    char pkg_name[256] = "";

    if (strcmp(cmd, "search") == 0) {
        if (get_arg_or_prompt(pkg_name, sizeof(pkg_name), 1, argc, argv, "Enter package query to search: ") == 0) {
            package_mgr_search(pkg_name);
        }
    } else if (strcmp(cmd, "info") == 0) {
        if (get_arg_or_prompt(pkg_name, sizeof(pkg_name), 1, argc, argv, "Enter package name: ") == 0) {
            package_mgr_info(pkg_name);
        }
    } else if (strcmp(cmd, "install") == 0) {
        if (get_arg_or_prompt(pkg_name, sizeof(pkg_name), 1, argc, argv, "Enter package to install: ") == 0) {
            package_mgr_install(pkg_name);
        }
    } else if (strcmp(cmd, "remove") == 0) {
        if (get_arg_or_prompt(pkg_name, sizeof(pkg_name), 1, argc, argv, "Enter package to remove: ") == 0) {
            package_mgr_remove(pkg_name);
        }
    } else if (strcmp(cmd, "demo") == 0) {
        package_mgr_demo();
    } else {
        ui_print_error("Error: Unknown package command '%s'. Type / for options.\n", cmd);
    }
}

/* Dispatches commands under 'shell' context */
static void dispatch_shell(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    char command_str[512] = "";

    if (strcmp(cmd, "exec") == 0) {
        if (get_arg_or_prompt(command_str, sizeof(command_str), 1, argc, argv, "Enter command to execute: ") == 0) {
            shell_mgr_execute(command_str);
        }
    } else if (strcmp(cmd, "script") == 0) {
        if (argc > 1) {
            char run_cmd[512];
            snprintf(run_cmd, sizeof(run_cmd), "/bin/bash scripts/%s", argv[1]);
            shell_mgr_execute(run_cmd);
        } else {
            const char* script_options[] = {
                "backup.sh (Execute pre-loaded backup script)",
                "disk_usage.sh (Show disk usage statistics)",
                "show_date.sh (Display current system date)",
                "Cancel"
            };
            int sel = ui_select_menu("Select Script to Run", script_options, 4);
            if (sel == 0) {
                shell_mgr_execute("/bin/bash scripts/backup.sh");
            } else if (sel == 1) {
                shell_mgr_execute("/bin/bash scripts/disk_usage.sh");
            } else if (sel == 2) {
                shell_mgr_execute("/bin/bash scripts/show_date.sh");
            }
        }
    } else if (strcmp(cmd, "env") == 0) {
        shell_mgr_env_run();
    } else if (strcmp(cmd, "file") == 0) {
        shell_mgr_execute("/bin/bash shell/file.sh");
    } else if (strcmp(cmd, "cron") == 0) {
        shell_mgr_cron_execute(argc, argv);
    } else if (strcmp(cmd, "time") == 0) {
        shell_mgr_time_execute(argc, argv);
    } else if (strcmp(cmd, "install") == 0) {
        shell_mgr_execute("/bin/bash shell/program.sh");
    } else if (strcmp(cmd, "monitor") == 0) {
        shell_mgr_execute("/bin/bash shell/monitor.sh");
    } else {
        ui_print_error("Error: Unknown shell command '%s'. Type / for options.\n", cmd);
    }
}

/* Dispatches commands under 'kernel' context */
static void dispatch_kernel(int argc, char** argv) {
    if (argc == 0) return;
    const char* cmd = argv[0];

    if (strcmp(cmd, "info") == 0) {
        kernel_mgr_show_info();
    } else if (strcmp(cmd, "load") == 0) {
        kernel_mgr_load_module();
    } else if (strcmp(cmd, "unload") == 0) {
        kernel_mgr_unload_module();
    } else if (strcmp(cmd, "status") == 0) {
        kernel_mgr_show_status();
    } else if (strcmp(cmd, "log") == 0) {
        kernel_mgr_show_log();
    } else if (strcmp(cmd, "stack") == 0) {
        kernel_mgr_show_network_stack();
    } else if (strcmp(cmd, "skbuff") == 0) {
        kernel_mgr_show_sk_buff_overview();
    } else if (strcmp(cmd, "napi") == 0) {
        kernel_mgr_show_napi_overview();
    } else {
        ui_print_error("Error: Unknown kernel module command '%s'. Type / for options.\n", cmd);
    }
}

/* General Command Route and Dispatcher */
static void route_and_dispatch(int argc, char** argv) {
    if (argc == 0) return;

    const char* root_token = argv[0];
    const char* ctx = context_stack[context_depth];

    /* Log dispatch command */
    log_info("REPL", "Command received: '%s' (args count: %d) under context '%s'", root_token, argc - 1, ctx);

    /* Main context-switching or root-level actions */
    if (strcmp(ctx, "main") == 0) {
        if (strcmp(root_token, "file") == 0) {
            if (argc == 1) {
                context_push("file");
            } else {
                dispatch_file(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "process") == 0) {
            if (argc == 1) {
                context_push("process");
            } else {
                dispatch_process(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "network") == 0) {
            if (argc == 1) {
                context_push("network");
            } else {
                dispatch_network(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "socket") == 0) {
            if (argc == 1) {
                context_push("socket");
            } else {
                dispatch_socket(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "package") == 0 || strcmp(root_token, "packet") == 0) {
            if (argc == 1) {
                context_push("package");
            } else {
                dispatch_package(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "shell") == 0) {
            if (argc == 1) {
                context_push("shell");
            } else {
                dispatch_shell(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "cron") == 0) {
            shell_mgr_cron_execute(argc, argv);
        } else if (strcmp(root_token, "time") == 0) {
            shell_mgr_time_execute(argc, argv);
        } else if (strcmp(root_token, "kernel") == 0) {
            if (argc == 1) {
                context_push("kernel");
            } else {
                dispatch_kernel(argc - 1, argv + 1);
            }
        } else if (strcmp(root_token, "help") == 0) {
            ui_show_help();
        } else if (strcmp(root_token, "exit") == 0) {
            log_info("REPL", "Graceful exit triggered.");
            ui_print_success("Goodbye!\n");
            exit(0);
        } else {
            ui_print_error("Error: Unknown context or command '%s'. Type / to browse contexts.\n", root_token);
        }
    } else {
        /* Under nested context */
        if (strcmp(root_token, "back") == 0 || strcmp(root_token, "..") == 0) {
            context_pop();
        } else {
            /* Route to context dispatcher */
            if (strcmp(ctx, "file") == 0) {
                dispatch_file(argc, argv);
            } else if (strcmp(ctx, "process") == 0) {
                dispatch_process(argc, argv);
            } else if (strcmp(ctx, "network") == 0) {
                dispatch_network(argc, argv);
            } else if (strcmp(ctx, "socket") == 0) {
                dispatch_socket(argc, argv);
            } else if (strcmp(ctx, "package") == 0) {
                dispatch_package(argc, argv);
            } else if (strcmp(ctx, "shell") == 0) {
                dispatch_shell(argc, argv);
            } else if (strcmp(ctx, "kernel") == 0) {
                dispatch_kernel(argc, argv);
            }
        }
    }
}

static int is_valid_command(const char* ctx, const char* cmd) {
    if (cmd == NULL || strlen(cmd) == 0) return 0;
    
    /* In main context, check main commands and sub-contexts */
    if (strcmp(ctx, "main") == 0) {
        if (strcmp(cmd, "file") == 0 ||
            strcmp(cmd, "process") == 0 ||
            strcmp(cmd, "network") == 0 ||
            strcmp(cmd, "socket") == 0 ||
            strcmp(cmd, "package") == 0 ||
            strcmp(cmd, "packet") == 0 ||
            strcmp(cmd, "shell") == 0 ||
            strcmp(cmd, "kernel") == 0 ||
            strcmp(cmd, "cron") == 0 ||
            strcmp(cmd, "time") == 0 ||
            strcmp(cmd, "help") == 0 ||
            strcmp(cmd, "exit") == 0) {
            return 1;
        }
        return 0;
    }
    
    /* In sub-contexts, check their commands */
    for (int i = 0; i < palette_items_count; i++) {
        if (strcmp(palette_items[i].context, ctx) == 0) {
            if (strcmp(palette_items[i].command, cmd) == 0) {
                return 1;
            }
            size_t cmd_len = strlen(cmd);
            if (strncmp(palette_items[i].command, cmd, cmd_len) == 0 && palette_items[i].command[cmd_len] == ' ') {
                return 1;
            }
        }
    }
    return 0;
}

void repl_run(void) {
    /* Initialize subsystems */
    history_init();
    autocomplete_init();

    /* Clear and print welcome header */
    ui_clear_screen();
    ui_show_banner("1.0.0");

    char prompt[128];
    char* line = NULL;

    while (1) {
        /* Set prompt dynamic context styling */
        const char* ctx = context_stack[context_depth];
        if (strcmp(ctx, "main") == 0) {
            snprintf(prompt, sizeof(prompt), "%ssysmgr > %s", ANSI_GREEN, ANSI_RESET);
        } else {
            snprintf(prompt, sizeof(prompt), "%ssysmgr/%s > %s", ANSI_CYAN, ctx, ANSI_RESET);
        }

        /* Read raw input line via linenoise */
        line = linenoise(prompt);

        /* Handle EOF or Ctrl+D */
        if (line == NULL) {
            printf("\n");
            break;
        }

        /* Handle ESC / back keys */
        if (strcmp(line, "\x1b") == 0) {
            linenoiseFree(line);
            context_pop();
            continue;
        }

        /* Handle empty lines */
        if (line[0] == '\0') {
            linenoiseFree(line);
            continue;
        }

        /* Save to history (if it's not a palette inspect command) */
        if (line[0] != '/') {
            history_add(line);
            history_save();
        }

        /* Handle Command Palette trigger or slash command execution */
        if (line[0] == '/') {
            char* cmd_part = line + 1;
            char* words[16];
            char* copy = strdup(cmd_part);
            int word_count = 0;
            char* tok = strtok(copy, " ");
            while (tok != NULL && word_count < 16) {
                words[word_count++] = tok;
                tok = strtok(NULL, " ");
            }
            
            if (word_count > 0 && is_valid_command(ctx, words[0])) {
                /* Valid command: strip '/' and execute directly */
                char* token_argv[64];
                int token_argc = parser_tokenize(cmd_part, token_argv, 64);
                if (token_argc > 0) {
                    route_and_dispatch(token_argc, token_argv);
                    parser_free_tokens(token_argv, token_argc);
                }
            } else {
                /* Invalid command or empty filter: show matching palette list */
                palette_search_and_show(ctx, line);
            }
            free(copy);
            linenoiseFree(line);
            continue;
        }

        /* Tokenize input */
        char* token_argv[64];
        int token_argc = parser_tokenize(line, token_argv, 64);

        if (token_argc > 0) {
            route_and_dispatch(token_argc, token_argv);
            parser_free_tokens(token_argv, token_argc);
        }

        linenoiseFree(line);
    }
}

void repl_execute_command(const char* cmd_line) {
    if (cmd_line == NULL || cmd_line[0] == '\0') return;
    
    char* copy = strdup(cmd_line);
    char* actual_cmd = copy;
    if (copy[0] == '/') {
        actual_cmd = copy + 1;
    }
    
    char* token_argv[64];
    int token_argc = parser_tokenize(actual_cmd, token_argv, 64);
    if (token_argc > 0) {
        route_and_dispatch(token_argc, token_argv);
        parser_free_tokens(token_argv, token_argc);
    }
    free(copy);
}
