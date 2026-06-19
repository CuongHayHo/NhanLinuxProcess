/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/ipc/sem_demo.c
 * Purpose: Standalone POSIX Semaphore synchronization demonstration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "ipc_demo.h"

#define SEM_NAME "/sysmgr_sem_demo"

sem_t* sem_demo_init(const char* name, int value) {
    /* Initialize named POSIX semaphore with initial value */
    sem_t* sem = sem_open(name, O_CREAT, 0666, value);
    return sem;
}

int sem_demo_wait(sem_t* sem) {
    return sem_wait(sem);
}

int sem_demo_post(sem_t* sem) {
    return sem_post(sem);
}

int main(void) {
    printf("POSIX Named Semaphore Demonstration Program.\n");

    /* Create/initialize named semaphore with value 1 */
    sem_t* sem = sem_demo_init(SEM_NAME, 1);
    if (sem == SEM_FAILED) {
        perror("sem_demo_init failed");
        return 1;
    }
    printf("[PARENT] Semaphore initialized: name = %s, value = 1\n", SEM_NAME);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        return 1;
    }

    if (pid == 0) {
        /* Child Process */
        printf("[CHILD] Child process spawned. Requesting semaphore lock...\n");
        sem_t* child_sem = sem_open(SEM_NAME, 0);
        if (child_sem == SEM_FAILED) {
            perror("[CHILD] sem_open failed");
            exit(1);
        }

        if (sem_demo_wait(child_sem) == -1) {
            perror("[CHILD] sem_wait failed");
            sem_close(child_sem);
            exit(1);
        }
        printf("[CHILD] Lock acquired (critical section start).\n");
        sleep(1); /* Simulate work */
        printf("[CHILD] Releasing lock (critical section end).\n");
        sem_demo_post(child_sem);

        sem_close(child_sem);
        exit(0);
    } else {
        /* Parent Process */
        printf("[PARENT] Parent process requesting semaphore lock...\n");
        
        if (sem_demo_wait(sem) == -1) {
            perror("[PARENT] sem_wait failed");
            sem_close(sem);
            sem_unlink(SEM_NAME);
            return 1;
        }
        printf("[PARENT] Lock acquired (critical section start).\n");
        sleep(1); /* Simulate work */
        printf("[PARENT] Releasing lock (critical section end).\n");
        sem_demo_post(sem);

        /* Wait for child process to finish */
        int status;
        waitpid(pid, &status, 0);
        printf("[PARENT] Child finished execution.\n");

        /* Close and unlink named semaphore */
        sem_close(sem);
        if (sem_unlink(SEM_NAME) == -1) {
            perror("sem_unlink failed");
            return 1;
        }
        printf("[PARENT] Semaphore destroyed cleanly.\n");
    }

    return 0;
}
