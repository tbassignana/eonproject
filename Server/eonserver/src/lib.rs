//! Eon Project - SpaceTimeDB Game Server Module
//!
//! This module defines the database schema and reducers for a 3rd person
//! multiplayer action-adventure game. It handles:
//! - Player state (position, rotation, health)
//! - Game instances (rooms/servers for multiplayer)
//! - Inventory management
//! - Real-time synchronization

use spacetimedb::{table, reducer, Identity, ReducerContext, Table, Timestamp};

// ============================================================================
// DATABASE TABLES
// ============================================================================

/// Represents a game instance (room/server) that players can join
#[table(name = game_instance, public)]
pub struct GameInstance {
    #[primary_key]
    #[auto_inc]
    pub id: u64,
    pub name: String,
    pub max_players: u32,
    pub current_players: u32,
    pub state: InstanceState,
    pub created_at: Timestamp,
    pub owner_identity: Identity,
}

/// State of a game instance
#[derive(Clone, Copy, Debug, PartialEq, Eq, spacetimedb::SpacetimeType)]
pub enum InstanceState {
    Lobby,
    InProgress,
    Finished,
    Closed,
}

/// Represents a player in the game
#[table(name = player, public)]
pub struct Player {
    #[primary_key]
    pub identity: Identity,
    pub name: String,
    pub instance_id: Option<u64>,
    // Position (world coordinates)
    pub pos_x: f32,
    pub pos_y: f32,
    pub pos_z: f32,
    // Rotation (euler angles in degrees)
    pub rot_pitch: f32,
    pub rot_yaw: f32,
    pub rot_roll: f32,
    // Combat state
    pub health: f32,
    pub max_health: f32,
    pub is_attacking: bool,
    pub last_update: Timestamp,
    pub is_online: bool,
}

/// Definition of an item type (template)
#[table(name = item_definition, public)]
pub struct ItemDefinition {
    #[primary_key]
    #[auto_inc]
    pub id: u64,
    pub name: String,
    pub description: String,
    pub category: ItemCategory,
    pub max_stack: u32,
    pub damage: Option<f32>,     // For weapons
    pub heal_amount: Option<f32>, // For consumables
    pub is_consumable: bool,
}

/// Category of items
#[derive(Clone, Copy, Debug, PartialEq, Eq, spacetimedb::SpacetimeType)]
pub enum ItemCategory {
    Weapon,
    Consumable,
    Key,
    Quest,
    Misc,
}

/// An item in a player's inventory
#[table(name = inventory_item, public)]
pub struct InventoryItem {
    #[primary_key]
    #[auto_inc]
    pub id: u64,
    pub player_identity: Identity,
    pub item_def_id: u64,
    pub quantity: u32,
    pub slot_index: u32,
}

/// World item pickup that can be collected
#[table(name = world_item, public)]
pub struct WorldItem {
    #[primary_key]
    #[auto_inc]
    pub id: u64,
    pub instance_id: u64,
    pub item_def_id: u64,
    pub quantity: u32,
    pub pos_x: f32,
    pub pos_y: f32,
    pub pos_z: f32,
    pub is_collected: bool,
}

// ============================================================================
// PLAYER MANAGEMENT REDUCERS
// ============================================================================

/// Called when a player connects to the server
#[reducer(client_connected)]
pub fn on_connect(ctx: &ReducerContext) {
    let identity = ctx.sender;

    // Check if player already exists
    if ctx.db.player().identity().find(identity).is_some() {
        // Update existing player to online
        if let Some(mut player) = ctx.db.player().identity().find(identity) {
            player.is_online = true;
            player.last_update = ctx.timestamp;
            ctx.db.player().identity().update(player);
        }
        log::info!("Player reconnected: {:?}", identity);
    } else {
        // Create new player
        ctx.db.player().insert(Player {
            identity,
            name: format!("Player_{}", identity.to_hex()[..8].to_string()),
            instance_id: None,
            pos_x: 0.0,
            pos_y: 0.0,
            pos_z: 100.0, // Start slightly above ground
            rot_pitch: 0.0,
            rot_yaw: 0.0,
            rot_roll: 0.0,
            health: 100.0,
            max_health: 100.0,
            is_attacking: false,
            last_update: ctx.timestamp,
            is_online: true,
        });
        log::info!("New player connected: {:?}", identity);
    }
}

