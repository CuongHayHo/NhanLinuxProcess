/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/package/package_mgr.c
 * Purpose: Package manager enumeration and search diagnostics panel.
 */

#define _GNU_SOURCE /* Required for strdup and strcasestr */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "package_mgr.h"
#include "logger.h"

static int is_safe_package_name(const char* name) {
    if (name == NULL || strlen(name) == 0) return 0;
    for (size_t i = 0; i < strlen(name); i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '-' || c == '_' || c == '+')) {
            return 0;
        }
    }
    return 1;
}

static void trim(char* str) {
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
    size_t start = 0;
    while (isspace((unsigned char)str[start])) {
        start++;
    }
    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
}

/* Static submenu helpers */
static int read_package_choice(void) {
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

static void package_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

const char* package_mgr_detect(void) {
    if (access("/usr/bin/dpkg-query", X_OK) == 0) {
        return "dpkg";
    }
    if (access("/usr/bin/rpm", X_OK) == 0) {
        return "rpm";
    }
    return NULL;
}

void package_mgr_run(void) {
    int choice;
    log_info("PACKAGE", "Entering Package Manager");

    while (1) {
        printf("\n========================================\n");
        printf("            Package Manager\n");
        printf("========================================\n");
        printf("1. List Installed Packages\n");
        printf("2. Search Package\n");
        printf("3. Install Package (Preview)\n");
        printf("4. Remove Package (Preview)\n");
        printf("5. Package Information\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_package_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 5) {
            printf("\nInvalid input. Please choose a number between 0 and 5.\n");
            package_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("PACKAGE", "Leaving Package Manager");
            break;
        }

        if (choice >= 3 && choice <= 4) {
            printf("\nComing in a future sprint.\n");
            package_menu_pause();
            continue;
        }

        if (choice == 1) {
            package_mgr_list_installed();
            package_menu_pause();
        } else if (choice == 2) {
            char query[128];
            printf("Enter package name to search: ");
            fflush(stdout);
            if (fgets(query, sizeof(query), stdin) != NULL) {
                query[strcspn(query, "\n")] = '\0';
                package_mgr_search(query);
            }
            package_menu_pause();
        } else if (choice == 5) {
            char pkg_name[128];
            printf("Enter package name: ");
            fflush(stdout);
            if (fgets(pkg_name, sizeof(pkg_name), stdin) != NULL) {
                pkg_name[strcspn(pkg_name, "\n")] = '\0';
                package_mgr_info(pkg_name);
            }
            package_menu_pause();
        }
    }
}

int package_mgr_list_installed(void) {
    const char* pm = package_mgr_detect();
    if (!pm) {
        log_error("PACKAGE", "No supported package manager detected");
        printf("\nPackage manager not supported.\n");
        return -1;
    }

    log_info("PACKAGE", "Package manager detected: %s", pm);
    log_info("PACKAGE", "Package enumeration started");

    char cmd[256];
    if (strcmp(pm, "dpkg") == 0) {
        strcpy(cmd, "/usr/bin/dpkg-query -W -f='${Package} ${Version}\\n'");
    } else {
        strcpy(cmd, "/usr/bin/rpm -qa --qf '%{NAME} %{VERSION}\\n'");
    }

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        log_error("PACKAGE", "API failure: popen() failed to execute query cmd (errno %d)", errno);
        printf("\nError: Failed to query package database.\n");
        return -1;
    }

    printf("\n%-40s %-30s\n", "Package Name", "Version");
    printf("------------------------------------------------------------------------\n");

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        char name[128] = "";
        char version[128] = "";

        if (sscanf(line, "%127s %127s", name, version) >= 1) {
            printf("%-40s %-30s\n", name, (strlen(version) > 0) ? version : "Unknown");
            count++;
        }
    }

    printf("------------------------------------------------------------------------\n");
    printf("Total Installed Packages: %d\n", count);

    int status = pclose(fp);
    if (status == -1) {
        log_error("PACKAGE", "API failure: pclose() failed (errno %d)", errno);
    }

    log_info("PACKAGE", "Package count: %d", count);
    return 0;
}

