#!/bin/bash
# Eon Project - iOS Build Script
# Builds the game for iOS 15+

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CLIENT_DIR="$PROJECT_ROOT/Client"
UE_ROOT="/Users/Shared/Epic Games/UE_5.5"
UAT="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"

BUILD_CONFIG="${1:-Development}"  # Development, Shipping, DebugGame
OUTPUT_DIR="$PROJECT_ROOT/Builds/iOS"

echo "=========================================="
echo "Eon Project - iOS Build"
echo "=========================================="
echo "Configuration: $BUILD_CONFIG"
echo "Output: $OUTPUT_DIR"
echo ""

# Check UE5 installation
if [ ! -f "$UAT" ]; then
    echo "[ERROR] Unreal Engine not found at $UE_ROOT"
    exit 1
fi

# Check Xcode
if ! command -v xcodebuild &> /dev/null; then
    echo "[ERROR] Xcode not installed. Install from App Store."
    exit 1
fi

# Check for valid iOS signing identity
if ! security find-identity -v -p codesigning | grep -q "iPhone"; then
    echo "[WARNING] No iOS signing identity found."
    echo "For device deployment, configure signing in Xcode."
    echo "Continuing with simulator build..."
fi

# Check project file
if [ ! -f "$CLIENT_DIR/Eon.uproject" ]; then
    echo "[ERROR] Project file not found"
    exit 1
fi

# Clean previous builds
echo "[INFO] Cleaning previous builds..."
rm -rf "$OUTPUT_DIR" 2>/dev/null || true
mkdir -p "$OUTPUT_DIR"

# Build for iOS
echo "[INFO] Starting iOS build (this may take a while)..."
"$UAT" BuildCookRun \
    -project="$CLIENT_DIR/Eon.uproject" \
    -noP4 \
    -platform=IOS \
    -clientconfig=$BUILD_CONFIG \
    -cook \
    -build \
    -stage \
    -pak \
    -archive \
    -archivedirectory="$OUTPUT_DIR" \
    -prereqs \
    -nodebuginfo \
    -utf8output \
    -distribution

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "[SUCCESS] iOS build completed!"
    echo "=========================================="
    echo "Output: $OUTPUT_DIR"
    echo ""
    echo "To deploy to device:"
    echo "1. Open Xcode"
    echo "2. Window > Devices and Simulators"
    echo "3. Drag the .ipa to your connected device"
    echo ""
    echo "For TestFlight/App Store:"
    echo "1. Open $OUTPUT_DIR in Finder"
    echo "2. Use Transporter app to upload .ipa"
else
    echo ""
    echo "[ERROR] Build failed. Check output above for errors."
    exit 1
fi
