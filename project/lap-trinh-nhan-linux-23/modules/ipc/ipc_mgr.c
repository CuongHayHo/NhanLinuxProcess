/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/ipc/ipc_mgr.c
 * Purpose: POSIX Shared Memory IPC manager and TUI submenu loop.
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
#include <semaphore.h>
#include "ipc_mgr.h"
#include "logger.h"
#include "menu.h"

#define SHM_NAME "/sysmgr_shm"
#define SHM_SIZE 4096
#define SEM_NAME "/sysmgr_sem"

static int read_ipc_choice(void) {
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

static void ipc_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void ipc_mgr_run(void) {
    int choice;
    log_info("IPC", "Entering IPC Manager");

    while (1) {
        printf("\n========================================\n");
        printf("              IPC Manager\n");
        printf("========================================\n");
        printf("1. Shared Memory Demo\n");
        printf("2. Semaphore Demo\n");
        printf("3. Producer Consumer Demo\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_ipc_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 3) {
            printf("\nInvalid input. Please choose a number between 0 and 3.\n");
            ipc_menu_pause();
            continue;
        }

        /* Support both 0 and 4 for Return as per acceptance criteria and UI standards */
        if (choice == 0 || choice == 4) {
            log_info("IPC", "Leaving IPC Manager");
            break;
        }

        if (choice == 1) {
            ipc_mgr_shm_demo();
            ipc_menu_pause();
        } else if (choice == 2) {
            ipc_mgr_sem_demo();
            ipc_menu_pause();
        } else if (choice == 3) {
            ipc_mgr_prod_cons_demo();
            ipc_menu_pause();
        }
    }
}

int ipc_mgr_shm_demo(void) {
    int fd;
    void* ptr;
    pid_t pid;

    printf("\n--- Starting POSIX Shared Memory Demo ---\n");

    /* 1. Create POSIX shared memory object */
    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        log_error("IPC", "Errors: shm_open failed (errno %d)", errno);
        printf("Error: Failed to create POSIX shared memory object (shm_open failed).\n");
        return -1;
    }
    log_info("IPC", "Shared memory created");
    printf("[WRITER] Shared memory created: name = %s\n", SHM_NAME);

    /* 2. Set the size of the shared memory object */
    if (ftruncate(fd, SHM_SIZE) == -1) {
        log_error("IPC", "Errors: ftruncate failed (errno %d)", errno);
        printf("Error: Failed to size POSIX shared memory object (ftruncate failed).\n");
        close(fd);
        shm_unlink(SHM_NAME);
        return -1;
    }

    /* 3. Map shared memory into caller's address space */
    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        log_error("IPC", "Errors: mmap failed (errno %d)", errno);
        printf("Error: Failed to map POSIX shared memory (mmap failed).\n");
        close(fd);
        shm_unlink(SHM_NAME);
        return -1;
    }
    log_info("IPC", "Mapped");
    printf("[WRITER] Shared memory mapped at address: %p\n", ptr);

    /* Close file descriptor (no longer needed after mapping) */
    close(fd);

    /* 4. Write message to shared memory */
    const char* message = "Hello from POSIX Shared Memory Writer!";
    snprintf((char*)ptr, SHM_SIZE, "%s", message);
    log_info("IPC", "Written");
    printf("[WRITER] Message written to shared memory: '%s'\n", message);

    /* 5. Fork Reader process */
    printf("[WRITER] Forking independent Reader process...\n");
    pid = fork();
    if (pid < 0) {
        log_error("IPC", "Errors: fork failed (errno %d)", errno);
        printf("Error: Fork failed.\n");
        munmap(ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);
        return -1;
    }

    if (pid == 0) {
        /* Independent Child Process (Reader) */
        int r_fd;
        void* r_ptr;

        /* Sleep briefly to simulate async processing */
        usleep(100000); /* 100ms */

        printf("[READER] Reader process spawned (PID: %d).\n", getpid());

        /* Open POSIX shared memory */
        r_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
        if (r_fd == -1) {
            log_error("IPC", "Errors: reader shm_open failed (errno %d)", errno);
            printf("[READER] Error: Failed to open shared memory.\n");
            exit(1);
        }

        /* Map shared memory */
        r_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, r_fd, 0);
        if (r_ptr == MAP_FAILED) {
            log_error("IPC", "Errors: reader mmap failed (errno %d)", errno);
            printf("[READER] Error: Failed to map shared memory.\n");
            close(r_fd);
            exit(1);
        }
        log_info("IPC", "Mapped");
        printf("[READER] Shared memory mapped at address: %p\n", r_ptr);
        close(r_fd);

        /* Read message */
        printf("[READER] Message read from shared memory: '%s'\n", (char*)r_ptr);
        log_info("IPC", "Read");

        /* Unmap shared memory */
        munmap(r_ptr, SHM_SIZE);
        printf("[READER] Shared memory unmapped.\n");
        exit(0);
    } else {
        /* Parent Process (Writer / Coordinator) */
        int status;
        waitpid(pid, &status, 0);
        printf("[WRITER] Reader process finished execution.\n");

        /* 6. Unmap and Destroy (unlink) shared memory object */
        munmap(ptr, SHM_SIZE);
        printf("[WRITER] Shared memory unmapped.\n");

        if (shm_unlink(SHM_NAME) == -1) {
            log_error("IPC", "Errors: shm_unlink failed (errno %d)", errno);
            printf("Error: Failed to unlink POSIX shared memory.\n");
            return -1;
        }
        log_info("IPC", "Destroyed");
        printf("[WRITER] Shared memory object destroyed cleanly.\n");
    }

    printf("--- Shared Memory Demo Completed Successfully ---\n");
    return 0;
}

