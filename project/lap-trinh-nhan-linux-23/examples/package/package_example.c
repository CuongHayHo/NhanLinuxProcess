/*
 * Minimal standalone example demonstrating read-only rpm or dpkg queries using popen()
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    const char* pm = NULL;
    if (access("/usr/bin/dpkg-query", X_OK) == 0) {
        pm = "dpkg";
    } else if (access("/usr/bin/rpm", X_OK) == 0) {
        pm = "rpm";
    }

    if (!pm) {
        printf("No supported package manager (rpm or dpkg) detected on this host.\n");
        return 0;
    }

    printf("Detected Package Manager: %s\n", pm);
    char cmd[256];
    if (strcmp(pm, "dpkg") == 0) {
        strcpy(cmd, "/usr/bin/dpkg-query -l bash 2>/dev/null");
    } else {
        strcpy(cmd, "/usr/bin/rpm -q bash 2>/dev/null");
    }

    printf("Executing read-only query command: '%s'\n", cmd);
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        perror("popen failed");
        return 1;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        printf("Output: %s", buf);
    }

    pclose(fp);
    return 0;
}
