/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/autocomplete.c
 * Purpose: Interactive CLI autocomplete logic.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "autocomplete.h"
#include "linenoise.h"
#include "palette.h"

/* Extern declaration of REPL context query function */
extern const char* repl_get_current_context(void);

/* Helper to check if string starts with prefix */
static int starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

/* Autocomplete callback */
static void completion_callback(const char *buf, linenoiseCompletions *lc) {
    const char* ctx = repl_get_current_context();
    size_t len = strlen(buf);

    /* If context is main */
    if (strcmp(ctx, "main") == 0) {
        /* Check if starting with "/" */
        if (len > 0 && buf[0] == '/') {
            const char* prefix = buf + 1;
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, "main") == 0) {
                    if (starts_with(item->command, prefix)) {
                        char suggestion[256];
                        snprintf(suggestion, sizeof(suggestion), "/%s", item->command);
                        linenoiseAddCompletion(lc, suggestion);
                    }
                }
            }
            return;
        }

        /* Check direct multi-level commands: "package", "file", etc. */
        char buf_copy[512];
        strncpy(buf_copy, buf, sizeof(buf_copy) - 1);
        buf_copy[sizeof(buf_copy) - 1] = '\0';

        char* words[16];
        int word_count = 0;
        char* token = strtok(buf_copy, " ");
        while (token != NULL && word_count < 16) {
            words[word_count++] = token;
            token = strtok(NULL, " ");
        }

        int ends_with_space = (len > 0 && buf[len - 1] == ' ');

        if (word_count == 0) {
            /* Suggest main contexts */
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, "main") == 0) {
                    linenoiseAddCompletion(lc, item->command);
                }
            }
        } else if (word_count == 1 && !ends_with_space) {
            /* Autocomplete first word main command */
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, "main") == 0) {
                    if (starts_with(item->command, words[0])) {
                        linenoiseAddCompletion(lc, item->command);
                    }
                }
            }
        } else if (word_count == 1 && ends_with_space) {
            /* First word is fully matched main context, suggest its subcommands */
            const char* target_ctx = words[0];
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, target_ctx) == 0) {
                    char suggestion[256];
                    snprintf(suggestion, sizeof(suggestion), "%s %s", words[0], item->command);
                    linenoiseAddCompletion(lc, suggestion);
                }
            }
        } else if (word_count == 2 && !ends_with_space) {
            /* Autocomplete second word subcommand of main context */
            const char* target_ctx = words[0];
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, target_ctx) == 0) {
                    if (starts_with(item->command, words[1])) {
                        char suggestion[256];
                        snprintf(suggestion, sizeof(suggestion), "%s %s", words[0], item->command);
                        linenoiseAddCompletion(lc, suggestion);
                    }
                }
            }
        } else if (word_count == 2 && ends_with_space) {
            /* Suggest options / templates for subcommands */
            if (strcmp(words[0], "package") == 0 && (strcmp(words[1], "search") == 0 || strcmp(words[1], "info") == 0 || strcmp(words[1], "install") == 0 || strcmp(words[1], "remove") == 0)) {
                char suggestion[256];
                snprintf(suggestion, sizeof(suggestion), "%s %s hello", words[0], words[1]);
                linenoiseAddCompletion(lc, suggestion);
                snprintf(suggestion, sizeof(suggestion), "%s %s sl", words[0], words[1]);
                linenoiseAddCompletion(lc, suggestion);
                snprintf(suggestion, sizeof(suggestion), "%s %s cowsay", words[0], words[1]);
                linenoiseAddCompletion(lc, suggestion);
            }
        }
    } else {
        /* In sub-context (e.g. "package", "file", etc.) */
        if (len > 0 && buf[0] == '/') {
            const char* prefix = buf + 1;
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, ctx) == 0) {
                    if (starts_with(item->command, prefix)) {
                        char suggestion[256];
                        snprintf(suggestion, sizeof(suggestion), "/%s", item->command);
                        linenoiseAddCompletion(lc, suggestion);
                    }
                }
            }
            return;
        }

        char buf_copy[512];
        strncpy(buf_copy, buf, sizeof(buf_copy) - 1);
        buf_copy[sizeof(buf_copy) - 1] = '\0';

        char* words[16];
        int word_count = 0;
        char* token = strtok(buf_copy, " ");
        while (token != NULL && word_count < 16) {
            words[word_count++] = token;
            token = strtok(NULL, " ");
        }

        int ends_with_space = (len > 0 && buf[len - 1] == ' ');

        if (word_count == 0) {
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, ctx) == 0) {
                    linenoiseAddCompletion(lc, item->command);
                }
            }
        } else if (word_count == 1 && !ends_with_space) {
            for (int i = 0; i < palette_items_count; i++) {
                const palette_item_t* item = &palette_items[i];
                if (strcmp(item->context, ctx) == 0) {
                    if (starts_with(item->command, words[0])) {
                        linenoiseAddCompletion(lc, item->command);
                    }
                }
            }
        } else if (word_count == 1 && ends_with_space) {
            /* Package context suggests package templates */
            if (strcmp(ctx, "package") == 0 && (strcmp(words[0], "search") == 0 || strcmp(words[0], "info") == 0 || strcmp(words[0], "install") == 0 || strcmp(words[0], "remove") == 0)) {
                char suggestion[256];
                snprintf(suggestion, sizeof(suggestion), "%s hello", words[0]);
                linenoiseAddCompletion(lc, suggestion);
                snprintf(suggestion, sizeof(suggestion), "%s sl", words[0]);
                linenoiseAddCompletion(lc, suggestion);
                snprintf(suggestion, sizeof(suggestion), "%s cowsay", words[0]);
                linenoiseAddCompletion(lc, suggestion);
            }
        }
    }
}

void autocomplete_init(void) {
    linenoiseSetCompletionCallback(completion_callback);
}
