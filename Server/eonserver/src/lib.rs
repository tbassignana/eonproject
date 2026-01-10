use spacetimedb::{spacetimedb, Identity, ReducerContext, Table, Timestamp};

// ============================================================================
// DATA TYPES
// ============================================================================

#[derive(Clone, Debug, PartialEq)]
pub struct Vector3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Rotator {
    pub pitch: f32,
    pub yaw: f32,
    pub roll: f32,
}

// ============================================================================
// TABLES
// ============================================================================

#[spacetimedb::table(name = instance, public)]
pub struct Instance {
    #[primary_key]
    #[auto_inc]
    pub instance_id: u64,
    pub name: String,
    pub max_players: u32,
    pub is_public: bool,
    pub created_at: Timestamp,
    pub owner_identity: Identity,
}

#[spacetimedb::table(name = player, public)]
pub struct Player {
    #[primary_key]
    pub identity: Identity,
    pub username: String,
    pub instance_id: Option<u64>,
    pub position_x: f32,
    pub position_y: f32,
    pub position_z: f32,
    pub rotation_pitch: f32,
    pub rotation_yaw: f32,
    pub rotation_roll: f32,
    pub health: f32,
    pub max_health: f32,
    pub is_online: bool,
    pub last_seen: Timestamp,
}

#[spacetimedb::table(name = item_definition, public)]
pub struct ItemDefinition {
    #[primary_key]
    pub item_id: String,
    pub display_name: String,
    pub description: String,
    pub item_type: String, // "weapon", "consumable", "key", "resource"
    pub max_stack: u32,
    pub icon_path: String,
}

#[spacetimedb::table(name = inventory_item, public)]
pub struct InventoryItem {
    #[primary_key]
    #[auto_inc]
    pub entry_id: u64,
    pub owner_identity: Identity,
    pub item_id: String,
    pub quantity: u32,
    pub slot_index: u32,
}

#[spacetimedb::table(name = world_item, public)]
pub struct WorldItem {
    #[primary_key]
    #[auto_inc]
    pub world_item_id: u64,
    pub instance_id: u64,
    pub item_id: String,
    pub quantity: u32,
    pub position_x: f32,
    pub position_y: f32,
    pub position_z: f32,
    pub is_collected: bool,
}

#[spacetimedb::table(name = interactable_state, public)]
pub struct InteractableState {
    #[primary_key]
    pub interactable_id: String,
    pub instance_id: u64,
    pub is_active: bool,
    pub state_data: String, // JSON for flexible state storage
}

// ============================================================================
// INSTANCE MANAGEMENT REDUCERS
// ============================================================================

#[spacetimedb::reducer]
pub fn create_instance(ctx: &ReducerContext, name: String, max_players: u32, is_public: bool) {
    let instance = Instance {
        instance_id: 0, // auto_inc
        name,
        max_players,
        is_public,
        created_at: ctx.timestamp,
        owner_identity: ctx.sender,
    };
    ctx.db.instance().insert(instance);
    log::info!("Instance created by {:?}", ctx.sender);
}

#[spacetimedb::reducer]
pub fn delete_instance(ctx: &ReducerContext, instance_id: u64) {
    if let Some(instance) = ctx.db.instance().instance_id().find(instance_id) {
        if instance.owner_identity == ctx.sender {
            // Remove all players from this instance
            for player in ctx.db.player().iter() {
                if player.instance_id == Some(instance_id) {
                    let mut updated = player.clone();
                    updated.instance_id = None;
                    ctx.db.player().identity().update(updated);
                }
            }
            // Remove all world items from this instance
            for item in ctx.db.world_item().iter() {
                if item.instance_id == instance_id {
                    ctx.db.world_item().world_item_id().delete(item.world_item_id);
                }
            }
            ctx.db.instance().instance_id().delete(instance_id);
            log::info!("Instance {} deleted", instance_id);
        }
    }
}

// ============================================================================
// PLAYER MANAGEMENT REDUCERS
// ============================================================================

