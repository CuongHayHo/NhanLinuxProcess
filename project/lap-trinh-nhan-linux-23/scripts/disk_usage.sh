#!/bin/bash
# Educational Disk Usage Script
echo "=== Disk Usage Analyzer ==="
echo "Displaying root partition usage stats:"
df -h /
echo "Checking largest files in /var/log (top 5):"
find /var/log -type f -exec du -sh {} + 2>/dev/null | sort -rh | head -n 5
exit 0
