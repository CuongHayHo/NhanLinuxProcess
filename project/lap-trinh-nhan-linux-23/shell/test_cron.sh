#!/bin/bash
# Copyright (c) 2026 Linux System Manager Project
# Purpose: Simple script to test cron execution by writing a timestamp.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( dirname "$DIR" )"
mkdir -p "$PROJECT_DIR/tmp"

echo "[CRON TEST] Executed successfully at $(date)" >> "$PROJECT_DIR/tmp/cron_test.log"
