#!/bin/bash
# Description: Displays a hello popup/notification on the user's desktop screen.

# Set DISPLAY if not set (default to :0)
export DISPLAY="${DISPLAY:-:0}"

# Detect current user ID to dynamically construct DBUS session path
USER_ID=$(id -u)
export DBUS_SESSION_BUS_ADDRESS="${DBUS_SESSION_BUS_ADDRESS:-unix:path=/run/user/${USER_ID}/bus}"

# Try using notify-send (non-blocking notification toast)
if command -v notify-send &> /dev/null; then
    notify-send "Hệ thống thông báo" "Xin chào! Đây là thông báo từ tác vụ lập lịch định kỳ."
    echo "Notification sent via notify-send."
# Fallback to zenity (blocking dialog box)
elif command -v zenity &> /dev/null; then
    zenity --info --text="Xin chào! Đây là thông báo từ tác vụ lập lịch định kỳ."
    echo "Popup displayed via zenity."
else
    echo "No notification utility (notify-send or zenity) found on system."
    exit 1
fi

exit 0
