# Linux API Reference Guide

This reference documents the system calls and POSIX APIs utilized throughout the Linux System Manager modules.

---

## 1. File APIs

### `open`
*   **Purpose**: Open or create a file.
*   **Prototype**:
    ```c
    int open(const char *pathname, int flags, mode_t mode);
    ```
*   **Typical Usage**:
    ```c
    int fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ```
*   **Related Man Page**: `open(2)`

### `read`
*   **Purpose**: Read bytes from a file descriptor.
*   **Prototype**:
    ```c
    ssize_t read(int fd, void *buf, size_t count);
    ```
*   **Typical Usage**:
    ```c
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    ```
*   **Related Man Page**: `read(2)`

### `write`
*   **Purpose**: Write bytes to a file descriptor.
*   **Prototype**:
    ```c
    ssize_t write(int fd, const void *buf, size_t count);
    ```
*   **Typical Usage**:
    ```c
    ssize_t bytes = write(fd, data, strlen(data));
    ```
*   **Related Man Page**: `write(2)`

### `close`
*   **Purpose**: Close a file descriptor.
*   **Prototype**:
    ```c
    int close(int fd);
    ```
*   **Typical Usage**:
    ```c
    close(fd);
    ```
*   **Related Man Page**: `close(2)`

### `access`
*   **Purpose**: Check accessibility of a file (e.g. existence and executable permission).
*   **Prototype**:
    ```c
    int access(const char *pathname, int mode);
    ```
*   **Typical Usage**:
    ```c
    if (access("/usr/bin/chronyc", X_OK) == 0) {
        // File exists and is executable
    }
    ```
*   **Related Man Page**: `access(2)`

### `readlink`
*   **Purpose**: Read the target of a symbolic link.
*   **Prototype**:
    ```c
    ssize_t readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz);
    ```
*   **Typical Usage**:
    ```c
    ssize_t len = readlink("/etc/localtime", tz_link, sizeof(tz_link) - 1);
    ```
*   **Related Man Page**: `readlink(2)`

---

## 2. Process APIs

### `fork`
*   **Purpose**: Create a new child process.
*   **Prototype**:
    ```c
    pid_t fork(void);
    ```
*   **Typical Usage**:
    ```c
    pid_t pid = fork();
    if (pid == 0) {
        // Child execution path
    }
    ```
*   **Related Man Page**: `fork(2)`

### `execve`
*   **Purpose**: Execute a binary program.
*   **Prototype**:
    ```c
    int execve(const char *pathname, char *const argv[], char *const envp[]);
    ```
*   **Typical Usage**:
    ```c
    char *args[] = {"/bin/ls", NULL};
    execv(args[0], args);
    ```
*   **Related Man Page**: `execve(2)`

### `execvp`
*   **Purpose**: Execute a program by filename, searching the `PATH` environment variable.
*   **Prototype**:
    ```c
    int execvp(const char *file, char *const argv[]);
    ```
*   **Typical Usage**:
    ```c
    char *args[] = {"ls", "-l", NULL};
    execvp(args[0], args);
    ```
*   **Related Man Page**: `exec(3)`

### `waitpid`
*   **Purpose**: Wait for state changes in a child process (reap zombie processes).
*   **Prototype**:
    ```c
    pid_t waitpid(pid_t pid, int *wstatus, int options);
    ```
*   **Typical Usage**:
    ```c
    int status;
    waitpid(child_pid, &status, 0);
    ```
*   **Related Man Page**: `waitpid(2)`

### `kill`
*   **Purpose**: Send a signal to a process.
*   **Prototype**:
    ```c
    int kill(pid_t pid, int sig);
    ```
*   **Typical Usage**:
    ```c
    kill(pid, SIGTERM);
    ```
*   **Related Man Page**: `kill(2)`

### `dup2`
*   **Purpose**: Duplicate an open file descriptor onto another specified descriptor (such as standard output/error).
*   **Prototype**:
    ```c
    int dup2(int oldfd, int newfd);
    ```
*   **Typical Usage**:
    ```c
    dup2(temp_fd, STDOUT_FILENO);
    dup2(temp_fd, STDERR_FILENO);
    ```
*   **Related Man Page**: `dup2(2)`

---

## 3. Network APIs

### `getifaddrs`
*   **Purpose**: Retrieve linked list of network interface addresses.
*   **Prototype**:
    ```c
    int getifaddrs(struct ifaddrs **ifap);
    ```
*   **Typical Usage**:
    ```c
    struct ifaddrs *ifaddr;
    getifaddrs(&ifaddr);
    ```
*   **Related Man Page**: `getifaddrs(3)`

### `ioctl`
*   **Purpose**: Direct device control / fetch interface attributes (e.g. MAC address, MTU).
*   **Prototype**:
    ```c
    int ioctl(int fd, unsigned long request, ...);
    ```
