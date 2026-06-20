#!/bin/bash
# NhanLinuxProcess - Package Manager Shell implementation
# Purpose: Auto install/uninstall package manager demonstration

log_info() {
  local msg="$1"
  local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
  echo "$timestamp [INFO] [PACKAGE] $msg" >> logs/system.log
}

log_error() {
  local msg="$1"
  local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
  echo "$timestamp [ERROR] [PACKAGE] $msg" >> logs/system.log
}

# 1. Detect package manager
if [ -f /usr/bin/dnf ] && [ -f /usr/bin/rpm ]; then
  PM="dnf"
  QUERY="rpm"
elif [ -f /usr/bin/apt-get ] && [ -f /usr/bin/dpkg-query ]; then
  PM="apt"
  QUERY="dpkg"
else
  if which dnf >/dev/null 2>&1; then
    PM="dnf"
    QUERY="rpm"
  elif which apt-get >/dev/null 2>&1; then
    PM="apt"
    QUERY="dpkg"
  else
    echo "Error: No supported package manager detected."
    log_error "No supported package manager detected"
    exit 1
  fi
fi

ACTION="$1"
PKG="$2"

case "$ACTION" in
  search)
    if [ -z "$PKG" ]; then
      echo "Error: Empty search query."
      exit 1
    fi
    if [ "$QUERY" = "rpm" ]; then
      rpm -qa --nosignature --nodigest --qf '%{NAME} %{VERSION}\n' | grep -i "$PKG"
    else
      dpkg-query -W -f='${Package} ${Version}\n' | grep -i "$PKG"
    fi
    exit 0
    ;;

  info)
    if [ -z "$PKG" ]; then
      echo "Error: Empty package name."
      exit 1
    fi
    if [ "$QUERY" = "rpm" ]; then
      if rpm -q "$PKG" >/dev/null 2>&1; then
        rpm -q --nosignature --nodigest --qf "Name: %{NAME}\nVersion: %{VERSION}\nRelease: %{RELEASE}\nArchitecture: %{ARCH}\nVendor: %{VENDOR}\nLicense: %{LICENSE}\nInstall Date: %{INSTALLTIME:date}\nSummary: %{SUMMARY}\nDescription:\n%{DESCRIPTION}\n" "$PKG"
      else
        dnf info "$PKG"
      fi
    else
      if dpkg -s "$PKG" >/dev/null 2>&1; then
        dpkg-query -s "$PKG"
      else
        apt-cache show "$PKG"
      fi
    fi
    exit $?
    ;;

  install)
    if [ -z "$PKG" ]; then
      echo "Error: Empty package name."
      exit 1
    fi
    # Prevent installing critical system packages
    case "$PKG" in
      kernel*|glibc*|bash*|systemd*|gcc*|dnf*|rpm*|python*)
        echo "Error: Installation of critical system package '$PKG' is blocked."
        log_error "Blocked installation of critical package '$PKG'"
        exit 1
        ;;
    esac

    if [ "$PM" = "dnf" ]; then
      sudo dnf install -y "$PKG"
    else
      sudo apt-get install -y "$PKG"
    fi
    exit $?
    ;;

  remove)
    if [ -z "$PKG" ]; then
      echo "Error: Empty package name."
      exit 1
    fi
    # Prevent removing critical system packages
    case "$PKG" in
      kernel*|glibc*|bash*|systemd*|gcc*|dnf*|rpm*|python*)
        echo "Error: Removal of critical system package '$PKG' is blocked."
        log_error "Blocked removal of critical package '$PKG'"
        exit 1
        ;;
    esac

    if [ "$PM" = "dnf" ]; then
      sudo dnf remove -y "$PKG"
    else
      sudo apt-get remove -y "$PKG"
    fi
    exit $?
    ;;

  demo)
    candidates=("hello" "sl" "cowsay" "figlet" "tree" "jq")
    selected=""

    for cand in "${candidates[@]}"; do
      if [ "$QUERY" = "rpm" ]; then
        if ! rpm -q "$cand" >/dev/null 2>&1; then
          selected="$cand"
          break
        fi
      else
        if ! dpkg -s "$cand" >/dev/null 2>&1; then
          selected="$cand"
          break
        fi
      fi
    done

    if [ -z "$selected" ]; then
      fallbacks=("cmatrix" "tmux" "nmap")
      for fb in "${fallbacks[@]}"; do
        if [ "$QUERY" = "rpm" ]; then
          if ! rpm -q "$fb" >/dev/null 2>&1; then
            selected="$fb"
            break
          fi
        else
          if ! dpkg -s "$fb" >/dev/null 2>&1; then
            selected="$fb"
            break
          fi
        fi
      done
    fi

    if [ -z "$selected" ]; then
      echo "Error: No suitable demo package found."
      log_error "Safe demo failed: no candidates available"
      exit 1
    fi

    echo "========================================"
    echo "Package Manager Safe Demonstration"
    echo "----------------------------------------"
    echo "Selected package:       $selected"
    echo "Safe for demonstration: YES"
    echo "========================================"
    log_info "Safe demonstration started"
    log_info "Package selected: $selected"

    # 1. Search
    echo -e "\n[STEP 1] Searching for package '$selected'..."
    if [ "$QUERY" = "rpm" ]; then
      rpm -qa | grep -i "$selected" || echo "Not currently installed (Search query ok)"
    else
      dpkg-query -W -f='${Package} ${Version}\n' | grep -i "$selected" || echo "Not currently installed (Search query ok)"
    fi

    # 2. Information
    echo -e "\n[STEP 2] Querying repository information for '$selected'..."
    if [ "$QUERY" = "rpm" ]; then
      dnf info "$selected"
    else
      apt-cache show "$selected"
    fi

    # 3. Install
    echo -e "\n[STEP 3] Installing package '$selected' (requires sudo)..."
    log_info "Package install requested: $selected"
    if [ "$PM" = "dnf" ]; then
      sudo dnf install -y "$selected"
    else
      sudo apt-get install -y "$selected"
    fi
    if [ $? -ne 0 ]; then
      echo "Error: Installation failed."
      log_error "Installation failed for $selected"
      exit 1
    fi
    log_info "Package installed"
    log_info "Package installed: $selected"

    # 4. Verify
    echo -e "\n[STEP 4] Verifying installation status..."
    installed=0
    if [ "$QUERY" = "rpm" ]; then
      if rpm -q "$selected" >/dev/null 2>&1; then
        installed=1
      fi
    else
      if dpkg-query -W -f='${Status}' "$selected" 2>/dev/null | grep -q "install ok installed"; then
        installed=1
      fi
    fi

    if [ $installed -eq 1 ]; then
      echo "Verification Success: Package '$selected' is successfully installed."
      log_info "Package verified"
      log_info "Package verified: $selected"
    else
      echo "Verification Failed: Package '$selected' was not installed."
      log_error "Verification failed for $selected"
      exit 1
    fi

    # 5. Remove
    echo -e "\n[STEP 5] Removing package '$selected' (requires sudo)..."
    log_info "Package removal requested: $selected"
    if [ "$PM" = "dnf" ]; then
      sudo dnf remove -y "$selected"
    else
      sudo apt-get remove -y "$selected"
    fi
    if [ $? -ne 0 ]; then
      echo "Error: Removal failed."
      log_error "Removal failed for $selected"
      exit 1
    fi
    log_info "Package removed"
    log_info "Package removed: $selected"

    # 6. Verify Removal
    echo -e "\n[STEP 6] Verifying removal status..."
    installed=0
    if [ "$QUERY" = "rpm" ]; then
      if rpm -q "$selected" >/dev/null 2>&1; then
        installed=1
      fi
    else
      if dpkg-query -W -f='${Status}' "$selected" 2>/dev/null | grep -q "install ok installed"; then
        installed=1
      fi
    fi

    if [ $installed -eq 0 ]; then
      echo "Verification Success: Package '$selected' has been successfully removed."
      log_info "Verification completed"
      log_info "Cleanup completed"
      echo "Final cleanup confirmation: Package '$selected' has been successfully removed and system is clean."
    else
      echo "Verification Failed: Package '$selected' was not successfully uninstalled."
      log_error "Cleanup verification failed for $selected"
      exit 1
    fi
    exit 0
    ;;

  *)
    echo "Usage: $0 {search|info|install|remove|demo} [package_name]"
    exit 1
    ;;
esac