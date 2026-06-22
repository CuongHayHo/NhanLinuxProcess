/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/package/package_mgr.c
 * Purpose: Package manager Shell Programming launcher TUI and wrappers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "package_mgr.h"
#include "logger.h"
#include "menu.h"
#include "ui.h"
#include "repl.h"

static int run_script(char *const argv[], int *exit_code) {
    pid_t pid = fork();
    if (pid == -1) {
        log_error("PACKAGE", "fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        execvp("shell/program.sh", argv);
        exit(errno);
    }
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        log_error("PACKAGE", "waitpid failed: %s", strerror(errno));
        return -1;
    }
    if (WIFEXITED(status)) {
        if (exit_code) {
            *exit_code = WEXITSTATUS(status);
        }
        return 0;
    }
    return -1;
}

static void package_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

static int read_package_choice(void) {
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

const char* package_mgr_detect(void) {
    if (access("/usr/bin/dpkg-query", X_OK) == 0) {
        return "dpkg";
    }
    if (access("/usr/bin/rpm", X_OK) == 0) {
        return "rpm";
    }
    return NULL;
}

int package_mgr_search(const char* query) {
    if (!query || strlen(query) == 0) return -1;
    log_info("PACKAGE", "Package search");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "search", (char*)query, NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "Search failed: shell script returned status %d", exit_code);
        return -1;
    }
    return 0;
}

int package_mgr_info(const char* pkg_name) {
    if (!pkg_name || strlen(pkg_name) == 0) return -1;
    log_info("PACKAGE", "Package information");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "info", (char*)pkg_name, NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "Info query failed: shell script returned status %d", exit_code);
        return -1;
    }
    return 0;
}

int package_mgr_install(const char* pkg_name) {
    if (!pkg_name || strlen(pkg_name) == 0) return -1;
    log_info("PACKAGE", "Package install requested");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "install", (char*)pkg_name, NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "Install failed: shell script returned status %d", exit_code);
        return -1;
    }
    return 0;
}

int package_mgr_remove(const char* pkg_name) {
    if (!pkg_name || strlen(pkg_name) == 0) return -1;
    log_info("PACKAGE", "Package removal requested");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "remove", (char*)pkg_name, NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "Removal failed: shell script returned status %d", exit_code);
        return -1;
    }
    return 0;
}

int is_package_installed(const char* pkg_name) {
    if (!pkg_name || strlen(pkg_name) == 0) return 0;
    const char *pm = package_mgr_detect();
    if (!pm) return 0;
    pid_t pid = fork();
    if (pid == -1) return 0;
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        if (strcmp(pm, "dpkg") == 0) {
            char *argv[] = {"dpkg-query", "-W", "-f=${Status}", (char*)pkg_name, NULL};
            execvp("dpkg-query", argv);
        } else {
            char *argv[] = {"rpm", "-q", (char*)pkg_name, NULL};
            execvp("rpm", argv);
        }
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

void package_mgr_run(void) {
    int choice;
    char pkg_name[128];
    extern int is_interactive;

    log_info("PACKAGE", "Entering Package Manager (Shell script execution)");

    while (1) {
        if (is_interactive) {
            const char* package_options[] = {
                "Search Package (Tìm kiếm gói)",
                "Package Information (Thông tin gói)",
                "Install Package (Cài đặt gói)",
                "Remove Package (Gỡ bỏ gói)",
                "Safe Demonstration (Chạy Demo an toàn)",
                "Setup Environment (Cài đặt môi trường SSH Docker)",
                "Return (Trở về)"
            };
            int sel = ui_select_menu("Package Manager", package_options, 7);
            if (sel == 6 || sel == -1) {
                break;
            }
            choice = sel + 1;
        } else {
            printf("\n========================================\n");
            printf("Package Manager\n");
            printf("===============\n");
            printf("1. Search Package\n");
            printf("2. Package Information\n");
            printf("3. Install Package\n");
            printf("4. Remove Package\n");
            printf("5. Safe Demonstration\n");
            printf("6. Setup Environment\n");
            printf("7. Return\n");
            printf("========================================\n");
            printf("Select option: ");
            fflush(stdout);

            choice = read_package_choice();
            if (choice < 1 || choice > 7) {
                printf("\nInvalid choice. Please enter a number between 1 and 7.\n");
                package_menu_pause();
                continue;
            }

            if (choice == 7) {
                break;
            }
        }

        if (choice == 1) {
            if (is_interactive) print_prompt_explanation("Enter package query to search");
            printf("Enter package name to search (e.g. nginx, python, gcc, docker, git): ");
            fflush(stdout);
            if (fgets(pkg_name, sizeof(pkg_name), stdin) != NULL) {
                pkg_name[strcspn(pkg_name, "\n")] = '\0';
                if (strlen(pkg_name) > 0) {
                    package_mgr_search(pkg_name);
                }
            }
            package_menu_pause();
        } else if (choice == 2) {
            if (is_interactive) print_prompt_explanation("Enter package name");
            printf("Enter package name for information (e.g. tmux, curl, htop, wget, git): ");
            fflush(stdout);
            if (fgets(pkg_name, sizeof(pkg_name), stdin) != NULL) {
                pkg_name[strcspn(pkg_name, "\n")] = '\0';
                if (strlen(pkg_name) > 0) {
                    package_mgr_info(pkg_name);
                }
            }
            package_menu_pause();
        } else if (choice == 3) {
            if (is_interactive) print_prompt_explanation("Enter package to install");
            printf("Enter package name to install (e.g. tmux, curl, htop, wget, git): ");
            fflush(stdout);
            if (fgets(pkg_name, sizeof(pkg_name), stdin) != NULL) {
                pkg_name[strcspn(pkg_name, "\n")] = '\0';
                if (strlen(pkg_name) > 0) {
                    package_mgr_install(pkg_name);
                }
            }
            package_menu_pause();
        } else if (choice == 4) {
            if (is_interactive) print_prompt_explanation("Enter package to remove");
            printf("Enter package name to remove (e.g. tmux, curl, htop, wget, git): ");
            fflush(stdout);
            if (fgets(pkg_name, sizeof(pkg_name), stdin) != NULL) {
                pkg_name[strcspn(pkg_name, "\n")] = '\0';
                if (strlen(pkg_name) > 0) {
                    package_mgr_remove(pkg_name);
                }
            }
            package_menu_pause();
        } else if (choice == 5) {
            package_mgr_demo();
            package_menu_pause();
        } else if (choice == 6) {
            package_mgr_setup();
            package_menu_pause();
        }
    }

    log_info("PACKAGE", "Leaving Package Manager");
}

int package_mgr_demo(void) {
    log_info("PACKAGE", "Safe demonstration started");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "demo", NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "Safe demonstration failed with status %d", exit_code);
        printf("\nSafe Demonstration failed.\n");
        return -1;
    }
    return 0;
}

int package_mgr_setup(void) {
    log_info("PACKAGE", "System setup started");
    int exit_code = 0;
    char *argv[] = {"shell/program.sh", "setup", NULL};
    int res = run_script(argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("PACKAGE", "System setup failed with status %d", exit_code);
        printf("\nSystem setup failed.\n");
        return -1;
    }
    log_info("PACKAGE", "System setup completed successfully");
    printf("\nSystem setup completed successfully.\n");
    return 0;
}