/// Called when a player disconnects
#[reducer(client_disconnected)]
pub fn on_disconnect(ctx: &ReducerContext) {
    let identity = ctx.sender;

    if let Some(mut player) = ctx.db.player().identity().find(identity) {
        player.is_online = false;
        player.last_update = ctx.timestamp;
        ctx.db.player().identity().update(player);
        log::info!("Player disconnected: {:?}", identity);
    }
}

/// Set the player's display name
#[reducer]
pub fn set_player_name(ctx: &ReducerContext, name: String) -> Result<(), String> {
    let identity = ctx.sender;

    if name.is_empty() || name.len() > 32 {
        return Err("Name must be 1-32 characters".to_string());
    }

    let mut player = ctx.db.player().identity().find(identity)
        .ok_or("Player not found")?;

    player.name = name;
    player.last_update = ctx.timestamp;
    ctx.db.player().identity().update(player);

    Ok(())
}

/// Update player position and rotation (called frequently for sync)
#[reducer]
pub fn update_player_transform(
    ctx: &ReducerContext,
    pos_x: f32, pos_y: f32, pos_z: f32,
    rot_pitch: f32, rot_yaw: f32, rot_roll: f32,
) -> Result<(), String> {
    let identity = ctx.sender;

    let mut player = ctx.db.player().identity().find(identity)
        .ok_or("Player not found")?;

    player.pos_x = pos_x;
    player.pos_y = pos_y;
    player.pos_z = pos_z;
    player.rot_pitch = rot_pitch;
    player.rot_yaw = rot_yaw;
    player.rot_roll = rot_roll;
    player.last_update = ctx.timestamp;

    ctx.db.player().identity().update(player);

    Ok(())
}

/// Update player combat state
#[reducer]
pub fn set_attacking(ctx: &ReducerContext, is_attacking: bool) -> Result<(), String> {
    let identity = ctx.sender;

    let mut player = ctx.db.player().identity().find(identity)
        .ok_or("Player not found")?;

    player.is_attacking = is_attacking;
    player.last_update = ctx.timestamp;
    ctx.db.player().identity().update(player);

    Ok(())
}

/// Apply damage to a player
#[reducer]
pub fn apply_damage(ctx: &ReducerContext, target_identity: Identity, damage: f32) -> Result<(), String> {
    if damage < 0.0 {
        return Err("Damage cannot be negative".to_string());
    }

    let mut target = ctx.db.player().identity().find(target_identity)
        .ok_or("Target player not found")?;

    // Check if attacker and target are in the same instance
    let attacker = ctx.db.player().identity().find(ctx.sender)
        .ok_or("Attacker not found")?;

    if attacker.instance_id != target.instance_id {
        return Err("Cannot damage players in different instances".to_string());
    }

    target.health = (target.health - damage).max(0.0);
    target.last_update = ctx.timestamp;
    ctx.db.player().identity().update(target);

    log::info!("Player {:?} dealt {} damage to {:?}", ctx.sender, damage, target_identity);

    Ok(())
}

/// Heal a player
#[reducer]
pub fn heal_player(ctx: &ReducerContext, amount: f32) -> Result<(), String> {
    if amount < 0.0 {
        return Err("Heal amount cannot be negative".to_string());
    }

    let mut player = ctx.db.player().identity().find(ctx.sender)
        .ok_or("Player not found")?;

    player.health = (player.health + amount).min(player.max_health);
    player.last_update = ctx.timestamp;
    ctx.db.player().identity().update(player);

    Ok(())
}