*   **Typical Usage**:
    ```c
    struct ifreq ifr;
    ioctl(sock_fd, SIOCGIFHWADDR, &ifr);
    ```
*   **Related Man Page**: `ioctl(2)`

### `inet_ntop`
*   **Purpose**: Convert IPv4/IPv6 network byte order addresses to human-readable text.
*   **Prototype**:
    ```c
    const char *inet_ntop(int af, const void *restrict src, char *restrict dst, socklen_t size);
    ```
*   **Typical Usage**:
    ```c
    inet_ntop(AF_INET, &sa->sin_addr, addr_str, sizeof(addr_str));
    ```
*   **Related Man Page**: `inet_ntop(3)`

---

## 4. Time APIs

### `clock_gettime`
*   **Purpose**: Fetch high-resolution clock source timestamps.
*   **Prototype**:
    ```c
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    ```
*   **Typical Usage**:
    ```c
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ```
*   **Related Man Page**: `clock_gettime(2)`

### `clock_settime`
*   **Purpose**: Set system-wide clock time.
*   **Prototype**:
    ```c
    int clock_settime(clockid_t clk_id, const struct timespec *tp);
    ```
*   **Typical Usage**:
    ```c
    struct timespec ts;
    ts.tv_sec = new_epoch_seconds;
    ts.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME, &ts);
    ```
*   **Permissions required**: `CAP_SYS_TIME` capability or superuser (root) privileges (otherwise fails with `EPERM`).
*   **Related Man Page**: `clock_settime(2)`

### `localtime_r`
*   **Purpose**: Thread-safe conversion of calendar time (`time_t`) to broken-down local representation.
*   **Prototype**:
    ```c
    struct tm *localtime_r(const time_t *restrict timep, struct tm *restrict result);
    ```
*   **Typical Usage**:
    ```c
    struct tm result;
    localtime_r(&epoch, &result);
    ```
*   **Related Man Page**: `localtime_r(3)`

### `gmtime_r`
*   **Purpose**: Thread-safe conversion of calendar time (`time_t`) to broken-down UTC representation.
*   **Prototype**:
    ```c
    struct tm *gmtime_r(const time_t *restrict timep, struct tm *restrict result);
    ```
*   **Typical Usage**:
    ```c
    struct tm result;
    gmtime_r(&epoch, &result);
    ```
*   **Related Man Page**: `gmtime_r(3)`

### `time`
*   **Purpose**: Get the current system time in seconds since the Epoch.
*   **Prototype**:
    ```c
    time_t time(time_t *tloc);
    ```
*   **Typical Usage**:
    ```c
    time_t now = time(NULL);
    ```
*   **Related Man Page**: `time(2)`

### `strftime`
*   **Purpose**: Format broken-down time into a custom date/time string representation.
*   **Prototype**:
    ```c
    size_t strftime(char *restrict s, size_t max, const char *restrict format, const struct tm *restrict tm);
    ```
*   **Typical Usage**:
    ```c
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_ptr);
    ```
*   **Related Man Page**: `strftime(3)`

---

## 5. Socket APIs

### `socket`
*   **Purpose**: Create an endpoint for communication.
*   **Prototype**:
    ```c
    int socket(int domain, int type, int protocol);
    ```
*   **Typical Usage**:
    ```c
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ```
*   **Related Man Page**: `socket(2)`

### `bind`
*   **Purpose**: Bind a name (address) to a socket.
*   **Prototype**:
    ```c
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```
*   **Typical Usage**:
    ```c
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    ```
*   **Related Man Page**: `bind(2)`

### `listen`
*   **Purpose**: Listen for connections on a socket.
*   **Prototype**:
    ```c
    int listen(int sockfd, int backlog);
    ```
*   **Typical Usage**:
    ```c
    listen(sockfd, 5);
    ```
*   **Related Man Page**: `listen(2)`

### `accept`
*   **Purpose**: Accept a connection on a socket.
*   **Prototype**:
    ```c
    int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen);
    ```
*   **Typical Usage**:
    ```c
    int client_fd = accept(sockfd, (struct sockaddr *)&addr, &addrlen);
    ```
*   **Related Man Page**: `accept(2)`

### `connect`
*   **Purpose**: Initiate a connection on a socket.
*   **Prototype**:
    ```c
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```
*   **Typical Usage**:
    ```c
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    ```
*   **Related Man Page**: `connect(2)`

### `send`
*   **Purpose**: Send a message on a socket.
*   **Prototype**:
    ```c
    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    ```
*   **Typical Usage**:
    ```c
    send(client_fd, message, strlen(message), 0);
    ```
*   **Related Man Page**: `send(2)`

### `recv`
*   **Purpose**: Receive a message from a socket.
*   **Prototype**:
    ```c
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    ```
*   **Typical Usage**:
    ```c
    recv(client_fd, buffer, sizeof(buffer), 0);
    ```
*   **Related Man Page**: `recv(2)`