int ipc_mgr_sem_demo(void) {
    sem_t* sem;
    pid_t pid;

    printf("\n--- Starting POSIX Named Semaphore Demo ---\n");

    /* 1. Create named semaphore with value 1 */
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        log_error("IPC", "Errors: sem_open failed (errno %d)", errno);
        printf("Error: Failed to create POSIX named semaphore (sem_open failed).\n");
        return -1;
    }
    log_info("IPC", "Semaphore created");
    printf("[PARENT] Named Semaphore created: name = %s, initial value = 1\n", SEM_NAME);

    /* 2. Fork Reader/Worker process */
    printf("[PARENT] Forking independent process to demonstrate lock contention...\n");
    pid = fork();
    if (pid < 0) {
        log_error("IPC", "Errors: fork failed (errno %d)", errno);
        printf("Error: Fork failed.\n");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        return -1;
    }

    if (pid == 0) {
        /* Independent Child Process */
        sem_t* child_sem;

        printf("[CHILD] Spawned (PID: %d). Opening named semaphore...\n", getpid());
        child_sem = sem_open(SEM_NAME, 0);
        if (child_sem == SEM_FAILED) {
            log_error("IPC", "Errors: child sem_open failed (errno %d)", errno);
            exit(1);
        }

        printf("[CHILD] Requesting lock (sem_wait)...\n");
        log_info("IPC", "Wait");
        if (sem_wait(child_sem) == -1) {
            log_error("IPC", "Errors: child sem_wait failed (errno %d)", errno);
            sem_close(child_sem);
            exit(1);
        }
        printf("[CHILD] Lock acquired! Accessing critical section...\n");
        usleep(200000); /* 200ms simulated work */
        printf("[CHILD] Releasing lock (sem_post)...\n");
        log_info("IPC", "Post");
        sem_post(child_sem);

        sem_close(child_sem);
        printf("[CHILD] Finished execution.\n");
        exit(0);
    } else {
        /* Parent Process */
        int status;

        printf("[PARENT] Requesting lock (sem_wait)...\n");
        log_info("IPC", "Wait");
        if (sem_wait(sem) == -1) {
            log_error("IPC", "Errors: parent sem_wait failed (errno %d)", errno);
            sem_close(sem);
            sem_unlink(SEM_NAME);
            return -1;
        }
        printf("[PARENT] Lock acquired! Accessing critical section...\n");
        usleep(200000); /* 200ms simulated work */
        printf("[PARENT] Releasing lock (sem_post)...\n");
        log_info("IPC", "Post");
        sem_post(sem);

        /* Wait for child process to finish */
        waitpid(pid, &status, 0);
        printf("[PARENT] Child worker process finished execution.\n");

        /* 3. Close and unlink semaphore */
        sem_close(sem);
        if (sem_unlink(SEM_NAME) == -1) {
            log_error("IPC", "Errors: sem_unlink failed (errno %d)", errno);
            printf("Error: Failed to unlink POSIX named semaphore.\n");
            return -1;
        }
        log_info("IPC", "Destroyed");
        printf("[PARENT] Named Semaphore destroyed cleanly.\n");
    }

    printf("--- Semaphore Demo Completed Successfully ---\n");
    return 0;
}

