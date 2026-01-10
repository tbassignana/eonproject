# Eon Project

A 3rd person single-player and multiplayer action-adventure game built with Unreal Engine 5.5.4 (C++ only) and SpaceTimeDB for real-time backend services.

## Overview

Eon is an online-only game supporting both single-player (solo instance) and multiplayer modes. The game features:
- 3rd person camera and controls
- Character movement, jumping, and environment interaction
- Simple combat system
- Inventory management with SpaceTimeDB persistence
- Real-time multiplayer synchronization
- Cross-platform support (Apple Silicon Mac OS and iOS 15+)

## Technical Stack

- **Frontend**: Unreal Engine 5.5.4 (C++ only, no Blueprints)
- **Backend**: SpaceTimeDB for database, real-time updates, and instance management
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
│   └── server/            # Server integration tests
├── docs/                   # Documentation
├── TODO.md                # Development task tracker
├── LOG.md                 # Session log
├── PROMPT.MD              # Project requirements
└── LICENSE                # MIT License
```

## Setup Instructions

### Prerequisites

1. **Unreal Engine 5.5.4** - Install via Epic Games Launcher
2. **Rust** - Install via rustup: `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`
3. **SpaceTimeDB CLI** - Install: `curl -sSf https://install.spacetimedb.com | sh`

### SpaceTimeDB Setup

```bash
# Login to SpaceTimeDB (user: tbassignana)
spacetime login

# Build and publish the server module
cd Server/eonserver
spacetime publish eon --project-path .
```

### Client Setup (Mac)

```bash
# Generate project files
cd Client
/path/to/UnrealEngine/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project="$(pwd)/Eon.uproject"

# Build for Apple Silicon
xcodebuild -project Eon.xcodeproj -scheme Eon -configuration Development build
```

### Client Setup (iOS)

```bash
# Configure for iOS build
# Ensure iOS provisioning profiles are set up
cd Client
xcodebuild -project Eon.xcodeproj -scheme Eon -configuration Development -destination 'platform=iOS'
```

## Architecture

### Server (SpaceTimeDB)

The server module defines:
- **Tables**: Player, Instance, InventoryItem, WorldItem
- **Reducers**: CRUD operations for all entities, real-time sync handlers

### Client (UE5)

Key components:
- `SpaceTimeDBManager`: Backend connectivity and data sync
- `EonCharacter`: Player character with 3rd person controls
- `InventoryComponent`: Client-side inventory management
- `PlayerSyncComponent`: Multiplayer position/state syncing
- `InteractionComponent`: World object interaction system

## License

MIT License - See LICENSE file for details.
