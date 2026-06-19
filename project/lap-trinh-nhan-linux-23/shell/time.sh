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

internetTimeSync() {
    clear
    echo "--- Internet Time Synchronization ---"
    
    # 1. Check network connectivity
    ping -c 1 -W 2 pool.ntp.org > /dev/null 2>&1
    local has_net=$?
    
    local backend=""
    if command -v chronyc &> /dev/null; then
        backend="chronyc"
        echo "Detecting synchronization backend: chronyc"
        echo "Executing synchronization request..."
        sudo chronyc makestep > /dev/null 2>&1
    elif command -v ntpdate &> /dev/null; then
        backend="ntpdate"
        echo "Detecting synchronization backend: ntpdate"
        echo "Executing synchronization request..."
        sudo ntpdate pool.ntp.org > /dev/null 2>&1
    else
        echo "Immediate Internet synchronization is not supported on this system."
        read -p "Press ENTER to continue..."
        return
    fi
    
    # 2. Wait briefly
    sleep 2
    
    # 3. Verify status
    local is_synchronized=0
    if [ $has_net -eq 0 ]; then
        if [ "$backend" = "chronyc" ]; then
            chronyc tracking | grep -q -E "Reference ID.*(0\.0\.0\.0|Not synchronised)"
            if [ $? -ne 0 ]; then
                is_synchronized=1
            fi
        else
            is_synchronized=1
        fi
    fi
    
    # 4. Display result
    if [ $is_synchronized -eq 1 ]; then
        echo "Synchronization completed."
        echo "Current Time"
        date "+%Y-%m-%d %H:%M:%S"
        echo "Time Zone"
        cat /etc/timezone 2>/dev/null || date +%Z
        echo "Synchronization Source"
        echo "$backend"
    else
        echo "Synchronization request sent."
        echo "Verification failed."
        echo "Current Time"
        date "+%Y-%m-%d %H:%M:%S"
        echo "Status"
        echo "NOT synchronized"
        echo "Possible Reasons"
        echo "• No Internet connection"
        echo "• NTP server unreachable"
        echo "• chronyd has not synchronized yet"
        echo "• Permission denied"
    fi
    read -p "Press ENTER to continue..."
}

enableAutoSync() {
    clear
    echo "--- Enable Automatic Time Synchronization (NTP) ---"
    if command -v timedatectl &> /dev/null; then
        sudo timedatectl set-ntp true
        if [ $? -eq 0 ]; then
            echo "Automatic synchronization enabled."
            timedatectl status
        else
            echo "Failed to enable NTP synchronization."
        fi
    else
        echo "timedatectl not supported on this system."
    fi
    read -p "Press ENTER to continue..."
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
        echo "4. Internet Time Synchronization"
        echo "5. Enable Automatic Time Synchronization (NTP)"
        echo "0. Return"
        echo "========================================"
        read -p "Select option: " option
        case "$option" in
            1) showCurrentTime ;;
            2) showTimeZone ;;
            3) setTimeManually ;;
            4) internetTimeSync ;;
            5) enableAutoSync ;;
            0) exit 0 ;;
            *)
                echo "Invalid input. Please choose a number between 0 and 5."
                read -p "Press ENTER to continue..."
                ;;
        esac
    done
}

showMenu
