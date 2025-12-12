#!/bin/bash
#
# Install git hooks for StreamingPlayback project
#

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
HOOKS_DIR="$REPO_ROOT/.githooks"
GIT_HOOKS_DIR="$REPO_ROOT/.git/hooks"

echo "Installing git hooks..."

# Create .git/hooks directory if it doesn't exist
mkdir -p "$GIT_HOOKS_DIR"

# Install pre-commit hook
if [ -f "$HOOKS_DIR/pre-commit" ]; then
    cp "$HOOKS_DIR/pre-commit" "$GIT_HOOKS_DIR/pre-commit"
    chmod +x "$GIT_HOOKS_DIR/pre-commit"
    echo "✓ Installed pre-commit hook"
else
    echo "✗ Error: pre-commit hook not found in $HOOKS_DIR"
    exit 1
fi

# Optionally, configure git to use the hooks directory directly
# This allows hooks to be version controlled
if [ -d "$HOOKS_DIR" ]; then
    git config core.hooksPath "$HOOKS_DIR" 2>/dev/null || true
    echo "✓ Configured git to use hooks from .githooks directory"
fi

echo ""
echo "Git hooks installed successfully!"
echo ""
echo "The pre-commit hook will:"
echo "  - Check code formatting with clang-format"
echo "  - Run static analysis with clang-tidy (if compile_commands.json exists)"
echo ""
echo "To generate compile_commands.json, run:"
echo "  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .."

