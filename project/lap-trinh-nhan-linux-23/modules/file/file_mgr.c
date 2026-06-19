/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/file/file_mgr.c
 * Purpose: Low-level file system call operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include "file_mgr.h"
#include "logger.h"

void file_mgr_run(void) {
    printf("--- File Manager (Stub) ---\n");
    printf("1. Create Stub File\n");
    printf("2. List Dir Stub\n");
    printf("0. Return\n");
    log_info("FILE", "File manager interactive loop started.");
    
    /* Stub code simply returns */
    // TODO: Implement interactive shell menu mapping to syscall functions
}

int file_mgr_create(const char* path) {
    int fd;

    if (path == NULL) {
        log_error("FILE", "Failed to create file: NULL path pointer");
        return -1;
    }

    /* Open file with O_WRONLY, O_CREAT, and O_TRUNC flags (permission 0644) */
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        log_error("FILE", "Failed to create file '%s'", path);
        return -1;
    }

    log_info("FILE", "Created file: %s", path);
    close(fd);
    return 0;
}

int file_mgr_read(const char* path, char* buffer, size_t size) {
    int fd;
    ssize_t bytes_read;

    if (path == NULL || buffer == NULL || size == 0) {
        log_error("FILE", "Failed to read file: invalid parameters");
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        log_error("FILE", "Failed to open file for reading '%s'", path);
        return -1;
    }

    /* Read up to size - 1 bytes to guarantee null-termination */
    bytes_read = read(fd, buffer, size - 1);
    if (bytes_read < 0) {
        log_error("FILE", "Failed to read data from file '%s'", path);
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';
    log_info("FILE", "Read %zd bytes from file: %s", bytes_read, path);
    close(fd);
    return (int)bytes_read;
}

int file_mgr_write(const char* path, const char* data, size_t size) {
    int fd;
    ssize_t bytes_written;

    if (path == NULL || data == NULL) {
        log_error("FILE", "Failed to write file: invalid parameters");
        return -1;
    }

    /* Open file for writing, create if missing, truncate existing data */
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        log_error("FILE", "Failed to open file for writing '%s'", path);
        return -1;
    }

    bytes_written = write(fd, data, size);
    if (bytes_written < 0) {
        log_error("FILE", "Failed to write data to file '%s'", path);
        close(fd);
        return -1;
    }

    log_info("FILE", "Wrote %zd bytes to file: %s", bytes_written, path);
    close(fd);
    return (int)bytes_written;
}

int file_mgr_delete(const char* path) {
    if (path == NULL) {
        log_error("FILE", "Failed to delete: NULL path pointer");
        return -1;
    }

    if (unlink(path) != 0) {
        log_error("FILE", "Failed to delete file '%s'", path);
        return -1;
    }

    log_info("FILE", "Deleted file: %s", path);
    return 0;
}

int file_mgr_rename(const char* old_path, const char* new_path) {
    if (old_path == NULL || new_path == NULL) {
        log_error("FILE", "Failed to rename: NULL path pointers");
        return -1;
    }

    if (rename(old_path, new_path) != 0) {
        log_error("FILE", "Failed to rename '%s' to '%s'", old_path, new_path);
        return -1;
    }

    log_info("FILE", "Renamed file from '%s' to '%s'", old_path, new_path);
    return 0;
}

