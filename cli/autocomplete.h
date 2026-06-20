/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/autocomplete.h
 * Purpose: Tab autocompleter callbacks.
 */

#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

/**
 * Initializes and registers the autocompletion callbacks with linenoise.
 */
void autocomplete_init(void);

/**
 * Sets whether autocomplete should be active (only for main command REPL prompts).
 * 1 = active (command prompt), 0 = inactive (parameter prompt).
 */
void autocomplete_set_command_mode(int mode);

#endif /* AUTOCOMPLETE_H */
