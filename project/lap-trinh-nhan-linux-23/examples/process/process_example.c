/*
 * Minimal standalone example demonstrating fork() and waitpid()
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    printf("1. Calling fork()...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process
        printf("Child: Running (PID: %d, Parent PID: %d)\n", getpid(), getppid());
        printf("Child: Exiting with status 42...\n");
        exit(42);
    } else {
        // Parent process
        printf("Parent: Spawned child (PID: %d)\n", pid);
        printf("Parent: Calling waitpid() to reap child...\n");
        int status;
        pid_t reaped_pid = waitpid(pid, &status, 0);
        if (reaped_pid < 0) {
            perror("waitpid failed");
            return 1;
        }

        if (WIFEXITED(status)) {
            printf("Parent: Child PID %d exited normally with status %d.\n", 
                   reaped_pid, WEXITSTATUS(status));
        } else {
            printf("Parent: Child exited abnormally.\n");
        }
    }

    return 0;
}
