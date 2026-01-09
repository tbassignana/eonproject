# Eon Project

A 3rd person single-player and multiplayer action-adventure game built with Unreal Engine 5.5.4 (C++) and SpaceTimeDB for real-time, persistent backend services.

## Project Overview

Eon Project is an online-only game featuring:
- **Third-person camera and controls** - Movement, jumping, interacting with environment
- **Single-player mode** - Online solo instance with state persistence
- **Multiplayer mode** - Instanced sessions with real-time player synchronization
- **Combat system** - Melee attacks, health/damage
- **Inventory management** - Collect items, use consumables, manage inventory
- **Exploration** - Navigate environments, collect items, interact with objects

## Architecture

```
┌─────────────────┐      WebSocket       ┌──────────────────┐
│   UE5 Client    │◄───────────────────►│   SpaceTimeDB    │
│   (C++ Only)    │   Real-time Sync    │   Cloud Server   │
└─────────────────┘                      └──────────────────┘
        │                                         │
        │                                         │
   Player Input                            Game State
   Rendering                               - Players
   Animation                               - Instances
   UI/HUD                                  - Inventory
                                           - World Items
```

## Project Structure

```
eonproject/
├── Server/                 # SpaceTimeDB backend module
│   └── eonserver/         # Rust module with game logic
│       ├── Cargo.toml
│       └── src/
│           └── lib.rs     # Database schema & reducers
├── Client/                 # Unreal Engine 5.5.4 client
│   ├── Source/            # C++ source code
│   ├── Config/            # UE configuration files
│   └── Content/           # Game assets
├── Tests/                  # Test suites
├── Documentation/          # Project documentation
├── Scripts/                # Build and deployment scripts
├── TODO.md                 # Development tracker
└── README.md              # This file
```

## Prerequisites

### Required Software
- **Unreal Engine 5.5.4** - [Download from Epic Games Launcher](https://www.unrealengine.com/)
- **Rust** - Install via [rustup](https://rustup.rs/): `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`
- **SpaceTimeDB CLI** - Install via: `curl -sSf https://install.spacetimedb.com | sh`
- **CMake** - For building C++ components
- **Xcode** (macOS) or **Visual Studio** (Windows) - For C++ compilation

### Optional
- **wasm-opt** (from [Binaryen](https://github.com/WebAssembly/binaryen)) - For optimizing WASM modules

## Setup Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/tbassignana/eonproject.git
cd eonproject
```

### 2. Install Rust and SpaceTimeDB CLI

```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source "$HOME/.cargo/env"

# Install SpaceTimeDB CLI
curl -sSf https://install.spacetimedb.com | sh

# Add to PATH (add to your shell profile)
export PATH="$HOME/.local/bin:$PATH"

# Verify installation
spacetime --version
```

### 3. Authenticate with SpaceTimeDB

```bash
# This will open a browser for authentication
spacetime login
```

### 4. Build the Server Module

```bash
cd Server/eonserver
spacetime build
```

### 5. Start Local Development Server

```bash
# Start local SpaceTimeDB instance
spacetime start

# In another terminal, publish the module
spacetime publish --local eonserver
```

### 6. Initialize Game Data

```bash
# Call the init_item_definitions reducer to populate test items
spacetime call --local eonserver init_item_definitions
```

## SpaceTimeDB Schema

### Tables

| Table | Description |
|-------|-------------|
| `game_instance` | Multiplayer game rooms/sessions |
| `player` | Player state (position, health, etc.) |
| `item_definition` | Item templates (weapons, consumables) |
| `inventory_item` | Player inventory contents |
| `world_item` | Collectible items in the game world |

### Key Reducers (Server Functions)

| Reducer | Description |
|---------|-------------|
| `create_instance` | Create new multiplayer room |
| `join_instance` | Join existing room |
| `leave_instance` | Leave current room |
| `update_player_transform` | Sync position/rotation |
| `apply_damage` | Deal damage to another player |
| `add_inventory_item` | Add item to inventory |
| `use_item` | Consume an item |
| `collect_world_item` | Pick up item from world |

## Development

### Running Tests

```bash
# Server-side tests
cd Server/eonserver
cargo test
```

### Building for Production

```bash
# Build optimized WASM module
cd Server/eonserver
spacetime build

# Deploy to SpaceTimeDB cloud
spacetime publish eonserver
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit a pull request

## License

MIT License - see [LICENSE](LICENSE) for details.

## Links

- [SpaceTimeDB Documentation](https://spacetimedb.com/docs/)
- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [Project Repository](https://github.com/tbassignana/eonproject)