int package_mgr_search(const char* query) {
    if (query == NULL || strlen(query) == 0) {
        log_error("PACKAGE", "Search failed: empty query");
        return -1;
    }

    log_info("PACKAGE", "Search started");
    log_info("PACKAGE", "Keyword: %s", query);

    const char* pm = package_mgr_detect();
    if (!pm) {
        log_error("PACKAGE", "No supported package manager detected");
        printf("\nPackage manager not supported.\n");
        return -1;
    }

    char cmd[256];
    if (strcmp(pm, "dpkg") == 0) {
        strcpy(cmd, "/usr/bin/dpkg-query -W -f='${Package} ${Version}\\n'");
    } else {
        strcpy(cmd, "/usr/bin/rpm -qa --qf '%{NAME} %{VERSION}\\n'");
    }

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        log_error("PACKAGE", "API failure: popen() failed to execute search cmd (errno %d)", errno);
        printf("\nError: Failed to query package database.\n");
        return -1;
    }

    printf("\nSearch Results for '%s':\n", query);
    printf("%-40s %-30s\n", "Package Name", "Version");
    printf("------------------------------------------------------------------------\n");

    char line[256];
    int match_count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        char name[128] = "";
        char version[128] = "";

        if (sscanf(line, "%127s %127s", name, version) >= 1) {
            if (strcasestr(name, query) != NULL) {
                printf("%-40s %-30s\n", name, (strlen(version) > 0) ? version : "Unknown");
                match_count++;
            }
        }
    }

    printf("------------------------------------------------------------------------\n");
    printf("Total Matches Found: %d\n", match_count);

    int status = pclose(fp);
    if (status == -1) {
        log_error("PACKAGE", "API failure: pclose() failed (errno %d)", errno);
    }

    log_info("PACKAGE", "Match count: %d", match_count);
    return 0;
}

