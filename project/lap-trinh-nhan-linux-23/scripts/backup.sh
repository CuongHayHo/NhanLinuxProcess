#!/bin/bash
# Educational Read-Only Backup Script
echo "=== Educational System Backup (Mock) ==="
echo "Checking configuration files in /etc..."
if [ -d "/etc" ]; then
    echo "[SUCCESS] /etc directory exists."
    echo "[INFO] Simulated target: /backup/sysconfig_backup.tar.gz"
    echo "[INFO] Backup size estimation: $(du -sh /etc 2>/dev/null | cut -f1)"
else
    echo "[ERROR] /etc directory not found."
    exit 1
fi
echo "[SUCCESS] Backup simulation finished successfully."
exit 0
