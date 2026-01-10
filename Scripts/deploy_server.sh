#!/bin/bash
# Eon Project - SpaceTimeDB Server Deployment Script
# Deploys the eonserver module to SpaceTimeDB cloud

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SERVER_DIR="$PROJECT_ROOT/Server/eonserver"
SPACETIME_CLI="${HOME}/.local/bin/spacetime"
MODULE_NAME="eon"

echo "=========================================="
echo "Eon Project - Server Deployment"
echo "=========================================="

# Check SpaceTimeDB CLI
if [ ! -f "$SPACETIME_CLI" ]; then
    echo "[ERROR] SpaceTimeDB CLI not found at $SPACETIME_CLI"
    echo "Install with: curl -sSf https://install.spacetimedb.com | sh"
    exit 1
fi

# Check Rust/Cargo
if ! command -v cargo &> /dev/null; then
    echo "[ERROR] Cargo not found. Install Rust first."
    exit 1
fi

# Check wasm32 target
if ! rustup target list --installed | grep -q "wasm32-unknown-unknown"; then
    echo "[INFO] Installing wasm32-unknown-unknown target..."
    rustup target add wasm32-unknown-unknown
fi

# Check login status
if ! $SPACETIME_CLI login show 2>&1 | grep -q "logged in"; then
    echo "[ERROR] Not logged in to SpaceTimeDB. Run: spacetime login"
    exit 1
fi

echo "[INFO] Building server module..."
cd "$SERVER_DIR"
$SPACETIME_CLI build

echo ""
echo "[INFO] Publishing to SpaceTimeDB cloud..."
read -p "This will update the live server. Continue? (y/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    $SPACETIME_CLI publish $MODULE_NAME
    echo ""
    echo "[SUCCESS] Server deployed successfully!"
    echo "Module: $MODULE_NAME"
    echo "Host: maincloud.spacetimedb.com"
    echo ""
    echo "View logs: spacetime logs $MODULE_NAME -f"
    echo "Query data: spacetime sql $MODULE_NAME \"SELECT * FROM player\""
else
    echo "[INFO] Deployment cancelled."
fi
