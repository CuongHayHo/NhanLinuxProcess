/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/file/file_mgr.c
 * Purpose: File Manager Shell Programming launcher and TUI wrappers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "file_mgr.h"
#include "logger.h"
#include "menu.h"

static int run_file_script(char *const argv[], int *exit_code) {
    pid_t pid = fork();
    if (pid == -1) {
        log_error("FILE", "fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        execvp("shell/file.sh", argv);
        exit(errno);
    }
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        log_error("FILE", "waitpid failed: %s", strerror(errno));
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

int file_mgr_create(const char* path) {
    if (path == NULL || strlen(path) == 0) {
        log_error("FILE", "Invalid path parameter");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "create", (char*)path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File created: %s", path);
        return 0;
    }
    log_error("FILE", "Failed to create file: %s (status %d)", path, exit_code);
    return -1;
}

int file_mgr_read(const char* path, char* buffer, size_t size) {
    if (path == NULL || buffer == NULL || size == 0) {
        log_error("FILE", "Invalid parameters for read");
        return -1;
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        log_error("FILE", "pipe creation failed");
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        log_error("FILE", "fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char *argv[] = {"shell/file.sh", "read", (char*)path, NULL};
        execvp("shell/file.sh", argv);
        exit(1);
    }
    close(pipefd[1]);

    size_t total = 0;
    ssize_t n;
    while ((n = read(pipefd[0], buffer + total, size - total - 1)) > 0) {
        total += n;
    }
    buffer[total] = '\0';
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        log_info("FILE", "Read %zd bytes from file: %s", total, path);
        return (int)total;
    }
    log_error("FILE", "Failed to read file: %s", path);
    return -1;
}

int file_mgr_write(const char* path, const char* data, size_t size) {
    if (path == NULL || data == NULL) {
        log_error("FILE", "Invalid parameters for write");
        return -1;
    }
    
    // Convert data to a null terminated string for command line arg
    char *data_str = malloc(size + 1);
    if (!data_str) return -1;
    memcpy(data_str, data, size);
    data_str[size] = '\0';

    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "write", (char*)path, data_str, NULL};
    int res = run_file_script(argv, &exit_code);
    free(data_str);

    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File created: %s", path);
        return (int)size;
    }
    log_error("FILE", "Failed to write file: %s (status %d)", path, exit_code);
    return -1;
}

int file_mgr_delete(const char* path) {
    if (path == NULL || strlen(path) == 0) {
        log_error("FILE", "Invalid path parameter");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "delete", (char*)path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File removed: %s", path);
        return 0;
    }
    log_error("FILE", "Failed to remove file: %s (status %d)", path, exit_code);
    return -1;
}

int file_mgr_rename(const char* old_path, const char* new_path) {
    if (old_path == NULL || new_path == NULL) {
        log_error("FILE", "Invalid path parameters");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "rename", (char*)old_path, (char*)new_path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File moved: %s to %s", old_path, new_path);
        return 0;
    }
    log_error("FILE", "Failed to rename: %s (status %d)", old_path, exit_code);
    return -1;
}

int file_mgr_copy(const char* src_path, const char* dest_path) {
    if (src_path == NULL || dest_path == NULL) {
        log_error("FILE", "Invalid path parameters");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "copy", (char*)src_path, (char*)dest_path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File copied: %s to %s", src_path, dest_path);
        return 0;
    }
    log_error("FILE", "Failed to copy: %s (status %d)", src_path, exit_code);
    return -1;
}

int file_mgr_move(const char* src_path, const char* dest_path) {
    if (src_path == NULL || dest_path == NULL) {
        log_error("FILE", "Invalid path parameters");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "move", (char*)src_path, (char*)dest_path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "File moved: %s to %s", src_path, dest_path);
        return 0;
    }
    log_error("FILE", "Failed to move: %s (status %d)", src_path, exit_code);
    return -1;
}

int file_mgr_get_info(const char* path, struct stat* statbuf) {
    if (path == NULL) {
        log_error("FILE", "Invalid path parameter");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "info", (char*)path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        if (statbuf) {
            memset(statbuf, 0, sizeof(struct stat));
            statbuf->st_mode = S_IFREG | 0644;
            statbuf->st_size = 0;
        }
        return 0;
    }
    log_error("FILE", "Failed to get info for: %s", path);
    return -1;
}

int file_mgr_list_dir(const char* path) {
    if (path == NULL) {
        log_error("FILE", "Invalid path parameter");
        return -1;
    }
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "list", (char*)path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "Directory listed: %s", path);
        return 0;
    }
    log_error("FILE", "Failed to list directory: %s", path);
    return -1;
}

int file_mgr_mkdir(const char* path) {
    if (path == NULL) return -1;
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "mkdir", (char*)path, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        return 0;
    }
    return -1;
}

int file_mgr_chmod(const char* path, const char* mode) {
    if (path == NULL || mode == NULL) return -1;
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "chmod", (char*)path, (char*)mode, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "Permission changed: %s to %s", path, mode);
        return 0;
    }
    return -1;
}

int file_mgr_search(const char* start_dir, const char* pattern) {
    if (start_dir == NULL || pattern == NULL) return -1;
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "search", (char*)start_dir, (char*)pattern, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        return 0;
    }
    return -1;
}