// ============================================================================
// INSTANCE MANAGEMENT REDUCERS
// ============================================================================

/// Create a new game instance
/// Note: The instance ID can be observed via subscription to game_instance table
#[reducer]
pub fn create_instance(ctx: &ReducerContext, name: String, max_players: u32) -> Result<(), String> {
    if name.is_empty() || name.len() > 64 {
        return Err("Instance name must be 1-64 characters".to_string());
    }

    if max_players < 1 || max_players > 16 {
        return Err("Max players must be between 1 and 16".to_string());
    }

    let instance = ctx.db.game_instance().insert(GameInstance {
        id: 0, // Auto-incremented
        name,
        max_players,
        current_players: 0,
        state: InstanceState::Lobby,
        created_at: ctx.timestamp,
        owner_identity: ctx.sender,
    });

    log::info!("Created instance {} by {:?}", instance.id, ctx.sender);

    Ok(())
}

/// Join an existing game instance
#[reducer]
pub fn join_instance(ctx: &ReducerContext, instance_id: u64) -> Result<(), String> {
    let mut instance = ctx.db.game_instance().id().find(instance_id)
        .ok_or("Instance not found")?;

    if instance.state == InstanceState::Closed || instance.state == InstanceState::Finished {
        return Err("Instance is not joinable".to_string());
    }

    if instance.current_players >= instance.max_players {
        return Err("Instance is full".to_string());
    }

    let mut player = ctx.db.player().identity().find(ctx.sender)
        .ok_or("Player not found")?;

    // Leave current instance if in one
    if let Some(old_instance_id) = player.instance_id {
        if let Some(mut old_instance) = ctx.db.game_instance().id().find(old_instance_id) {
            old_instance.current_players = old_instance.current_players.saturating_sub(1);
            ctx.db.game_instance().id().update(old_instance);
        }
    }

    // Join new instance
    player.instance_id = Some(instance_id);
    // Reset position to spawn point
    player.pos_x = 0.0;
    player.pos_y = 0.0;
    player.pos_z = 100.0;
    player.last_update = ctx.timestamp;
    ctx.db.player().identity().update(player);

    instance.current_players += 1;
    ctx.db.game_instance().id().update(instance);

    log::info!("Player {:?} joined instance {}", ctx.sender, instance_id);

    Ok(())
}

/// Leave the current game instance
#[reducer]
pub fn leave_instance(ctx: &ReducerContext) -> Result<(), String> {
    let mut player = ctx.db.player().identity().find(ctx.sender)
        .ok_or("Player not found")?;

    let instance_id = player.instance_id
        .ok_or("Player is not in an instance")?;

    let mut instance = ctx.db.game_instance().id().find(instance_id)
        .ok_or("Instance not found")?;

    player.instance_id = None;
    player.last_update = ctx.timestamp;
    ctx.db.player().identity().update(player);

    instance.current_players = instance.current_players.saturating_sub(1);
    ctx.db.game_instance().id().update(instance);

    log::info!("Player {:?} left instance {}", ctx.sender, instance_id);

    Ok(())
}

/// Update instance state (owner only)
#[reducer]
pub fn set_instance_state(ctx: &ReducerContext, instance_id: u64, state: InstanceState) -> Result<(), String> {
    let mut instance = ctx.db.game_instance().id().find(instance_id)
        .ok_or("Instance not found")?;

    if instance.owner_identity != ctx.sender {
        return Err("Only the instance owner can change state".to_string());
    }

    instance.state = state;
    ctx.db.game_instance().id().update(instance);

    log::info!("Instance {} state changed to {:?}", instance_id, state);

    Ok(())
}

// ============================================================================
// INVENTORY MANAGEMENT REDUCERS
// ============================================================================

