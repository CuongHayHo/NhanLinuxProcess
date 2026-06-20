/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/history.h
 * Purpose: REPL CLI history loading and saving.
 */

#ifndef HISTORY_H
#define HISTORY_H

/**
 * Initializes history limits and loads persistent history if available.
 */
void history_init(void);

/**
 * Adds an executed command string to the in-memory history.
 */
void history_add(const char* line);

/**
 * Persists the current in-memory history back to disk.
 */
void history_save(void);

#endif /* HISTORY_H */