int file_mgr_copy(const char* src_path, const char* dest_path) {
    int fd_src = -1;
    int fd_dest = -1;
    struct stat stat_src;
    mode_t mode = 0644;
    char buffer[4096];
    ssize_t bytes_read;

    if (src_path == NULL || dest_path == NULL) {
        log_error("FILE", "Failed to copy: NULL path pointers");
        return -1;
    }

    fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        log_error("FILE", "Failed to copy: Cannot open source '%s'", src_path);
        return -1;
    }

    /* Get source file permissions to match on destination */
    if (fstat(fd_src, &stat_src) == 0) {
        mode = stat_src.st_mode & 0777;
    }

    fd_dest = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd_dest < 0) {
        log_error("FILE", "Failed to copy: Cannot create destination '%s'", dest_path);
        close(fd_src);
        return -1;
    }

    /* Manual byte-by-byte copy loop */
    while ((bytes_read = read(fd_src, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = write(fd_dest, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            log_error("FILE", "Failed to copy: Write mismatch on destination '%s'", dest_path);
            close(fd_src);
            close(fd_dest);
            return -1;
        }
    }

    if (bytes_read < 0) {
        log_error("FILE", "Failed to copy: Read error on source '%s'", src_path);
        close(fd_src);
        close(fd_dest);
        return -1;
    }

    close(fd_src);
    close(fd_dest);
    log_info("FILE", "Copied file from '%s' to '%s'", src_path, dest_path);
    return 0;
}

int file_mgr_move(const char* src_path, const char* dest_path) {
    if (src_path == NULL || dest_path == NULL) {
        log_error("FILE", "Failed to move: NULL path pointers");
        return -1;
    }

    /* 1. Try standard rename first */
    if (rename(src_path, dest_path) == 0) {
        log_info("FILE", "Moved file from '%s' to '%s' (via rename)", src_path, dest_path);
        return 0;
    }

    /* 2. Fallback to copy and delete if rename fails (e.g., cross-filesystem boundary) */
    log_warning("FILE", "Rename failed for '%s' to '%s', trying fallback copy+delete", src_path, dest_path);

    if (file_mgr_copy(src_path, dest_path) != 0) {
        log_error("FILE", "Failed to move: Fallback copy from '%s' to '%s' failed", src_path, dest_path);
        return -1;
    }

    if (file_mgr_delete(src_path) != 0) {
        log_error("FILE", "Failed to move: Fallback delete of '%s' failed. Removing partial target", src_path);
        unlink(dest_path); /* Clean up destination to prevent corrupt partial state */
        return -1;
    }

    log_info("FILE", "Moved file from '%s' to '%s' (via fallback copy+delete)", src_path, dest_path);
    return 0;
}

int file_mgr_get_info(const char* path, struct stat* statbuf) {
    mode_t mode;
    char perm[11];
    const char* type_str;
    char time_str[64];
    struct tm mtime_info;

    if (path == NULL || statbuf == NULL) {
        log_error("FILE", "Failed to get file info: invalid pointers");
        return -1;
    }

    if (stat(path, statbuf) != 0) {
        log_error("FILE", "Failed to retrieve metadata for '%s'", path);
        return -1;
    }

    mode = statbuf->st_mode;

    /* Build Unix style permission string */
    perm[0] = S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : (S_ISFIFO(mode) ? 'p' : '-'));
    perm[1] = (mode & S_IRUSR) ? 'r' : '-';
    perm[2] = (mode & S_IWUSR) ? 'w' : '-';
    perm[3] = (mode & S_IXUSR) ? 'x' : '-';
    perm[4] = (mode & S_IRGRP) ? 'r' : '-';
    perm[5] = (mode & S_IWGRP) ? 'w' : '-';
    perm[6] = (mode & S_IXGRP) ? 'x' : '-';
    perm[7] = (mode & S_IROTH) ? 'r' : '-';
    perm[8] = (mode & S_IWOTH) ? 'w' : '-';
    perm[9] = (mode & S_IXOTH) ? 'x' : '-';
    perm[10] = '\0';

    /* Determine file type description */
    type_str = "Unknown";
    if (S_ISREG(mode)) type_str = "Regular File";
    else if (S_ISDIR(mode)) type_str = "Directory";
    else if (S_ISLNK(mode)) type_str = "Symbolic Link";
    else if (S_ISCHR(mode)) type_str = "Character Device";
    else if (S_ISBLK(mode)) type_str = "Block Device";
    else if (S_ISFIFO(mode)) type_str = "FIFO (Named Pipe)";
    else if (S_ISSOCK(mode)) type_str = "Socket";

    /* Format modification time safely */
    if (localtime_r(&statbuf->st_mtime, &mtime_info) != NULL) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &mtime_info);
    } else {
        strncpy(time_str, "N/A", sizeof(time_str));
    }

    /* Print details directly to standard output */
    printf("Name: %s\n", path);
    printf("Type: %s\n", type_str);
    printf("Size: %ld bytes\n", (long)statbuf->st_size);
    printf("Permission: %s (%04o)\n", perm, mode & 0777);
    printf("Owner UID: %d\n", statbuf->st_uid);
    printf("Owner GID: %d\n", statbuf->st_gid);
    printf("Last modified time: %s\n", time_str);

    log_info("FILE", "Queried metadata for file: %s", path);
    return 0;
}

int file_mgr_list_dir(const char* path) {
    DIR* dir;
    struct dirent* entry;
    char full_path[1024];
    struct stat statbuf;

    if (path == NULL) {
        log_error("FILE", "Failed to list directory: NULL path pointer");
        return -1;
    }

    dir = opendir(path);
    if (dir == NULL) {
        log_error("FILE", "Failed to open directory '%s'", path);
        return -1;
    }

    printf("Directory Listing of: %s\n", path);
    printf("%-30s %-15s %-12s %-12s\n", "Name", "Type", "Size", "Permission");
    printf("--------------------------------------------------------------------------\n");

    while ((entry = readdir(dir)) != NULL) {
        mode_t mode = 0;
        long size = 0;
        char perm[11] = "----------";
        const char* type_str = "Unknown";

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &statbuf) == 0) {
            mode = statbuf.st_mode;
            size = (long)statbuf.st_size;

            /* Parse permissions */
            perm[0] = S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : (S_ISFIFO(mode) ? 'p' : '-'));
            perm[1] = (mode & S_IRUSR) ? 'r' : '-';
            perm[2] = (mode & S_IWUSR) ? 'w' : '-';
            perm[3] = (mode & S_IXUSR) ? 'x' : '-';
            perm[4] = (mode & S_IRGRP) ? 'r' : '-';
            perm[5] = (mode & S_IWGRP) ? 'w' : '-';
            perm[6] = (mode & S_IXGRP) ? 'x' : '-';
            perm[7] = (mode & S_IROTH) ? 'r' : '-';
            perm[8] = (mode & S_IWOTH) ? 'w' : '-';
            perm[9] = (mode & S_IXOTH) ? 'x' : '-';
            perm[10] = '\0';

            /* Parse type */
            if (S_ISREG(mode)) type_str = "Regular File";
            else if (S_ISDIR(mode)) type_str = "Directory";
            else if (S_ISLNK(mode)) type_str = "Symbolic Link";
            else if (S_ISCHR(mode)) type_str = "Char Device";
            else if (S_ISBLK(mode)) type_str = "Block Device";
            else if (S_ISFIFO(mode)) type_str = "Named Pipe";
            else if (S_ISSOCK(mode)) type_str = "Socket";
        } else {
            /* Fallback type strings if stat fails (e.g. broken symlink or special folder) */
            if (entry->d_type == DT_DIR) {
                type_str = "Directory";
                perm[0] = 'd';
            } else if (entry->d_type == DT_REG) {
                type_str = "Regular File";
            }
        }

        printf("%-30s %-15s %-12ld %-12s\n", entry->d_name, type_str, size, perm);
    }

    printf("--------------------------------------------------------------------------\n");
    closedir(dir);
    log_info("FILE", "Listed directory: %s", path);
    return 0;
}
