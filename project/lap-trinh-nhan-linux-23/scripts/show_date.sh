#!/bin/bash
# Educational Uptime and Date Script
echo "=== System Date & Uptime Info ==="
echo "Current system date/time: $(date)"
echo "Current system timezone: $(date +%Z)"
echo "System uptime duration: $(uptime -p)"
exit 0
