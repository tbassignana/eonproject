use spacetimedb::{table, reducer, Table, ReducerContext, Identity, Timestamp};

// ============================================================================
// TABLES
// ============================================================================

#[table(name = instance, public)]
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

#[table(name = player, public)]
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

#[table(name = item_definition, public)]
pub struct ItemDefinition {
    #[primary_key]
    pub item_id: String,
    pub display_name: String,
    pub description: String,
    pub item_type: String,
    pub max_stack: u32,
    pub icon_path: String,
}

#[table(name = inventory_item, public)]
pub struct InventoryItem {
    #[primary_key]
    #[auto_inc]
    pub entry_id: u64,
    pub owner_identity: Identity,
    pub item_id: String,
    pub quantity: u32,
    pub slot_index: u32,
}

#[derive(Clone)]
#[table(name = world_item, public)]
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

#[table(name = interactable_state, public)]
pub struct InteractableState {
    #[primary_key]
    pub interactable_id: String,
    pub instance_id: u64,
    pub is_active: bool,
    pub state_data: String,
}

// ============================================================================
// LIFECYCLE REDUCERS
// ============================================================================

#[reducer(client_connected)]
pub fn client_connected(ctx: &ReducerContext) {
    log::info!("Client connected: {:?}", ctx.sender);
}

#[reducer(client_disconnected)]
pub fn client_disconnected(ctx: &ReducerContext) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            is_online: false,
            last_seen: ctx.timestamp,
            ..player
        });
    }
    log::info!("Client disconnected: {:?}", ctx.sender);
}

#[reducer(init)]
pub fn init(ctx: &ReducerContext) {
    // Initialize default item definitions
    let items = vec![
        ItemDefinition {
            item_id: "health_potion".to_string(),
            display_name: "Health Potion".to_string(),
            description: "Restores 50 health points.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 10,
            icon_path: "/Game/UI/Icons/health_potion".to_string(),
        },
        ItemDefinition {
            item_id: "mana_potion".to_string(),
            display_name: "Mana Potion".to_string(),
            description: "Restores 50 mana points.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 10,
            icon_path: "/Game/UI/Icons/mana_potion".to_string(),
        },
        ItemDefinition {
            item_id: "gold_coin".to_string(),
            display_name: "Gold Coin".to_string(),
            description: "Currency.".to_string(),
            item_type: "resource".to_string(),
            max_stack: 999,
            icon_path: "/Game/UI/Icons/gold_coin".to_string(),
        },
        ItemDefinition {
            item_id: "iron_sword".to_string(),
            display_name: "Iron Sword".to_string(),
            description: "A basic iron sword.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/iron_sword".to_string(),
        },
        ItemDefinition {
            item_id: "wooden_shield".to_string(),
            display_name: "Wooden Shield".to_string(),
            description: "A basic wooden shield.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/wooden_shield".to_string(),
        },
    ];

    for item in items {
        ctx.db.item_definition().insert(item);
    }
    log::info!("Item definitions initialized");
}

// ============================================================================
// INSTANCE MANAGEMENT
// ============================================================================

#[reducer]
pub fn create_instance(ctx: &ReducerContext, name: String, max_players: u32, is_public: bool) {
    ctx.db.instance().insert(Instance {
        instance_id: 0,
        name,
        max_players,
        is_public,
        created_at: ctx.timestamp,
        owner_identity: ctx.sender,
    });
    log::info!("Instance created by {:?}", ctx.sender);
}

#[reducer]
pub fn delete_instance(ctx: &ReducerContext, instance_id: u64) {
    if let Some(instance) = ctx.db.instance().instance_id().find(instance_id) {
        if instance.owner_identity == ctx.sender {
            for player in ctx.db.player().iter() {
                if player.instance_id == Some(instance_id) {
                    ctx.db.player().identity().update(Player {
                        instance_id: None,
                        ..player
                    });
                }
            }
            ctx.db.instance().instance_id().delete(instance_id);
            log::info!("Instance {} deleted", instance_id);
        }
    }
}

// ============================================================================
// PLAYER MANAGEMENT
// ============================================================================

#[reducer]
pub fn register_player(ctx: &ReducerContext, username: String) {
    if ctx.db.player().identity().find(ctx.sender).is_some() {
        log::warn!("Player already registered: {:?}", ctx.sender);
        return;
    }

    ctx.db.player().insert(Player {
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
    });
    log::info!("Player registered: {:?}", ctx.sender);
}

#[reducer]
pub fn set_player_online(ctx: &ReducerContext, online: bool) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            is_online: online,
            last_seen: ctx.timestamp,
            ..player
        });
    }
}

#[reducer]
pub fn join_instance(ctx: &ReducerContext, instance_id: u64) {
    let instance = match ctx.db.instance().instance_id().find(instance_id) {
        Some(i) => i,
        None => {
            log::warn!("Instance {} not found", instance_id);
            return;
        }
    };

    let current_players = ctx.db.player().iter()
        .filter(|p| p.instance_id == Some(instance_id) && p.is_online)
        .count() as u32;

    if current_players >= instance.max_players {
        log::warn!("Instance {} is full", instance_id);
        return;
    }

    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            instance_id: Some(instance_id),
            position_x: 0.0,
            position_y: 0.0,
            position_z: 100.0,
            ..player
        });
        log::info!("Player {:?} joined instance {}", ctx.sender, instance_id);
    }
}