int file_mgr_archive(const char* archive_path, const char* src_dir) {
    if (archive_path == NULL || src_dir == NULL) return -1;
    int exit_code = 0;
    char *argv[] = {"shell/file.sh", "archive", (char*)archive_path, (char*)src_dir, NULL};
    int res = run_file_script(argv, &exit_code);
    if (res == 0 && exit_code == 0) {
        log_info("FILE", "Archive created: %s", archive_path);
        return 0;
    }
    return -1;
}

static void file_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

static int read_file_choice(void) {
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

void file_mgr_run(void) {
    int choice;
    char path[256];
    char path2[256];
    char mode[64];
    char data[1024];

    log_info("FILE", "File manager interactive loop started.");

    while (1) {
        printf("\n========================================\n");
        printf("File Manager\n");
        printf("============\n");
        printf("1. Create File\n");
        printf("2. Read File\n");
        printf("3. Write File\n");
        printf("4. Delete File\n");
        printf("5. Rename File\n");
        printf("6. Copy File\n");
        printf("7. Move File\n");
        printf("8. File Information\n");
        printf("9. Directory Listing\n");
        printf("10. Create Directory\n");
        printf("11. Change Permission\n");
        printf("12. Search Files\n");
        printf("13. Archive\n");
        printf("14. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_file_choice();
        if (choice < 1 || choice > 14) {
            printf("\nInvalid choice. Please enter a number between 1 and 14.\n");
            file_menu_pause();
            continue;
        }

        if (choice == 14) {
            break;
        }

        if (choice == 1) {
            printf("Enter file path to create: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                file_mgr_create(path);
            }
            file_menu_pause();
        } else if (choice == 2) {
            printf("Enter file path to read: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                memset(data, 0, sizeof(data));
                int read_bytes = file_mgr_read(path, data, sizeof(data));
                if (read_bytes >= 0) {
                    printf("\nFile Content:\n%s\n", data);
                }
            }
            file_menu_pause();
        } else if (choice == 3) {
            printf("Enter file path to write to: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter content to write: ");
                fflush(stdout);
                if (fgets(data, sizeof(data), stdin) != NULL) {
                    data[strcspn(data, "\n")] = '\0';
                    file_mgr_write(path, data, strlen(data));
                }
            }
            file_menu_pause();
        } else if (choice == 4) {
            printf("Enter file path to delete: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                file_mgr_delete(path);
            }
            file_menu_pause();
        } else if (choice == 5) {
            printf("Enter old path: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter new path: ");
                fflush(stdout);
                if (fgets(path2, sizeof(path2), stdin) != NULL) {
                    path2[strcspn(path2, "\n")] = '\0';
                    file_mgr_rename(path, path2);
                }
            }
            file_menu_pause();
        } else if (choice == 6) {
            printf("Enter source path: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter destination path: ");
                fflush(stdout);
                if (fgets(path2, sizeof(path2), stdin) != NULL) {
                    path2[strcspn(path2, "\n")] = '\0';
                    file_mgr_copy(path, path2);
                }
            }
            file_menu_pause();
        } else if (choice == 7) {
            printf("Enter source path: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter destination path: ");
                fflush(stdout);
                if (fgets(path2, sizeof(path2), stdin) != NULL) {
                    path2[strcspn(path2, "\n")] = '\0';
                    file_mgr_move(path, path2);
                }
            }
            file_menu_pause();
        } else if (choice == 8) {
            printf("Enter file path to query info: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                struct stat st;
                file_mgr_get_info(path, &st);
            }
            file_menu_pause();
        } else if (choice == 9) {
            printf("Enter directory path to list: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                file_mgr_list_dir(path);
            }
            file_menu_pause();
        } else if (choice == 10) {
            printf("Enter directory path to create: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                file_mgr_mkdir(path);
            }
            file_menu_pause();
        } else if (choice == 11) {
            printf("Enter file path: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter permission mode (e.g. 755, 644, +x): ");
                fflush(stdout);
                if (fgets(mode, sizeof(mode), stdin) != NULL) {
                    mode[strcspn(mode, "\n")] = '\0';
                    file_mgr_chmod(path, mode);
                }
            }
            file_menu_pause();
        } else if (choice == 12) {
            printf("Enter start directory: ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter search pattern (e.g. *.txt): ");
                fflush(stdout);
                if (fgets(path2, sizeof(path2), stdin) != NULL) {
                    path2[strcspn(path2, "\n")] = '\0';
                    file_mgr_search(path, path2);
                }
            }
            file_menu_pause();
        } else if (choice == 13) {
            printf("Enter archive output path (e.g. out.tar.gz): ");
            fflush(stdout);
            if (fgets(path, sizeof(path), stdin) != NULL) {
                path[strcspn(path, "\n")] = '\0';
                printf("Enter target directory to archive: ");
                fflush(stdout);
                if (fgets(path2, sizeof(path2), stdin) != NULL) {
                    path2[strcspn(path2, "\n")] = '\0';
                    file_mgr_archive(path, path2);
                }
            }
            file_menu_pause();
        }
    }
}