#[spacetimedb::reducer]
pub fn register_player(ctx: &ReducerContext, username: String) {
    if ctx.db.player().identity().find(ctx.sender).is_some() {
        log::warn!("Player already registered: {:?}", ctx.sender);
        return;
    }

    let player = Player {
        identity: ctx.sender,
        username,
        instance_id: None,
        position_x: 0.0,
        position_y: 0.0,
        position_z: 0.0,
        rotation_pitch: 0.0,
        rotation_yaw: 0.0,
        rotation_roll: 0.0,
        health: 100.0,
        max_health: 100.0,
        is_online: true,
        last_seen: ctx.timestamp,
    };
    ctx.db.player().insert(player);
    log::info!("Player registered: {:?}", ctx.sender);
}

#[spacetimedb::reducer]
pub fn set_player_online(ctx: &ReducerContext, online: bool) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated = player.clone();
        updated.is_online = online;
        updated.last_seen = ctx.timestamp;
        ctx.db.player().identity().update(updated);
    }
}

#[spacetimedb::reducer]
pub fn join_instance(ctx: &ReducerContext, instance_id: u64) {
    let instance = match ctx.db.instance().instance_id().find(instance_id) {
        Some(i) => i,
        None => {
            log::warn!("Instance {} not found", instance_id);
            return;
        }
    };

    // Count current players in instance
    let current_players = ctx.db.player().iter()
        .filter(|p| p.instance_id == Some(instance_id) && p.is_online)
        .count() as u32;

    if current_players >= instance.max_players {
        log::warn!("Instance {} is full", instance_id);
        return;
    }

    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated = player.clone();
        updated.instance_id = Some(instance_id);
        updated.position_x = 0.0;
        updated.position_y = 0.0;
        updated.position_z = 100.0; // Spawn height
        ctx.db.player().identity().update(updated);
        log::info!("Player {:?} joined instance {}", ctx.sender, instance_id);
    }
}

#[spacetimedb::reducer]
pub fn leave_instance(ctx: &ReducerContext) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated = player.clone();
        updated.instance_id = None;
        ctx.db.player().identity().update(updated);
        log::info!("Player {:?} left instance", ctx.sender);
    }
}

// ============================================================================
// PLAYER SYNC REDUCERS (Real-time position/state updates)
// ============================================================================

#[spacetimedb::reducer]
pub fn update_player_position(
    ctx: &ReducerContext,
    x: f32, y: f32, z: f32,
    pitch: f32, yaw: f32, roll: f32
) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated = player.clone();
        updated.position_x = x;
        updated.position_y = y;
        updated.position_z = z;
        updated.rotation_pitch = pitch;
        updated.rotation_yaw = yaw;
        updated.rotation_roll = roll;
        updated.last_seen = ctx.timestamp;
        ctx.db.player().identity().update(updated);
    }
}

#[spacetimedb::reducer]
pub fn update_player_health(ctx: &ReducerContext, health: f32) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated = player.clone();
        updated.health = health.clamp(0.0, updated.max_health);
        ctx.db.player().identity().update(updated);
    }
}

// ============================================================================
// INVENTORY SYSTEM REDUCERS
// ============================================================================

#[spacetimedb::reducer]
pub fn init_item_definitions(ctx: &ReducerContext) {
    // Only initialize if no items exist
    if ctx.db.item_definition().iter().next().is_some() {
        return;
    }

    let items = vec![
        ItemDefinition {
            item_id: "sword_basic".to_string(),
            display_name: "Iron Sword".to_string(),
            description: "A basic iron sword for combat.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/sword_basic".to_string(),
        },
        ItemDefinition {
            item_id: "health_potion".to_string(),
            display_name: "Health Potion".to_string(),
            description: "Restores 50 health points.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 10,
            icon_path: "/Game/UI/Icons/health_potion".to_string(),
        },
        ItemDefinition {
            item_id: "key_gold".to_string(),
            display_name: "Golden Key".to_string(),
            description: "Opens golden chests and doors.".to_string(),
            item_type: "key".to_string(),
            max_stack: 5,
            icon_path: "/Game/UI/Icons/key_gold".to_string(),
        },
        ItemDefinition {
            item_id: "gem_ruby".to_string(),
            display_name: "Ruby Gem".to_string(),
            description: "A valuable ruby gem.".to_string(),
            item_type: "resource".to_string(),
            max_stack: 99,
            icon_path: "/Game/UI/Icons/gem_ruby".to_string(),
        },
        ItemDefinition {
            item_id: "shield_wooden".to_string(),
            display_name: "Wooden Shield".to_string(),
            description: "A simple wooden shield for defense.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/shield_wooden".to_string(),
        },
    ];

    for item in items {
        ctx.db.item_definition().insert(item);
    }
    log::info!("Item definitions initialized");
}

