#!/bin/bash
# Copyright (c) 2026 Linux System Manager Project
# All rights reserved.
#
# File: shell/time.sh
# Purpose: System time configuration and synchronization in Bash.

showCurrentTime() {
    clear
    echo "--- Thiết lập thời gian - Xem thời gian hiện tại ---"
    date
}

showTimeZone() {
    clear
    echo "--- Thiết lập thời gian - Xem múi giờ ---"
    if command -v timedatectl &> /dev/null; then
        timedatectl | grep "Time zone"
    else
        echo "Múi giờ hiện tại: $(cat /etc/timezone 2>/dev/null || date +%Z)"
    fi
}

validate_datetime() {
    local dt="$1"
    # Matches YYYY-MM-DD HH:MM:SS
    if [[ "$dt" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}\ [0-9]{2}:[0-9]{2}:[0-9]{2}$ ]]; then
        return 0
    else
        return 1
    fi
}

setTimeManually() {
    while true; do
        clear
        echo "========================================"
        echo "           Set Time Manually"
        echo "========================================"
        echo "1. Learning Mode"
        echo "2. Apply System Time"
        echo "0. Return"
        echo "========================================"
        read -p "Select option: " sub_choice
        case "$sub_choice" in
            1)
                read -p "Enter YYYY-MM-DD HH:MM:SS: " datetime
                if validate_datetime "$datetime"; then
                    echo "Format validated successfully."
                    echo "[LEARNING MODE] Equivalent command:"
                    echo "  date -s \"$datetime\""
                    echo "Equivalent POSIX API:"
                    echo "  clock_settime()"
                else
                    echo "Error: Invalid date/time format."
                fi
                read -p "Press ENTER to continue..."
                ;;
            2)
                read -p "Enter YYYY-MM-DD HH:MM:SS: " datetime
                if validate_datetime "$datetime"; then
                    sudo date -s "$datetime"
                    if [ $? -eq 0 ]; then
                        echo "System time successfully changed."
                    else
                        echo "Error: clock_settime failed (Root privileges required)."
                    fi
                else
                    echo "Error: Invalid date/time format."
                fi
                read -p "Press ENTER to continue..."
                ;;
            0)
                break
                ;;
            *)
                echo "Invalid input. Please choose a number between 0 and 2."
                read -p "Press ENTER to continue..."
                ;;
        esac
    done
}

showMenu() {
    while true; do
        clear
        echo "========================================"
        echo "           Time Configuration"
        echo "========================================"
        echo "1. Show Current Time"
        echo "2. Show Time Zone"
        echo "3. Set Time Manually"
        echo "4. Internet Time Synchronization (Auto-recovery)"
        echo "0. Return"
        echo "========================================"
        read -p "Select option: " option
        case "$option" in
            1) showCurrentTime ;;
            2) showTimeZone ;;
            3) setTimeManually ;;
            4) "$(dirname "$0")/autosync.sh" ;;
            0) exit 0 ;;
            *)
                echo "Invalid input. Please choose a number between 0 and 4."
                read -p "Press ENTER to continue..."
                ;;
        esac
    done
}

showMenu
