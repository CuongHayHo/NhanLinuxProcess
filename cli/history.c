/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/history.c
 * Purpose: REPL CLI history loading and saving implementation.
 */

#include "history.h"
#include "linenoise.h"

#define HISTORY_FILE ".sysmgr_history"
#define HISTORY_MAX_LEN 100

void history_init(void) {
    linenoiseHistorySetMaxLen(HISTORY_MAX_LEN);
    linenoiseHistoryLoad(HISTORY_FILE);
}

void history_add(const char* line) {
    if (line && line[0] != '\0') {
        linenoiseHistoryAdd(line);
    }
}

void history_save(void) {
    linenoiseHistorySave(HISTORY_FILE);
}
