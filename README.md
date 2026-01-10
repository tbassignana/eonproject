# Eon Project

A 3rd person single-player and multiplayer action-adventure game built with Unreal Engine 5.5.4 (C++ only) and SpaceTimeDB for real-time backend services.

## Overview

Eon is an online-enabled game supporting both single-player (solo instance) and multiplayer modes. The game features:
- 3rd person camera and controls
- Character movement, jumping, and environment interaction
- Simple combat system
- Inventory management with SpaceTimeDB persistence
- Real-time multiplayer synchronization
- Cross-platform support (Apple Silicon Mac OS and iOS 15+)

## Technical Stack

- **Frontend**: Unreal Engine 5.5.4 (C++ only, no Blueprints)
- **Backend**: SpaceTimeDB (maincloud.spacetimedb.com)
- **Module Name**: `eon`
- **Platforms**: Apple Silicon Mac OS, iOS 15+

## Project Structure

```
eonproject/
├── Client/                 # Unreal Engine 5.5.4 client
│   ├── Config/            # UE5 configuration files
│   ├── Source/
│   │   └── Eon/
│   │       ├── Private/   # Implementation files (.cpp)
│   │       └── Public/    # Header files (.h)
│   └── Eon.uproject
├── Server/                 # SpaceTimeDB backend
│   └── eonserver/
│       ├── src/
│       │   └── lib.rs     # Server module implementation
│       └── Cargo.toml
├── Tests/                  # Test suites
│   ├── integration/       # Client-server integration tests
│   ├── unit/              # Component unit tests
│   └── performance/       # Mobile performance tests
├── Scripts/               # Build and deployment scripts
│   ├── deploy_server.sh   # Deploy SpaceTimeDB module
│   ├── build_mac.sh       # Build for macOS
│   ├── build_ios.sh       # Build for iOS
│   └── run_all_tests.sh   # Run complete test suite
├── docs/                   # Documentation
├── TODO.md                # Development task tracker
├── LOG.md                 # Session log
└── README.md              # This file
```

## Quick Start

### Prerequisites

1. **Unreal Engine 5.5.4** - Install via Epic Games Launcher
2. **Xcode 15.2+** - Install from App Store (for iOS builds)
3. **Rust** - Install via rustup:
   ```bash
   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
   rustup target add wasm32-unknown-unknown
   ```
4. **SpaceTimeDB CLI**:
   ```bash
   curl -sSf https://install.spacetimedb.com | sh
   export PATH="$HOME/.local/bin:$PATH"
   ```

### Server Deployment

```bash
# Login to SpaceTimeDB
spacetime login

# Deploy the server module
cd Server/eonserver
spacetime build
spacetime publish eon

# View server logs
spacetime logs eon -f

# Query data
spacetime sql eon "SELECT * FROM player"
```

### Client Setup (Development)

1. Open `Client/Eon.uproject` in Unreal Engine 5.5.4
2. Let the editor compile shaders and build the project
3. Set the Game Mode:
   - Edit → Project Settings → Maps & Modes
   - Default GameMode: `EonGameMode`
4. Create a test level with a floor and spawn point
5. Press Play to test

### Building for Mac

```bash
# Using the build script
./Scripts/build_mac.sh Development

# Or manually with UAT
"/Users/Shared/Epic Games/UE_5.5/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$(pwd)/Client/Eon.uproject" \
    -platform=Mac -clientconfig=Development \
    -cook -build -stage -pak -archive \
    -archivedirectory="./Builds/Mac"
```

### Building for iOS

```bash
# Configure iOS signing in Xcode first
# Then use the build script
./Scripts/build_ios.sh Development
```

## Running Tests

```bash
# Run complete test suite
./Scripts/run_all_tests.sh

# Or run individual tests
./Tests/integration/test_spacetimedb_connection.sh
./Tests/integration/test_multiplayer_sync.sh
./Tests/integration/test_inventory_operations.sh
./Tests/performance/test_mobile_performance.sh
```

## Gameplay Controls

### Desktop (Mac)
- **WASD** - Move
- **Mouse** - Look around
- **Space** - Jump
- **Left Click** - Attack
- **E** - Interact
- **I** - Toggle Inventory

### Console Commands (Debug)
- `SpawnTestItem <item_id> <quantity>` - Spawn pickup in front of player
- `GiveItem <item_id> <quantity>` - Add item directly to inventory
- `ListInventory` - Print inventory contents

## Architecture

### Server (SpaceTimeDB)

**Tables:**
- `player` - Player data, position, health, online status
- `instance` - Game instances/lobbies
- `item_definition` - Item metadata
- `inventory_item` - Player inventory entries
- `world_item` - Spawned items in world
- `interactable_state` - Door/chest states

**Lifecycle Reducers:**
- `init` - Initialize item definitions
- `client_connected` - Handle player connection
- `client_disconnected` - Mark player offline

### Client (UE5)

**Core Components:**
- `SpaceTimeDBManager` - WebSocket connection, reducer calls
- `EonCharacter` - 3rd person character with camera
- `EonPlayerController` - Input handling, debug commands
- `InventoryComponent` - Local/synced inventory
- `PlayerSyncComponent` - Multiplayer position sync
- `InteractionComponent` - World object interaction

**Interactables:**
- `WorldItemPickup` - Collectible items
- `InteractableChest` - Loot containers
- `InteractableDoor` - Openable doors

## Configuration

### SpaceTimeDB Connection
Edit `SpaceTimeDBManager.h`:
```cpp
FString Host = TEXT("wss://maincloud.spacetimedb.com");
FString ModuleName = TEXT("eon");
```

### Enable/Disable Multiplayer
Edit `EonPlayerController.h`:
```cpp
bool bEnableSpaceTimeDB = true;  // Set to false for offline testing
```

## License

MIT License - Copyright 2026 tbassignana
