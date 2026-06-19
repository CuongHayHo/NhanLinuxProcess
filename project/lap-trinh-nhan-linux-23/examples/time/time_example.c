/*
 * Minimal standalone example demonstrating clock_gettime()
 */
#include <stdio.h>
#include <time.h>

int main(void) {
    struct timespec ts;

    printf("1. Calling clock_gettime(CLOCK_REALTIME)...\n");
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        perror("clock_gettime CLOCK_REALTIME failed");
        return 1;
    }
    printf("CLOCK_REALTIME: Epoch Seconds: %ld, Nanoseconds: %ld\n", 
           ts.tv_sec, ts.tv_nsec);

    printf("2. Calling clock_gettime(CLOCK_MONOTONIC)...\n");
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime CLOCK_MONOTONIC failed");
        return 1;
    }
    printf("CLOCK_MONOTONIC (uptime reference): Seconds: %ld, Nanoseconds: %ld\n", 
           ts.tv_sec, ts.tv_nsec);

    return 0;
}