/// Add an item to player's inventory (internal helper - not a reducer)
fn add_item_to_inventory(ctx: &ReducerContext, player_identity: Identity, item_def_id: u64, quantity: u32) -> Result<(), String> {
    // Verify item definition exists
    let item_def = ctx.db.item_definition().id().find(item_def_id)
        .ok_or("Item definition not found")?;

    if quantity == 0 {
        return Err("Quantity must be greater than 0".to_string());
    }

    // Find existing stack or create new
    let existing = ctx.db.inventory_item().iter()
        .filter(|i| i.player_identity == player_identity && i.item_def_id == item_def_id)
        .next();

    if let Some(mut existing_item) = existing {
        let new_qty = existing_item.quantity + quantity;
        if new_qty > item_def.max_stack {
            return Err(format!("Cannot exceed max stack of {}", item_def.max_stack));
        }
        existing_item.quantity = new_qty;
        ctx.db.inventory_item().id().update(existing_item);
    } else {
        if quantity > item_def.max_stack {
            return Err(format!("Cannot exceed max stack of {}", item_def.max_stack));
        }

        // Find next available slot
        let max_slot = ctx.db.inventory_item().iter()
            .filter(|i| i.player_identity == player_identity)
            .map(|i| i.slot_index)
            .max()
            .unwrap_or(0);

        ctx.db.inventory_item().insert(InventoryItem {
            id: 0,
            player_identity,
            item_def_id,
            quantity,
            slot_index: max_slot + 1,
        });
    }

    Ok(())
}

/// Add an item to player's inventory
/// Note: The item can be observed via subscription to inventory_item table
#[reducer]
pub fn add_inventory_item(ctx: &ReducerContext, item_def_id: u64, quantity: u32) -> Result<(), String> {
    add_item_to_inventory(ctx, ctx.sender, item_def_id, quantity)
}

/// Remove an item from player's inventory
#[reducer]
pub fn remove_inventory_item(ctx: &ReducerContext, item_id: u64, quantity: u32) -> Result<(), String> {
    let player_identity = ctx.sender;

    let mut item = ctx.db.inventory_item().id().find(item_id)
        .ok_or("Inventory item not found")?;

    if item.player_identity != player_identity {
        return Err("This item does not belong to you".to_string());
    }

    if quantity > item.quantity {
        return Err("Not enough items in stack".to_string());
    }

    if quantity == item.quantity {
        ctx.db.inventory_item().id().delete(item_id);
    } else {
        item.quantity -= quantity;
        ctx.db.inventory_item().id().update(item);
    }

    Ok(())
}

/// Use a consumable item
#[reducer]
pub fn use_item(ctx: &ReducerContext, item_id: u64) -> Result<(), String> {
    let player_identity = ctx.sender;

    let item = ctx.db.inventory_item().id().find(item_id)
        .ok_or("Inventory item not found")?;

    if item.player_identity != player_identity {
        return Err("This item does not belong to you".to_string());
    }

    let item_def = ctx.db.item_definition().id().find(item.item_def_id)
        .ok_or("Item definition not found")?;

    if !item_def.is_consumable {
        return Err("This item is not consumable".to_string());
    }

    // Apply item effect
    if let Some(heal) = item_def.heal_amount {
        heal_player(ctx, heal)?;
    }

    // Remove one from stack
    remove_inventory_item(ctx, item_id, 1)?;

    log::info!("Player {:?} used item {}", player_identity, item_def.name);

    Ok(())
}

