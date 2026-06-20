#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path_to_freebsd_src>"
    exit 1
fi

SRC_DIR="$1"
# Ensure we use an absolute path for the patch file if it's in the current directory
PATCH_FILE="$(pwd)/FreeBSD-15.0-dev.sound.diff"

if [ ! -d "$SRC_DIR" ]; then
    echo "Error: Directory '$SRC_DIR' does not exist."
    exit 1
fi

if [ ! -f "$PATCH_FILE" ]; then
    echo "Error: Patch file '$PATCH_FILE' not found."
    exit 1
fi

echo "Applying patch to $SRC_DIR..."
cd "$SRC_DIR" || exit 1
patch -p1 < "$PATCH_FILE"

if [ $? -eq 0 ]; then
    echo "Patch applied successfully."
else
    echo "Error applying patch."
    exit 1
fi
