#!/bin/bash

# Local vars
ARCHIVE_URL="https://51ddatafiles.blob.core.windows.net/enterpriseipi/51Degrees-LiteIpiV41.ipi.gz"
ARCHIVE_NAME="51Degrees-LiteV41.ipi.gz"
ARCHIVED_NAME="51Degrees-LiteV41.ipi"

# Default values
FORCE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -force|-f)
            FORCE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Download if forced or archive doesn't exist
if [ "$FORCE" = true ] || [ ! -f "$ARCHIVE_NAME" ]; then
    curl -o "$ARCHIVE_NAME" "$ARCHIVE_URL"
else
    echo "Archive found. Download skipped."
fi

# Compute MD5 hash with cross-platform support
if command -v md5sum >/dev/null 2>&1; then
    ARCHIVE_HASH=$(md5sum "$ARCHIVE_NAME" | awk '{ print $1 }')  # Ubuntu (Linux)
elif command -v md5 >/dev/null 2>&1; then
    ARCHIVE_HASH=$(md5 -q "$ARCHIVE_NAME")  # macOS
else
    echo "Error: No MD5 checksum tool found."
    exit 1
fi

echo "MD5 (fetched $ARCHIVE_NAME) = $ARCHIVE_HASH"

# Extract archive
echo "Extracting $ARCHIVE_NAME"
echo "Extracting '$ARCHIVE_NAME' to '$ARCHIVED_NAME'..."

# Use gunzip for decompression
gunzip -c "$ARCHIVE_NAME" > "$ARCHIVED_NAME"
