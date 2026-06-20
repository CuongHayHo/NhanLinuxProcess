#!/bin/bash
# NhanLinuxProcess - File Manager Shell backend script
# Purpose: Execute file system operations via standard Linux commands

ACTION="$1"
ARG1="$2"
ARG2="$3"

case "$ACTION" in
  create)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    touch "$ARG1"
    exit $?
    ;;

  read)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    if [ ! -f "$ARG1" ]; then
      echo "Error: File '$ARG1' does not exist."
      exit 1
    fi
    cat "$ARG1"
    exit $?
    ;;

  write)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    # ARG2 contains data. Echo it to target path
    echo -n "$ARG2" > "$ARG1"
    exit $?
    ;;

  delete)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    rm -rf "$ARG1"
    exit $?
    ;;

  rename)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty paths."
      exit 1
    fi
    mv "$ARG1" "$ARG2"
    exit $?
    ;;

  copy)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty paths."
      exit 1
    fi
    cp -r "$ARG1" "$ARG2"
    exit $?
    ;;

  move)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty paths."
      exit 1
    fi
    mv "$ARG1" "$ARG2"
    exit $?
    ;;

  info)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    stat "$ARG1"
    exit $?
    ;;

  list)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    ls -lah "$ARG1"
    exit $?
    ;;

  mkdir)
    if [ -z "$ARG1" ]; then
      echo "Error: Empty path."
      exit 1
    fi
    mkdir -p "$ARG1"
    exit $?
    ;;

  chmod)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty parameters."
      exit 1
    fi
    chmod "$ARG2" "$ARG1"
    exit $?
    ;;

  search)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty parameters."
      exit 1
    fi
    find "$ARG1" -name "$ARG2"
    exit $?
    ;;

  archive)
    if [ -z "$ARG1" ] || [ -z "$ARG2" ]; then
      echo "Error: Empty parameters."
      exit 1
    fi
    # Safely create tarball
    tar -czf "$ARG1" "$ARG2"
    exit $?
    ;;

  *)
    echo "Usage: $0 {create|read|write|delete|rename|copy|move|info|list|mkdir|chmod|search|archive} [args...]"
    exit 1
    ;;
esac