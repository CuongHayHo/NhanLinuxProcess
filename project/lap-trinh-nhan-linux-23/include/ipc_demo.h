/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/ipc_demo.h
 * Purpose: Header declarations for optional IPC demonstration programs.
 */

#ifndef IPC_DEMO_H
#define IPC_DEMO_H

#include <sys/types.h>
#include <semaphore.h>

/* ==========================================
 * 1. Shared Memory IPC
 * ========================================== */

/**
 * @brief Create a shared memory segment.
 * @return Segment ID, or -1 on error.
 */
int shm_demo_create(key_t key, size_t size);

/**
 * @brief Attach shared memory segment to process virtual address.
 * @return Pointer to mapped region, or (void*)-1 on error.
 */
void* shm_demo_attach(int shmid);

/**
 * @brief Detach shared memory from address space.
 * @return 0 on success, -1 on error.
 */
int shm_demo_detach(const void* addr);


/* ==========================================
 * 2. Semaphore Synchronization IPC
 * ========================================== */

/**
 * @brief Initialize a named POSIX semaphore.
 * @return Named semaphore descriptor, or SEM_FAILED on error.
 */
sem_t* sem_demo_init(const char* name, int value);

/**
 * @brief Acquire/lock the semaphore (blocking).
 * @return 0 on success, -1 on error.
 */
int sem_demo_wait(sem_t* sem);

/**
 * @brief Release/unlock the semaphore.
 * @return 0 on success, -1 on error.
 */
int sem_demo_post(sem_t* sem);


/* ==========================================
 * 3. Named Pipe (FIFO) IPC
 * ========================================== */

/**
 * @brief Create a named pipe file system node (FIFO).
 * @return 0 on success, -1 on error.
 */
int fifo_demo_create(const char* name);

/**
 * @brief Write data to named pipe.
 * @return Number of bytes written, or -1 on error.
 */
int fifo_demo_write(const char* name, const char* data);

/**
 * @brief Read data from named pipe.
 * @return Number of bytes read, or -1 on error.
 */
int fifo_demo_read(const char* name, char* buffer, size_t size);


/* ==========================================
 * 4. Message Queue IPC
 * ========================================== */

/**
 * @brief Initialize a message queue descriptor.
 * @return Queue ID, or -1 on error.
 */
int msgq_demo_init(key_t key);

/**
 * @brief Push a message into the queue.
 * @return 0 on success, -1 on error.
 */
int msgq_demo_send(int msqid, long type, const char* text);

/**
 * @brief Retrieve a message from the queue.
 * @return Bytes read, or -1 on error.
 */
int msgq_demo_receive(int msqid, long type, char* buffer, size_t size);

#endif /* IPC_DEMO_H */