#[reducer]
pub fn leave_instance(ctx: &ReducerContext) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            instance_id: None,
            ..player
        });
        log::info!("Player {:?} left instance", ctx.sender);
    }
}

// ============================================================================
// PLAYER SYNC
// ============================================================================

#[reducer]
pub fn update_player_position(
    ctx: &ReducerContext,
    x: f32, y: f32, z: f32,
    pitch: f32, yaw: f32, roll: f32
) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            position_x: x,
            position_y: y,
            position_z: z,
            rotation_pitch: pitch,
            rotation_yaw: yaw,
            rotation_roll: roll,
            last_seen: ctx.timestamp,
            ..player
        });
    }
}

#[reducer]
pub fn update_player_health(ctx: &ReducerContext, health: f32) {
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        ctx.db.player().identity().update(Player {
            health: health.clamp(0.0, player.max_health),
            ..player
        });
    }
}

// ============================================================================
// INVENTORY
// ============================================================================

#[reducer]
pub fn add_item_to_inventory(ctx: &ReducerContext, item_id: String, quantity: u32) {
    let item_def = match ctx.db.item_definition().item_id().find(&item_id) {
        Some(def) => def,
        None => {
            log::warn!("Item definition not found: {}", item_id);
            return;
        }
    };

    // Check existing stack
    for entry in ctx.db.inventory_item().iter() {
        if entry.owner_identity == ctx.sender && entry.item_id == item_id {
            let new_qty = (entry.quantity + quantity).min(item_def.max_stack);
            ctx.db.inventory_item().entry_id().update(InventoryItem {
                quantity: new_qty,
                ..entry
            });
            return;
        }
    }

    // Find next slot
    let mut used_slots: Vec<u32> = ctx.db.inventory_item().iter()
        .filter(|e| e.owner_identity == ctx.sender)
        .map(|e| e.slot_index)
        .collect();
    used_slots.sort();

    let next_slot = (0..100u32).find(|s| !used_slots.contains(s)).unwrap_or(0);

    ctx.db.inventory_item().insert(InventoryItem {
        entry_id: 0,
        owner_identity: ctx.sender,
        item_id,
        quantity: quantity.min(item_def.max_stack),
        slot_index: next_slot,
    });
}

#[reducer]
pub fn remove_item_from_inventory(ctx: &ReducerContext, entry_id: u64, quantity: u32) {
    if let Some(entry) = ctx.db.inventory_item().entry_id().find(entry_id) {
        if entry.owner_identity != ctx.sender {
            return;
        }

        if quantity >= entry.quantity {
            ctx.db.inventory_item().entry_id().delete(entry_id);
        } else {
            ctx.db.inventory_item().entry_id().update(InventoryItem {
                quantity: entry.quantity - quantity,
                ..entry
            });
        }
    }
}

#[reducer]
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
        return;
    }

    // Apply effect
    if let Some(player) = ctx.db.player().identity().find(ctx.sender) {
        match entry.item_id.as_str() {
            "health_potion" => {
                ctx.db.player().identity().update(Player {
                    health: (player.health + 50.0).min(player.max_health),
                    ..player
                });
            }
            "mana_potion" => {
                // Mana system would go here
            }
            _ => {}
        }
    }

    // Remove one
    if entry.quantity <= 1 {
        ctx.db.inventory_item().entry_id().delete(entry_id);
    } else {
        ctx.db.inventory_item().entry_id().update(InventoryItem {
            quantity: entry.quantity - 1,
            ..entry
        });
    }
}

// ============================================================================
// WORLD ITEMS
// ============================================================================

#[reducer]
pub fn collect_world_item(ctx: &ReducerContext, world_item_id: u64) {
    let world_item = match ctx.db.world_item().world_item_id().find(world_item_id) {
        Some(wi) if !wi.is_collected => wi,
        _ => return,
    };

    ctx.db.world_item().world_item_id().update(WorldItem {
        is_collected: true,
        ..world_item.clone()
    });

    // Add to inventory
    let item_def = match ctx.db.item_definition().item_id().find(&world_item.item_id) {
        Some(def) => def,
        None => return,
    };

    for entry in ctx.db.inventory_item().iter() {
        if entry.owner_identity == ctx.sender && entry.item_id == world_item.item_id {
            ctx.db.inventory_item().entry_id().update(InventoryItem {
                quantity: (entry.quantity + world_item.quantity).min(item_def.max_stack),
                ..entry
            });
            return;
        }
    }

    let mut used_slots: Vec<u32> = ctx.db.inventory_item().iter()
        .filter(|e| e.owner_identity == ctx.sender)
        .map(|e| e.slot_index)
        .collect();
    used_slots.sort();

    let next_slot = (0..100u32).find(|s| !used_slots.contains(s)).unwrap_or(0);

    ctx.db.inventory_item().insert(InventoryItem {
        entry_id: 0,
        owner_identity: ctx.sender,
        item_id: world_item.item_id,
        quantity: world_item.quantity.min(item_def.max_stack),
        slot_index: next_slot,
    });
}

// ============================================================================
// INTERACTABLES
// ============================================================================

#[reducer]
pub fn toggle_interactable(ctx: &ReducerContext, interactable_id: String) {
    if let Some(existing) = ctx.db.interactable_state().interactable_id().find(&interactable_id) {
        ctx.db.interactable_state().interactable_id().update(InteractableState {
            is_active: !existing.is_active,
            ..existing
        });
    }
}
