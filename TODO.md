# TODO.md - Eon Project Development Tracker

## Phase 0: Environment Setup
- [x] Read PROMPT.MD and understand requirements
- [x] **0.1** Install Rust and Cargo (required for SpaceTimeDB)
- [x] **0.2** Install SpaceTimeDB CLI (`spacetime`)
- [ ] **0.3** Authenticate SpaceTimeDB with user `tbassignana` (requires manual browser auth)
- [x] **0.4** Verify Unreal Engine 5.5.4 installation or provide installation instructions
- [x] **0.5** Create project directory structure

## Phase 1: Project Foundation
- [x] **1.1** Create Unreal Engine 5.5.4 C++ project structure (no Blueprints)
- [x] **1.2** Set up `.gitignore` for UE5 and SpaceTimeDB artifacts
- [x] **1.3** Create README.md with project overview and setup instructions
- [x] **1.4** Set up build configuration (Eon.Build.cs, Target.cs files)

## Phase 2: SpaceTimeDB Backend Module
- [x] **2.1** Initialize SpaceTimeDB module (`spacetime init`)
- [x] **2.2** Define core database tables:
  - [x] **2.2.1** `Player` table (id, name, position, rotation, health, instance_id)
  - [x] **2.2.2** `GameInstance` table (id, name, max_players, state, created_at)
  - [x] **2.2.3** `InventoryItem` table (id, player_id, item_type, quantity, metadata)
  - [x] **2.2.4** `ItemDefinition` table (id, name, description, category, properties)
  - [x] **2.2.5** `WorldItem` table (pickups in game world)
- [x] **2.3** Implement reducers (server-side functions):
  - [x] **2.3.1** `create_instance` - Create new game instance
  - [x] **2.3.2** `join_instance` - Join existing instance
  - [x] **2.3.3** `leave_instance` - Leave current instance
  - [x] **2.3.4** `update_player_transform` - Real-time position/rotation sync
  - [x] **2.3.5** `set_attacking` / `apply_damage` / `heal_player` - Combat state
  - [x] **2.3.6** `add_inventory_item` - Add item to player inventory
  - [x] **2.3.7** `remove_inventory_item` - Remove item from inventory
  - [x] **2.3.8** `use_item` - Use consumable items
  - [x] **2.3.9** `collect_world_item` - Pick up items from world
  - [x] **2.3.10** `spawn_world_item` - Spawn items in world
- [x] **2.4** Implement `init_item_definitions` reducer for test data
- [ ] **2.5** Write unit tests for all reducers
- [ ] **2.6** Deploy SpaceTimeDB module to cloud (requires login)

## Phase 3: Unreal Engine Core Systems (C++ Only)
- [x] **3.1** Create Game Module structure
  - [x] **3.1.1** Eon module (game logic, Build.cs)
  - [x] **3.1.2** SpaceTimeDBManager subsystem (network integration)
  - [ ] **3.1.3** EonUI module (HUD, menus) - TODO
- [x] **3.2** Implement Third Person Character Controller
  - [x] **3.2.1** `AEonCharacter` class with movement, jumping
  - [x] **3.2.2** `AEonPlayerController` for input handling
  - [x] **3.2.3** Third-person camera system (SpringArm + Camera)
  - [ ] **3.2.4** Animation state machine integration (C++ driven) - TODO
- [x] **3.3** Implement SpaceTimeDB Client Integration
  - [x] **3.3.1** WebSocket connection manager (USpaceTimeDBManager)
  - [x] **3.3.2** JSON serialization for SpaceTimeDB protocol
  - [x] **3.3.3** Subscription handling for real-time updates (partial)
  - [x] **3.3.4** Reducer call wrapper functions
- [ ] **3.4** Write tests for character controller

## Phase 4: Networking and Multiplayer
- [x] **4.1** Implement Network Manager (USpaceTimeDBManager)
  - [x] **4.1.1** Connection state machine (connecting, connected, disconnected)
  - [ ] **4.1.2** Reconnection logic with exponential backoff - TODO
  - [ ] **4.1.3** Message queue for offline buffering - TODO
- [ ] **4.2** Implement Player Synchronization
  - [ ] **4.2.1** Position interpolation for smooth movement
  - [ ] **4.2.2** Client-side prediction
  - [ ] **4.2.3** Server reconciliation
- [ ] **4.3** Implement Instance Management UI
  - [ ] **4.3.1** Instance browser widget
  - [ ] **4.3.2** Create instance dialog
  - [ ] **4.3.3** Player list widget
