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
    CURRENT_VERSION=$("$BINARY_NAME" --version | sed 's/^v//')
    echo "Found local installation: v$CURRENT_VERSION"
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

    # Extract the download link for the Linux binary
    DOWNLOAD_URL=$(echo "$LATEST_RELEASE_JSON" | grep -o -E '"browser_download_url": "[^"]+linux[^"]*"' | head -n 1 | cut -d'"' -f4)

    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Error: Could not find a suitable Linux release binary on GitHub."
        exit 1
    fi

    # Download to temporary location
    TEMP_FILE=$(mktemp)
    curl -L -o "$TEMP_FILE" "$DOWNLOAD_URL"

    # Remove old binary
    if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
        rm -f "$INSTALL_DIR/$BINARY_NAME"
    fi

    # Move new binary into place & make it executable
    mv "$TEMP_FILE" "$INSTALL_DIR/$BINARY_NAME"
    chmod +x "$INSTALL_DIR/$BINARY_NAME"

    echo -e "\033[32mSuccessfully installed Helios v$LATEST_VERSION!\033[0m"
else
    echo "Helios is already up to date."
fi
