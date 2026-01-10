#!/bin/bash
# Eon Project - macOS Build Script
# Builds the game for Apple Silicon Mac

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CLIENT_DIR="$PROJECT_ROOT/Client"
UE_ROOT="/Users/Shared/Epic Games/UE_5.5"
UAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"

BUILD_CONFIG="${1:-Development}"  # Development, Shipping, DebugGame
OUTPUT_DIR="$PROJECT_ROOT/Builds/Mac"

echo "=========================================="
echo "Eon Project - macOS Build"
echo "=========================================="
echo "Configuration: $BUILD_CONFIG"
echo "Output: $OUTPUT_DIR"
echo ""

# Check UE5 installation
if [ ! -f "$UAT" ]; then
    echo "[ERROR] Unreal Engine not found at $UE_ROOT"
    echo "Please install UE 5.5.4 via Epic Games Launcher"
    exit 1
fi

# Check project file
if [ ! -f "$CLIENT_DIR/Eon.uproject" ]; then
    echo "[ERROR] Project file not found: $CLIENT_DIR/Eon.uproject"
    exit 1
fi

# Clean previous builds
echo "[INFO] Cleaning previous builds..."
rm -rf "$OUTPUT_DIR" 2>/dev/null || true
mkdir -p "$OUTPUT_DIR"

# Build command
echo "[INFO] Starting build (this may take a while)..."
"$UAT" BuildCookRun \
    -project="$CLIENT_DIR/Eon.uproject" \
    -noP4 \
    -platform=Mac \
    -clientconfig=$BUILD_CONFIG \
    -cook \
    -build \
    -stage \
    -pak \
    -archive \
    -archivedirectory="$OUTPUT_DIR" \
    -prereqs \
    -nodebuginfo \
    -utf8output

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "[SUCCESS] macOS build completed!"
    echo "=========================================="
    echo "Output: $OUTPUT_DIR"
    echo ""
    echo "To run: open \"$OUTPUT_DIR/Mac/Eon.app\""
else
    echo ""
    echo "[ERROR] Build failed. Check output above for errors."
    exit 1
fi