- [ ] **4.4** Write integration tests for multiplayer sync

## Phase 5: Gameplay Features
- [x] **5.1** Implement Combat System (basic)
  - [x] **5.1.1** Basic melee attack (sphere trace)
  - [x] **5.1.2** Health and damage system
  - [ ] **5.1.3** Combat state replication - TODO
- [ ] **5.2** Implement Inventory System
  - [ ] **5.2.1** `UInventoryComponent` for players
  - [ ] **5.2.2** Item pickup actors
  - [ ] **5.2.3** Inventory UI widget
  - [ ] **5.2.4** Item use mechanics
- [ ] **5.3** Implement Exploration Features
  - [ ] **5.3.1** Interactable objects system
  - [ ] **5.3.2** Simple level/environment
  - [ ] **5.3.3** Collectibles and pickups
- [ ] **5.4** Write gameplay tests

## Phase 6: Testing and Quality Assurance
- [ ] **6.1** Unit Tests
  - [ ] **6.1.1** SpaceTimeDB reducer tests
  - [ ] **6.1.2** Character controller tests
  - [ ] **6.1.3** Network manager tests
  - [ ] **6.1.4** Inventory system tests
- [ ] **6.2** Integration Tests
  - [ ] **6.2.1** Single-player flow test
  - [ ] **6.2.2** Multiplayer sync test
  - [ ] **6.2.3** Instance creation/joining test
- [ ] **6.3** End-to-End Tests
  - [ ] **6.3.1** Full gameplay session test
  - [ ] **6.3.2** Network disconnect/reconnect test
  - [ ] **6.3.3** Performance benchmarks

## Phase 7: Deployment
- [ ] **7.1** Build Configuration
  - [ ] **7.1.1** Release build settings
  - [ ] **7.1.2** Optimization passes
- [ ] **7.2** SpaceTimeDB Production Deployment
  - [ ] **7.2.1** Production module deployment
  - [ ] **7.2.2** Connection string configuration
- [ ] **7.3** Client Distribution
  - [ ] **7.3.1** PC build packaging
  - [ ] **7.3.2** Installation instructions
- [ ] **7.4** Final verification and documentation

## Phase 8: Polish and Finalization
- [ ] **8.1** Code cleanup and documentation
- [ ] **8.2** Final test suite run
- [ ] **8.3** Update README with final instructions
- [ ] **8.4** Tag release version

---

## Current Status
**Current Task**: Phase 3.2.4 - Continue implementing UE5 client features
**Blockers**: SpaceTimeDB login requires interactive browser authentication for cloud deployment
**Notes**: Core project structure complete. SpaceTimeDB backend and UE5 client skeleton implemented.

## Session Log
- **2026-01-09 15:21**: Initial TODO.md created. Project is fresh with only LICENSE and PROMPT.MD.
- **2026-01-09 15:25**: Rust 1.92.0 installed via rustup. SpaceTimeDB CLI v1.11.2 installed via official script.
- **2026-01-09 15:26**: Note: `spacetime login` requires manual browser auth. Proceeding with local dev setup.
- **2026-01-09 15:30**: Created project directory structure (Server/, Client/, Tests/, etc.)
- **2026-01-09 15:31**: Created .gitignore for UE5 and SpaceTimeDB artifacts
- **2026-01-09 15:35**: Implemented complete SpaceTimeDB module with all tables and reducers
- **2026-01-09 15:38**: Fixed reducer return types (SpaceTimeDB reducers can only return () or Result<(), E>)
- **2026-01-09 15:39**: SpaceTimeDB module builds successfully (`spacetime build`)
- **2026-01-09 15:40**: Created README.md with project overview and setup instructions
- **2026-01-09 15:41**: Committed SpaceTimeDB backend module to GitHub
- **2026-01-09 15:45**: Created UE5.5.4 C++ project structure with Eon module
- **2026-01-09 15:50**: Implemented AEonCharacter (3rd person controller, camera, combat)
- **2026-01-09 15:52**: Implemented AEonGameMode and AEonPlayerController
- **2026-01-09 15:55**: Implemented USpaceTimeDBManager for WebSocket connection to backend
- **2026-01-09 15:57**: Added UE5 config files (DefaultEngine, DefaultGame, DefaultInput)
- **2026-01-09 15:58**: Committed UE5 client structure to GitHub