int package_mgr_info(const char* pkg_name) {
    if (!is_safe_package_name(pkg_name)) {
        log_error("PACKAGE", "Metadata query failed: invalid/unsafe package name '%s'", pkg_name ? pkg_name : "NULL");
        printf("\nError: Invalid package name.\n");
        return -1;
    }

    log_info("PACKAGE", "Package selected: %s", pkg_name);
    log_info("PACKAGE", "Metadata query started for: %s", pkg_name);

    const char* pm = package_mgr_detect();
    if (!pm) {
        log_error("PACKAGE", "No supported package manager detected");
        printf("\nPackage manager not supported.\n");
        return -1;
    }

    char cmd[512];
    if (strcmp(pm, "dpkg") == 0) {
        snprintf(cmd, sizeof(cmd), "/usr/bin/dpkg-query -s '%s' 2>/dev/null", pkg_name);
    } else {
        snprintf(cmd, sizeof(cmd), "/usr/bin/rpm -q --qf \"Name: %%{NAME}\\nVersion: %%{VERSION}\\nRelease: %%{RELEASE}\\nArchitecture: %%{ARCH}\\nVendor: %%{VENDOR}\\nLicense: %%{LICENSE}\\nInstall Date: %%{INSTALLTIME:date}\\nSummary: %%{SUMMARY}\\nDescription: %%{DESCRIPTION}\\n\" '%s' 2>/dev/null", pkg_name);
    }

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        log_error("PACKAGE", "API failure: popen() failed for package info (errno %d)", errno);
        printf("\nError: Failed to query package database.\n");
        return -1;
    }

    char name[256] = "N/A";
    char version[256] = "N/A";
    char release[256] = "N/A";
    char arch[256] = "N/A";
    char maintainer[256] = "N/A";
    char license[256] = "N/A";
    char install_date[256] = "N/A";
    char summary[1024] = "N/A";
    char description[8192] = "";

    char line[1024];
    int in_description = 0;
    int has_data = 0;

    if (strcmp(pm, "dpkg") == 0) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            has_data = 1;
            if (line[0] == ' ' || line[0] == '\t') {
                if (in_description) {
                    strncat(description, line, sizeof(description) - strlen(description) - 1);
                }
            } else {
                in_description = 0;
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    *colon = '\0';
                    char* key = line;
                    char* val = colon + 1;
                    trim(key);
                    trim(val);
                    if (strcasecmp(key, "Package") == 0) {
                        strncpy(name, val, sizeof(name) - 1);
                    } else if (strcasecmp(key, "Version") == 0) {
                        strncpy(version, val, sizeof(version) - 1);
                    } else if (strcasecmp(key, "Architecture") == 0) {
                        strncpy(arch, val, sizeof(arch) - 1);
                    } else if (strcasecmp(key, "Maintainer") == 0) {
                        strncpy(maintainer, val, sizeof(maintainer) - 1);
                    } else if (strcasecmp(key, "Description") == 0) {
                        strncpy(summary, val, sizeof(summary) - 1);
                        strncpy(description, val, sizeof(description) - 1);
                        strncat(description, "\n", sizeof(description) - strlen(description) - 1);
                        in_description = 1;
                    }
                }
            }
        }
    } else { // rpm
        while (fgets(line, sizeof(line), fp) != NULL) {
            has_data = 1;
            if (in_description) {
                strncat(description, line, sizeof(description) - strlen(description) - 1);
            } else {
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    *colon = '\0';
                    char* key = line;
                    char* val = colon + 1;
                    trim(key);
                    trim(val);
                    if (strcasecmp(key, "Name") == 0) {
                        strncpy(name, val, sizeof(name) - 1);
                    } else if (strcasecmp(key, "Version") == 0) {
                        strncpy(version, val, sizeof(version) - 1);
                    } else if (strcasecmp(key, "Release") == 0) {
                        strncpy(release, val, sizeof(release) - 1);
                    } else if (strcasecmp(key, "Architecture") == 0) {
                        strncpy(arch, val, sizeof(arch) - 1);
                    } else if (strcasecmp(key, "Vendor") == 0) {
                        strncpy(maintainer, val, sizeof(maintainer) - 1);
                    } else if (strcasecmp(key, "License") == 0) {
                        strncpy(license, val, sizeof(license) - 1);
                    } else if (strcasecmp(key, "Install Date") == 0) {
                        strncpy(install_date, val, sizeof(install_date) - 1);
                    } else if (strcasecmp(key, "Summary") == 0) {
                        strncpy(summary, val, sizeof(summary) - 1);
                    } else if (strcasecmp(key, "Description") == 0) {
                        strncpy(description, val, sizeof(description) - 1);
                        strncat(description, "\n", sizeof(description) - strlen(description) - 1);
                        in_description = 1;
                    }
                }
            }
        }
    }

    int status = pclose(fp);
    if (!has_data || status != 0) {
        log_error("PACKAGE", "Metadata query failed for package: %s (package not found or exit status %d)", pkg_name, status);
        printf("\nError: Package '%s' is not installed or not found.\n", pkg_name);
        return -1;
    }

    printf("\n========================================================================\n");
    printf("Package Information: %s\n", name);
    printf("------------------------------------------------------------------------\n");
    printf("Name:              %s\n", name);
    printf("Version:           %s\n", version);
    printf("Release:           %s\n", release);
    printf("Architecture:      %s\n", arch);
    printf("Vendor/Maintainer: %s\n", maintainer);
    printf("License:           %s\n", license);
    printf("Install Date:      %s\n", install_date);
    printf("Summary:           %s\n", summary);
    printf("Description:\n%s", description);
    printf("========================================================================\n");

    log_info("PACKAGE", "Metadata query successful for package: %s", pkg_name);
    return 0;
}