#[spacetimedb::reducer]
pub fn add_item_to_inventory(ctx: &ReducerContext, item_id: String, quantity: u32) {
    // Verify item exists
    let item_def = match ctx.db.item_definition().item_id().find(&item_id) {
        Some(def) => def,
        None => {
            log::warn!("Item definition not found: {}", item_id);
            return;
        }
    };

    // Check if player already has this item
    for entry in ctx.db.inventory_item().iter() {
        if entry.owner_identity == ctx.sender && entry.item_id == item_id {
            let new_qty = (entry.quantity + quantity).min(item_def.max_stack);
            let mut updated = entry.clone();
            updated.quantity = new_qty;
            ctx.db.inventory_item().entry_id().update(updated);
            log::info!("Updated inventory item {} for player {:?}", item_id, ctx.sender);
            return;
        }
    }

    // Find next available slot
    let mut used_slots: Vec<u32> = ctx.db.inventory_item().iter()
        .filter(|e| e.owner_identity == ctx.sender)
        .map(|e| e.slot_index)
        .collect();
    used_slots.sort();

    let next_slot = (0..100u32)
        .find(|s| !used_slots.contains(s))
        .unwrap_or(0);

    let entry = InventoryItem {
        entry_id: 0, // auto_inc
        owner_identity: ctx.sender,
        item_id,
        quantity: quantity.min(item_def.max_stack),
        slot_index: next_slot,
    };
    ctx.db.inventory_item().insert(entry);
    log::info!("Added new inventory item for player {:?}", ctx.sender);
}

#[spacetimedb::reducer]
pub fn remove_item_from_inventory(ctx: &ReducerContext, entry_id: u64, quantity: u32) {
    if let Some(entry) = ctx.db.inventory_item().entry_id().find(entry_id) {
        if entry.owner_identity != ctx.sender {
            log::warn!("Player {:?} does not own inventory entry {}", ctx.sender, entry_id);
            return;
        }

        if quantity >= entry.quantity {
            ctx.db.inventory_item().entry_id().delete(entry_id);
        } else {
            let mut updated = entry.clone();
            updated.quantity -= quantity;
            ctx.db.inventory_item().entry_id().update(updated);
        }
        log::info!("Removed {} items from inventory entry {}", quantity, entry_id);
    }
}

#[spacetimedb::reducer]
pub fn move_inventory_item(ctx: &ReducerContext, entry_id: u64, new_slot: u32) {
    if let Some(entry) = ctx.db.inventory_item().entry_id().find(entry_id) {
        if entry.owner_identity != ctx.sender {
            return;
        }

        // Check if slot is occupied
        for other in ctx.db.inventory_item().iter() {
            if other.owner_identity == ctx.sender && other.slot_index == new_slot && other.entry_id != entry_id {
                // Swap slots
                let mut other_updated = other.clone();
                other_updated.slot_index = entry.slot_index;
                ctx.db.inventory_item().entry_id().update(other_updated);
                break;
            }
        }

        let mut updated = entry.clone();
        updated.slot_index = new_slot;
        ctx.db.inventory_item().entry_id().update(updated);
    }
}

#[spacetimedb::reducer]
pub fn use_consumable(ctx: &ReducerContext, entry_id: u64) {
    let entry = match ctx.db.inventory_item().entry_id().find(entry_id) {
        Some(e) if e.owner_identity == ctx.sender => e,
        _ => return,
    };

    let item_def = match ctx.db.item_definition().item_id().find(&entry.item_id) {
        Some(def) => def,
        None => return,
    };

    if item_def.item_type != "consumable" {
        log::warn!("Item {} is not consumable", entry.item_id);
        return;
    }

    // Apply effect based on item
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        let mut updated_player = player.clone();
        match entry.item_id.as_str() {
            "health_potion" => {
                updated_player.health = (updated_player.health + 50.0).min(updated_player.max_health);
            }
            _ => {}
        }
        ctx.db.player().identity().update(updated_player);
    }

    // Remove one from stack
    if entry.quantity <= 1 {
        ctx.db.inventory_item().entry_id().delete(entry_id);
    } else {
        let mut updated = entry.clone();
        updated.quantity -= 1;
        ctx.db.inventory_item().entry_id().update(updated);
    }
}

