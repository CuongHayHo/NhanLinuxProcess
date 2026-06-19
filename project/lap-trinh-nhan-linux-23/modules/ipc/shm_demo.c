/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/ipc/shm_demo.c
 * Purpose: Standalone POSIX Shared Memory IPC demonstration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "ipc_demo.h"

#define SHM_NAME "/sysmgr_shm_demo"
#define SHM_SIZE 4096

/* Map legacy key-based stubs to POSIX operations */
int shm_demo_create(key_t key, size_t size) {
    (void)key;
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) return -1;
    if (ftruncate(fd, size) == -1) {
        close(fd);
        shm_unlink(SHM_NAME);
        return -1;
    }
    return fd;
}

void* shm_demo_attach(int fd) {
    void* ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); /* Safe to close fd after mapping */
    return ptr;
}

int shm_demo_detach(const void* addr) {
    return munmap((void*)addr, SHM_SIZE);
}

int main(void) {
    printf("POSIX Shared Memory Demonstration Program.\n");

    /* Create segment */
    int fd = shm_demo_create(0, SHM_SIZE);
    if (fd == -1) {
        perror("shm_demo_create failed");
        return 1;
    }
    printf("[PARENT] POSIX Shared memory segment created.\n");

    /* Map segment */
    void* ptr = shm_demo_attach(fd);
    if (ptr == MAP_FAILED) {
        perror("shm_demo_attach failed");
        shm_unlink(SHM_NAME);
        return 1;
    }
    printf("[PARENT] Segment mapped at address: %p\n", ptr);

    /* Write data */
    const char* data = "IPC Shared Memory Test Message";
    snprintf((char*)ptr, SHM_SIZE, "%s", data);
    printf("[PARENT] Message written: '%s'\n", data);

    /* Fork a child process to act as the Reader */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        shm_demo_detach(ptr);
        shm_unlink(SHM_NAME);
        return 1;
    }

    if (pid == 0) {
        /* Child process (Reader) */
        usleep(50000); /* 50ms */
        printf("[CHILD] Reader process spawned.\n");

        int r_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
        if (r_fd == -1) {
            perror("[CHILD] shm_open failed");
            exit(1);
        }

        void* r_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, r_fd, 0);
        if (r_ptr == MAP_FAILED) {
            perror("[CHILD] mmap failed");
            close(r_fd);
            exit(1);
        }
        close(r_fd);

        printf("[CHILD] Message read from shared memory: '%s'\n", (char*)r_ptr);

        munmap(r_ptr, SHM_SIZE);
        exit(0);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        printf("[PARENT] Child reader finished execution.\n");

        /* Clean up */
        shm_demo_detach(ptr);
        if (shm_unlink(SHM_NAME) == -1) {
            perror("shm_unlink failed");
            return 1;
        }
        printf("[PARENT] Shared memory segment unlinked and destroyed successfully.\n");
    }

    return 0;
}
