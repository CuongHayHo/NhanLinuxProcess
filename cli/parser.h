/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/parser.h
 * Purpose: Command line tokenizer.
 */

#ifndef PARSER_H
#define PARSER_H

/**
 * Tokenizes a command line string into argv array.
 * Supports double quotes for arguments containing spaces.
 * Returns the number of tokens parsed.
 */
int parser_tokenize(const char* line, char** argv, int max_args);

/**
 * Safely frees the allocated token strings returned by parser_tokenize.
 */
void parser_free_tokens(char** argv, int argc);

#endif /* PARSER_H */