// ============================================================================
// WORLD ITEM REDUCERS
// ============================================================================

#[spacetimedb::reducer]
pub fn spawn_world_item(
    ctx: &ReducerContext,
    instance_id: u64,
    item_id: String,
    quantity: u32,
    x: f32, y: f32, z: f32
) {
    // Verify instance exists
    if ctx.db.instance().instance_id().find(instance_id).is_none() {
        return;
    }

    // Verify item definition exists
    if ctx.db.item_definition().item_id().find(&item_id).is_none() {
        return;
    }

    let world_item = WorldItem {
        world_item_id: 0, // auto_inc
        instance_id,
        item_id,
        quantity,
        position_x: x,
        position_y: y,
        position_z: z,
        is_collected: false,
    };
    ctx.db.world_item().insert(world_item);
}

#[spacetimedb::reducer]
pub fn collect_world_item(ctx: &ReducerContext, world_item_id: u64) {
    let world_item = match ctx.db.world_item().world_item_id().find(world_item_id) {
        Some(wi) if !wi.is_collected => wi,
        _ => return,
    };

    // Mark as collected
    let mut updated = world_item.clone();
    updated.is_collected = true;
    ctx.db.world_item().world_item_id().update(updated);

    // Add to player inventory (reuse add_item logic inline)
    let item_def = match ctx.db.item_definition().item_id().find(&world_item.item_id) {
        Some(def) => def,
        None => return,
    };

    // Check existing stack
    for entry in ctx.db.inventory_item().iter() {
        if entry.owner_identity == ctx.sender && entry.item_id == world_item.item_id {
            let new_qty = (entry.quantity + world_item.quantity).min(item_def.max_stack);
            let mut entry_updated = entry.clone();
            entry_updated.quantity = new_qty;
            ctx.db.inventory_item().entry_id().update(entry_updated);
            return;
        }
    }

    // New stack
    let mut used_slots: Vec<u32> = ctx.db.inventory_item().iter()
        .filter(|e| e.owner_identity == ctx.sender)
        .map(|e| e.slot_index)
        .collect();
    used_slots.sort();

    let next_slot = (0..100u32).find(|s| !used_slots.contains(s)).unwrap_or(0);

    let entry = InventoryItem {
        entry_id: 0,
        owner_identity: ctx.sender,
        item_id: world_item.item_id,
        quantity: world_item.quantity.min(item_def.max_stack),
        slot_index: next_slot,
    };
    ctx.db.inventory_item().insert(entry);
}

// ============================================================================
// INTERACTABLE STATE REDUCERS
// ============================================================================

#[spacetimedb::reducer]
pub fn set_interactable_state(
    ctx: &ReducerContext,
    interactable_id: String,
    instance_id: u64,
    is_active: bool,
    state_data: String
) {
    if let Some(existing) = ctx.db.interactable_state().interactable_id().find(&interactable_id) {
        if existing.instance_id == instance_id {
            let mut updated = existing.clone();
            updated.is_active = is_active;
            updated.state_data = state_data;
            ctx.db.interactable_state().interactable_id().update(updated);
            return;
        }
    }

    let state = InteractableState {
        interactable_id,
        instance_id,
        is_active,
        state_data,
    };
    ctx.db.interactable_state().insert(state);
}

#[spacetimedb::reducer]
pub fn toggle_interactable(ctx: &ReducerContext, interactable_id: String) {
    if let Some(existing) = ctx.db.interactable_state().interactable_id().find(&interactable_id) {
        let mut updated = existing.clone();
        updated.is_active = !updated.is_active;
        ctx.db.interactable_state().interactable_id().update(updated);
    }
}
