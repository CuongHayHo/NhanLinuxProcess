/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/parser.c
 * Purpose: Command line parser/tokenizer implementation.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "parser.h"

int parser_tokenize(const char* line, char** argv, int max_args) {
    int argc = 0;
    int in_quotes = 0;
    char* copy = strdup(line);
    if (!copy) return 0;

    char* p = copy;
    char* arg_start = NULL;

    while (*p && argc < max_args) {
        if (*p == '"') {
            if (in_quotes) {
                /* End of quoted argument */
                *p = '\0';
                if (arg_start) {
                    argv[argc++] = strdup(arg_start);
                    arg_start = NULL;
                } else {
                    argv[argc++] = strdup("");
                }
                in_quotes = 0;
            } else {
                /* Start of quoted argument */
                in_quotes = 1;
                arg_start = p + 1;
            }
        } else if (isspace((unsigned char)*p)) {
            if (in_quotes) {
                /* Space inside quotes is preserved */
            } else {
                /* Space outside quotes splits argument */
                if (arg_start) {
                    *p = '\0';
                    argv[argc++] = strdup(arg_start);
                    arg_start = NULL;
                }
            }
        } else {
            if (!arg_start) {
                arg_start = p;
            }
        }
        p++;
    }

    if (arg_start && argc < max_args) {
        argv[argc++] = strdup(arg_start);
    }

    free(copy);
    return argc;
}

void parser_free_tokens(char** argv, int argc) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
        argv[i] = NULL;
    }
}
