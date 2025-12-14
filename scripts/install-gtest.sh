#!/bin/bash
#
# Install Google Test for testing
# This script helps install GTest on different platforms
#

set -e

OS="$(uname -s)"

echo "Installing Google Test..."

case "$OS" in
    Darwin)
        echo "Detected macOS"
        if command -v brew &> /dev/null; then
            echo "Installing via Homebrew..."
            brew install googletest
            echo "✓ GTest installed successfully"
        else
            echo "Error: Homebrew not found. Please install Homebrew first:"
            echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
        ;;
    Linux)
        echo "Detected Linux"
        if command -v apt-get &> /dev/null; then
            echo "Installing via apt-get..."
            sudo apt-get update
            sudo apt-get install -y libgtest-dev
            echo "✓ GTest installed successfully"
        elif command -v yum &> /dev/null; then
            echo "Installing via yum..."
            sudo yum install -y gtest-devel
            echo "✓ GTest installed successfully"
        else
            echo "Error: Package manager not found. Please install libgtest-dev manually."
            exit 1
        fi
        ;;
    *)
        echo "Error: Unsupported OS: $OS"
        echo "Please install GTest manually or set GTEST_ROOT environment variable"
        exit 1
        ;;
esac

echo ""
echo "GTest installation complete!"
echo "You can now build with tests: cmake .. -DBUILD_TESTS=ON"

