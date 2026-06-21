#!/bin/bash

# Auto-recovery Time Synchronization Script
# Resolves the DNSSEC / NTP chicken-and-egg problem by fetching approximate GMT time
# from direct IP addresses via HTTP headers, then synchronizing with NTP.

echo "=== System Time Auto-Recovery & Synchronization ==="

# List of reliable public IPs to query HTTP Date header (bypassing DNS and SSL)
IPs=(
    "1.1.1.1"          # Cloudflare
    "8.8.8.8"          # Google
    "9.9.9.9"          # Quad9
    "208.67.222.222"   # OpenDNS
)

HTTP_DATE=""

echo "Attempting to retrieve approximate current time from public servers..."
for ip in "${IPs[@]}"; do
    echo "Querying http://$ip..."
    # Query HEAD header from IP address over HTTP port 80
    header_date=$(curl -I -s --connect-timeout 3 "http://$ip" | grep -i "^date:" | cut -d' ' -f2- | tr -d '\r')
    if [ -n "$header_date" ]; then
        # Check if the date string is valid by testing it with date -d
        if date -d "$header_date" >/dev/null 2>&1; then
            HTTP_DATE="$header_date"
            echo "Successfully retrieved date from $ip: $HTTP_DATE"
            break
        fi
    fi
done

if [ -z "$HTTP_DATE" ]; then
    echo "ERROR: Failed to retrieve approximate time from any public servers."
    echo "Please check your internet connection or proxy settings."
    exit 1
fi

# Set system time to the approximate HTTP time
echo "Setting system time to approximate time: $HTTP_DATE..."
if ! sudo date -s "$HTTP_DATE" >/dev/null 2>&1; then
    echo "ERROR: Failed to set system time. Do you have sudo privileges?"
    exit 1
fi

echo "System time updated to approximate time. Waiting for DNS resolver to update..."
sleep 1.5

# Now that time is approximately correct, DNSSEC validation will pass.
# Attempt high-accuracy synchronization using standard NTP utilities.
echo "Performing high-accuracy synchronization with pool.ntp.org..."

if command -v chronyc &> /dev/null; then
    echo "Using chrony for synchronization..."
    sudo chronyc makestep >/dev/null 2>&1
    sleep 1
    # Verify chrony status
    chronyc tracking | grep -q -E "Reference ID.*(0\.0\.0\.0|Not synchronised)"
    if [ $? -ne 0 ]; then
        echo "SUCCESS: Time synchronized successfully via chrony."
        date
        exit 0
    fi
fi

if command -v ntpdate &> /dev/null; then
    echo "Using ntpdate for synchronization..."
    if sudo ntpdate pool.ntp.org >/dev/null 2>&1; then
        echo "SUCCESS: Time synchronized successfully via ntpdate."
        date
        exit 0
    fi
fi

if command -v timedatectl &> /dev/null; then
    echo "Using timedatectl set-ntp true..."
    sudo timedatectl set-ntp false
    sudo timedatectl set-ntp true
    sleep 2
    if timedatectl status | grep -q "System clock synchronized: yes"; then
        echo "SUCCESS: Time synchronized successfully via timedatectl."
        date
        exit 0
    fi
fi

# Fallback: check if the time is at least reasonably correct now
current_year=$(date +%Y)
if [ "$current_year" -ge 2026 ]; then
    echo "System year is $current_year. Synchronization is approximately correct."
    exit 0
else
    echo "WARNING: NTP synchronization failed, but time was set to HTTP date."
    exit 2
fi