int ipc_mgr_prod_cons_demo(void) {
    int fd;
    void* ptr;
    sem_t* sem;
    pid_t pid;

    printf("\n--- Starting POSIX Producer Consumer Demo ---\n");

    /* 1. Setup Shared Memory Object */
    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        log_error("IPC", "Errors: shm_open failed (errno %d)", errno);
        printf("Error: Failed to create POSIX shared memory object (shm_open failed).\n");
        return -1;
    }

    if (ftruncate(fd, SHM_SIZE) == -1) {
        log_error("IPC", "Errors: ftruncate failed (errno %d)", errno);
        printf("Error: Failed to size POSIX shared memory object.\n");
        close(fd);
        shm_unlink(SHM_NAME);
        return -1;
    }

    ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        log_error("IPC", "Errors: mmap failed (errno %d)", errno);
        printf("Error: Failed to map POSIX shared memory (mmap failed).\n");
        close(fd);
        shm_unlink(SHM_NAME);
        return -1;
    }
    close(fd);

    /* 2. Setup Named Semaphore (initialized to 0) */
    sem_unlink(SEM_NAME); /* Ensure clean state */
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        log_error("IPC", "Errors: sem_open failed (errno %d)", errno);
        printf("Error: Failed to create POSIX named semaphore (sem_open failed).\n");
        munmap(ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);
        return -1;
    }

    /* 3. Fork Producer & Consumer Roles */
    printf("[PARENT] Coordinator process ready. Forking child role...\n");
    pid = fork();
    if (pid < 0) {
        log_error("IPC", "Errors: fork failed (errno %d)", errno);
        printf("Error: Fork failed.\n");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        munmap(ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);
        return -1;
    }

    if (pid == 0) {
        /* Child Role: Consumer */
        sem_t* child_sem;
        int child_fd;
        void* child_ptr;

        /* Sleep briefly to simulate child starting consumer routine */
        usleep(50000); /* 50ms */

        printf("[CONSUMER] Consumer process spawned (PID: %d).\n", getpid());
        log_info("IPC", "Consumer started");

        /* Open resources */
        child_sem = sem_open(SEM_NAME, 0);
        if (child_sem == SEM_FAILED) {
            log_error("IPC", "Errors: child sem_open failed (errno %d)", errno);
            exit(1);
        }

        child_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
        if (child_fd == -1) {
            log_error("IPC", "Errors: child shm_open failed (errno %d)", errno);
            sem_close(child_sem);
            exit(1);
        }

        child_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, child_fd, 0);
        if (child_ptr == MAP_FAILED) {
            log_error("IPC", "Errors: child mmap failed (errno %d)", errno);
            close(child_fd);
            sem_close(child_sem);
            exit(1);
        }
        close(child_fd);

        /* Wait for message */
        printf("[CONSUMER] Waiting for message from Producer (sem_wait)...\n");
        log_info("IPC", "Semaphore wait");
        if (sem_wait(child_sem) == -1) {
            log_error("IPC", "Errors: child sem_wait failed (errno %d)", errno);
            munmap(child_ptr, SHM_SIZE);
            sem_close(child_sem);
            exit(1);
        }

        /* Read the message */
        printf("[CONSUMER] Lock acquired. Read message: '%s'\n", (char*)child_ptr);
        log_info("IPC", "Shared memory read");

        /* Clean up */
        munmap(child_ptr, SHM_SIZE);
        sem_close(child_sem);
        printf("[CONSUMER] Consumer finished execution.\n");
        exit(0);
    } else {
        /* Parent Role: Producer */
        int status;

        printf("[PRODUCER] Parent starting Producer role (PID: %d).\n", getpid());
        log_info("IPC", "Producer started");

        /* Write data */
        const char* payload = "POSIX Shared Memory + Semaphore Integration successful!";
        snprintf((char*)ptr, SHM_SIZE, "%s", payload);
        printf("[PRODUCER] Message written to shared memory: '%s'\n", payload);
        log_info("IPC", "Shared memory written");

        /* Post semaphore to signal Consumer */
        printf("[PRODUCER] Posting semaphore (sem_post) to signal Consumer...\n");
        log_info("IPC", "Semaphore post");
        sem_post(sem);

        /* Wait for Consumer process to exit */
        waitpid(pid, &status, 0);
        printf("[PRODUCER] Consumer process completed.\n");

        /* Final Cleanup */
        printf("[PRODUCER] Cleaning up resources...\n");
        log_info("IPC", "Cleanup");

        munmap(ptr, SHM_SIZE);
        shm_unlink(SHM_NAME);
        sem_close(sem);
        sem_unlink(SEM_NAME);
        printf("[PRODUCER] Resources cleaned up cleanly.\n");
    }

    printf("--- Producer Consumer Demo Completed Successfully ---\n");
    return 0;
}
