/*
 * Minimal standalone example demonstrating open(), read(), and write()
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void) {
    const char* filename = "example_temp.txt";
    const char* data = "Hello from Linux File API!\n";
    char buffer[128];

    printf("1. Opening file '%s' for writing (O_WRONLY|O_CREAT|O_TRUNC)...\n", filename);
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open for write failed");
        return 1;
    }

    printf("2. Writing data to file...\n");
    ssize_t bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("write failed");
        close(fd);
        return 1;
    }
    close(fd);

    printf("3. Opening file '%s' for reading (O_RDONLY)...\n", filename);
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open for read failed");
        return 1;
    }

    printf("4. Reading data from file...\n");
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("read failed");
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';
    close(fd);

    printf("Read data: %s", buffer);

    printf("5. Cleaning up temp file...\n");
    unlink(filename);

    return 0;
}
