#!/usr/bin/env bash

set -e

# Configuration (change this when publishing to your repository)
REPO_OWNER="event-h0r1zon"
REPO_NAME="Helios"
BINARY_NAME="helios"

# 1. Determine target directory
if [ "$EUID" -ne 0 ]; then
    INSTALL_DIR="$HOME/.local/bin"
    mkdir -p "$INSTALL_DIR"
    # Warn user if local bin isn't in their PATH
    if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
        echo -e "\n\033[33mWarning: $INSTALL_DIR is not in your PATH.\033[0m"
        echo "Please add the following line to your ~/.bashrc or ~/.zshrc:"
        echo "export PATH=\"\$PATH:\$HOME/.local/bin\""
    fi
else
    INSTALL_DIR="/usr/local/bin"
fi

# 2. Check currently installed version
CURRENT_VERSION="0.0.0"
if command -v "$BINARY_NAME" &> /dev/null; then
    # Run version check safely in case the installed binary is corrupted
    if CURRENT_VERSION=$("$BINARY_NAME" --version 2>/dev/null | sed 's/^v//') && [ -n "$CURRENT_VERSION" ]; then
        echo "Found local installation: v$CURRENT_VERSION"
    else
        CURRENT_VERSION="0.0.0"
        echo "Warning: Local installation is corrupted or unexecutable. Overwriting..."
    fi
fi

# 3. Retrieve latest release info from GitHub API
echo "Querying GitHub for the latest version..."
LATEST_RELEASE_JSON=$(curl -s "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/latest")
LATEST_VERSION=$(echo "$LATEST_RELEASE_JSON" | grep '"tag_name":' | sed -E 's/.*"v?([^"]+)".*/\1/')

if [ -z "$LATEST_VERSION" ]; then
    echo "Error: Could not retrieve latest version info from GitHub."
    exit 1
fi

echo "Latest version available: v$LATEST_VERSION"

# Version comparison function (returns true if $1 > $2)
version_gt() {
    test "$(printf '%s\n' "$@" | sort -V | head -n 1)" != "$1";
}

# 4. Install or upgrade if remote version is newer
if ! command -v "$BINARY_NAME" &> /dev/null || version_gt "$LATEST_VERSION" "$CURRENT_VERSION"; then
    echo "Downloading and installing v$LATEST_VERSION..."

    # Extract the download link for the Linux binary tarball
    DOWNLOAD_URL=$(echo "$LATEST_RELEASE_JSON" | grep -o -E '"browser_download_url": "[^"]+linux[^"]+\.tar\.gz"' | head -n 1 | cut -d'"' -f4)

    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Error: Could not find a suitable Linux release binary tarball on GitHub."
        exit 1
    fi

    # Create temporary file and directory
    TEMP_TAR=$(mktemp)
    TEMP_DIR=$(mktemp -d)

    # Download tarball
    curl -L -o "$TEMP_TAR" "$DOWNLOAD_URL"

    # Extract tarball
    tar -xzf "$TEMP_TAR" -C "$TEMP_DIR"

    # Remove old binary
    if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
        rm -f "$INSTALL_DIR/$BINARY_NAME"
    fi

    # Copy new binary into place & make it executable
    cp "$TEMP_DIR/helios" "$INSTALL_DIR/$BINARY_NAME"
    chmod +x "$INSTALL_DIR/$BINARY_NAME"

    # Determine standard share directory for assets
    if [ "$INSTALL_DIR" = "/usr/local/bin" ]; then
        SHARE_DIR="/usr/local/share/helios/assets"
    else
        SHARE_DIR="$HOME/.local/share/helios/assets"
    fi

    # Recreate target directory and copy assets
    rm -rf "$SHARE_DIR"
    mkdir -p "$SHARE_DIR"
    cp -r "$TEMP_DIR/assets/"* "$SHARE_DIR/"

    # Cleanup temporary locations
    rm -f "$TEMP_TAR"
    rm -rf "$TEMP_DIR"

    echo -e "\033[32mSuccessfully installed Helios v$LATEST_VERSION!\033[0m"
else
    echo "Helios is already up to date."
fi