/// Collect a world item pickup
#[reducer]
pub fn collect_world_item(ctx: &ReducerContext, world_item_id: u64) -> Result<(), String> {
    let player = ctx.db.player().identity().find(ctx.sender)
        .ok_or("Player not found")?;

    let mut world_item = ctx.db.world_item().id().find(world_item_id)
        .ok_or("World item not found")?;

    if world_item.is_collected {
        return Err("Item already collected".to_string());
    }

    // Check if player is in the same instance
    if player.instance_id != Some(world_item.instance_id) {
        return Err("Item is in a different instance".to_string());
    }

    // Check distance (simple 3D distance check)
    let dx = player.pos_x - world_item.pos_x;
    let dy = player.pos_y - world_item.pos_y;
    let dz = player.pos_z - world_item.pos_z;
    let distance = (dx * dx + dy * dy + dz * dz).sqrt();

    const PICKUP_RANGE: f32 = 200.0; // Unreal units
    if distance > PICKUP_RANGE {
        return Err("Too far from item".to_string());
    }

    // Add to inventory
    add_item_to_inventory(ctx, ctx.sender, world_item.item_def_id, world_item.quantity)?;

    // Mark as collected
    world_item.is_collected = true;
    ctx.db.world_item().id().update(world_item);

    log::info!("Player {:?} collected world item {}", ctx.sender, world_item_id);

    Ok(())
}

// ============================================================================
// ADMIN / SETUP REDUCERS
// ============================================================================

/// Initialize default item definitions (call once during setup)
#[reducer]
pub fn init_item_definitions(ctx: &ReducerContext) -> Result<(), String> {
    // Only allow if no items exist
    if ctx.db.item_definition().iter().count() > 0 {
        return Err("Item definitions already initialized".to_string());
    }

    // Weapons
    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Iron Sword".to_string(),
        description: "A basic iron sword".to_string(),
        category: ItemCategory::Weapon,
        max_stack: 1,
        damage: Some(25.0),
        heal_amount: None,
        is_consumable: false,
    });

    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Steel Sword".to_string(),
        description: "A sharp steel sword".to_string(),
        category: ItemCategory::Weapon,
        max_stack: 1,
        damage: Some(40.0),
        heal_amount: None,
        is_consumable: false,
    });

    // Consumables
    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Health Potion".to_string(),
        description: "Restores 50 health".to_string(),
        category: ItemCategory::Consumable,
        max_stack: 10,
        damage: None,
        heal_amount: Some(50.0),
        is_consumable: true,
    });

    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Large Health Potion".to_string(),
        description: "Restores 100 health".to_string(),
        category: ItemCategory::Consumable,
        max_stack: 5,
        damage: None,
        heal_amount: Some(100.0),
        is_consumable: true,
    });

    // Keys
    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Rusty Key".to_string(),
        description: "Opens old locks".to_string(),
        category: ItemCategory::Key,
        max_stack: 1,
        damage: None,
        heal_amount: None,
        is_consumable: false,
    });

    // Misc
    ctx.db.item_definition().insert(ItemDefinition {
        id: 0,
        name: "Gold Coin".to_string(),
        description: "Currency".to_string(),
        category: ItemCategory::Misc,
        max_stack: 9999,
        damage: None,
        heal_amount: None,
        is_consumable: false,
    });

    log::info!("Item definitions initialized");

    Ok(())
}

/// Spawn a world item in an instance (admin/game logic)
/// Note: The spawned item can be observed via subscription to world_item table
#[reducer]
pub fn spawn_world_item(
    ctx: &ReducerContext,
    instance_id: u64,
    item_def_id: u64,
    quantity: u32,
    pos_x: f32, pos_y: f32, pos_z: f32,
) -> Result<(), String> {
    // Verify instance exists
    ctx.db.game_instance().id().find(instance_id)
        .ok_or("Instance not found")?;

    // Verify item definition exists
    ctx.db.item_definition().id().find(item_def_id)
        .ok_or("Item definition not found")?;

    let item = ctx.db.world_item().insert(WorldItem {
        id: 0,
        instance_id,
        item_def_id,
        quantity,
        pos_x,
        pos_y,
        pos_z,
        is_collected: false,
    });

    log::info!("Spawned world item {} at ({}, {}, {})", item.id, pos_x, pos_y, pos_z);

    Ok(())
}
