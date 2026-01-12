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
    // Phase 9: Premium item fields
    pub is_premium: bool,
    pub premium_currency_price: u32,  // Price in premium currency (0 = not for sale)
    pub is_exclusive: bool,           // True = can only be obtained via cash shop
    pub rarity: u8,                   // 0=Common, 1=Uncommon, 2=Rare, 3=Epic, 4=Legendary
}

// ============================================================================
// PHASE 9: PREMIUM SYSTEM TABLES
// ============================================================================

/// Tracks player's premium currency balance
#[derive(Clone)]
#[table(name = player_wallet, public)]
pub struct PlayerWallet {
    #[primary_key]
    pub identity: Identity,
    pub premium_currency: u64,        // Gems, Crystals, etc.
    pub lifetime_purchased: u64,      // Total ever purchased (for VIP tiers)
    pub last_purchase_at: Timestamp,
}

/// Permanent record of premium item ownership - survives client reinstalls
#[table(name = premium_ownership, public)]
pub struct PremiumOwnership {
    #[primary_key]
    #[auto_inc]
    pub ownership_id: u64,
    pub owner_identity: Identity,
    pub item_id: String,
    pub acquired_at: Timestamp,
    pub transaction_id: String,       // Reference to purchase transaction
    pub is_gift: bool,                // True if received as gift
    pub gifted_by: Option<Identity>,  // Who gifted it (if applicable)
}

/// Transaction log for all premium purchases (audit trail)
#[table(name = premium_transaction, public)]
pub struct PremiumTransaction {
    #[primary_key]
    #[auto_inc]
    pub transaction_id: u64,
    pub buyer_identity: Identity,
    pub item_id: String,
    pub currency_spent: u32,
    pub transaction_type: String,     // "purchase", "gift", "refund", "grant"
    pub timestamp: Timestamp,
    pub receipt_id: String,           // External receipt from app store
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
    // Initialize default item definitions (non-premium)
    let standard_items = vec![
        ItemDefinition {
            item_id: "health_potion".to_string(),
            display_name: "Health Potion".to_string(),
            description: "Restores 50 health points.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 10,
            icon_path: "/Game/UI/Icons/health_potion".to_string(),
            is_premium: false,
            premium_currency_price: 0,
            is_exclusive: false,
            rarity: 0,
        },
        ItemDefinition {
            item_id: "mana_potion".to_string(),
            display_name: "Mana Potion".to_string(),
            description: "Restores 50 mana points.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 10,
            icon_path: "/Game/UI/Icons/mana_potion".to_string(),
            is_premium: false,
            premium_currency_price: 0,
            is_exclusive: false,
            rarity: 0,
        },
        ItemDefinition {
            item_id: "gold_coin".to_string(),
            display_name: "Gold Coin".to_string(),
            description: "Currency.".to_string(),
            item_type: "resource".to_string(),
            max_stack: 999,
            icon_path: "/Game/UI/Icons/gold_coin".to_string(),
            is_premium: false,
            premium_currency_price: 0,
            is_exclusive: false,
            rarity: 0,
        },
        ItemDefinition {
            item_id: "iron_sword".to_string(),
            display_name: "Iron Sword".to_string(),
            description: "A basic iron sword.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/iron_sword".to_string(),
            is_premium: false,
            premium_currency_price: 0,
            is_exclusive: false,
            rarity: 1,
        },
        ItemDefinition {
            item_id: "wooden_shield".to_string(),
            display_name: "Wooden Shield".to_string(),
            description: "A basic wooden shield.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/wooden_shield".to_string(),
            is_premium: false,
            premium_currency_price: 0,
            is_exclusive: false,
            rarity: 0,
        },
    ];

    // Phase 9: Premium/Cash Shop Items
    let premium_items = vec![
        // ===== EXCLUSIVE COSMETIC WEAPONS =====
        ItemDefinition {
            item_id: "celestial_blade".to_string(),
            display_name: "Celestial Blade".to_string(),
            description: "A legendary sword forged from starlight. Glows with ethereal energy.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/celestial_blade".to_string(),
            is_premium: true,
            premium_currency_price: 1500,
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        ItemDefinition {
            item_id: "shadow_dagger".to_string(),
            display_name: "Shadow Dagger".to_string(),
            description: "A dagger that seems to absorb light itself. Leaves trails of darkness.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/shadow_dagger".to_string(),
            is_premium: true,
            premium_currency_price: 800,
            is_exclusive: true,
            rarity: 3, // Epic
        },
        ItemDefinition {
            item_id: "phoenix_staff".to_string(),
            display_name: "Phoenix Staff".to_string(),
            description: "A staff crowned with eternal flames. Said to grant rebirth to its wielder.".to_string(),
            item_type: "weapon".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/phoenix_staff".to_string(),
            is_premium: true,
            premium_currency_price: 1200,
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        // ===== EXCLUSIVE ARMOR SETS =====
        ItemDefinition {
            item_id: "dragonscale_helm".to_string(),
            display_name: "Dragonscale Helm".to_string(),
            description: "Helmet forged from ancient dragon scales. Part of the Dragonscale set.".to_string(),
            item_type: "armor".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/dragonscale_helm".to_string(),
            is_premium: true,
            premium_currency_price: 600,
            is_exclusive: true,
            rarity: 3, // Epic
        },
        ItemDefinition {
            item_id: "dragonscale_armor".to_string(),
            display_name: "Dragonscale Armor".to_string(),
            description: "Chestplate forged from ancient dragon scales. Shimmers with inner fire.".to_string(),
            item_type: "armor".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/dragonscale_armor".to_string(),
            is_premium: true,
            premium_currency_price: 1000,
            is_exclusive: true,
            rarity: 3, // Epic
        },
        ItemDefinition {
            item_id: "void_walker_boots".to_string(),
            display_name: "Void Walker Boots".to_string(),
            description: "Boots that let you walk between dimensions. Leaves shadow footprints.".to_string(),
            item_type: "armor".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/void_walker_boots".to_string(),
            is_premium: true,
            premium_currency_price: 500,
            is_exclusive: true,
            rarity: 3, // Epic
        },
        // ===== EXCLUSIVE ACCESSORIES =====
        ItemDefinition {
            item_id: "ring_of_eternity".to_string(),
            display_name: "Ring of Eternity".to_string(),
            description: "An ancient ring that pulses with time magic. Grants +10% XP bonus.".to_string(),
            item_type: "accessory".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/ring_of_eternity".to_string(),
            is_premium: true,
            premium_currency_price: 2000,
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        ItemDefinition {
            item_id: "amulet_of_fortune".to_string(),
            display_name: "Amulet of Fortune".to_string(),
            description: "A lucky charm that increases gold drops by 15%.".to_string(),
            item_type: "accessory".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/amulet_of_fortune".to_string(),
            is_premium: true,
            premium_currency_price: 1500,
            is_exclusive: true,
            rarity: 3, // Epic
        },
        // ===== PREMIUM CONSUMABLES (not exclusive, can be found rarely) =====
        ItemDefinition {
            item_id: "elixir_of_power".to_string(),
            display_name: "Elixir of Power".to_string(),
            description: "Doubles damage output for 5 minutes.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 5,
            icon_path: "/Game/UI/Icons/Premium/elixir_of_power".to_string(),
            is_premium: true,
            premium_currency_price: 100,
            is_exclusive: false, // Can also drop from bosses
            rarity: 2, // Rare
        },
        ItemDefinition {
            item_id: "scroll_of_resurrection".to_string(),
            display_name: "Scroll of Resurrection".to_string(),
            description: "Instantly revive at your current location with full health.".to_string(),
            item_type: "consumable".to_string(),
            max_stack: 3,
            icon_path: "/Game/UI/Icons/Premium/scroll_of_resurrection".to_string(),
            is_premium: true,
            premium_currency_price: 200,
            is_exclusive: false,
            rarity: 3, // Epic
        },
        // ===== PREMIUM MOUNTS/COMPANIONS (exclusive) =====
        ItemDefinition {
            item_id: "spirit_wolf_whistle".to_string(),
            display_name: "Spirit Wolf Whistle".to_string(),
            description: "Summons a spectral wolf mount. 50% speed bonus.".to_string(),
            item_type: "mount".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/spirit_wolf".to_string(),
            is_premium: true,
            premium_currency_price: 2500,
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        ItemDefinition {
            item_id: "baby_dragon_egg".to_string(),
            display_name: "Baby Dragon Egg".to_string(),
            description: "A companion that follows you and occasionally breathes fire at enemies.".to_string(),
            item_type: "companion".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/baby_dragon".to_string(),
            is_premium: true,
            premium_currency_price: 3000,
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        // ===== STARTER PACKS (bundles represented as single items) =====
        ItemDefinition {
            item_id: "founders_pack_token".to_string(),
            display_name: "Founder's Pack".to_string(),
            description: "Exclusive founder's pack. Contains: Founder's Sword, 1000 Gems, Founder Title.".to_string(),
            item_type: "bundle".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/founders_pack".to_string(),
            is_premium: true,
            premium_currency_price: 4999, // $49.99 equivalent
            is_exclusive: true,
            rarity: 4, // Legendary
        },
        ItemDefinition {
            item_id: "battle_pass_season1".to_string(),
            display_name: "Battle Pass - Season 1".to_string(),
            description: "Unlock exclusive seasonal rewards as you play.".to_string(),
            item_type: "pass".to_string(),
            max_stack: 1,
            icon_path: "/Game/UI/Icons/Premium/battle_pass_s1".to_string(),
            is_premium: true,
            premium_currency_price: 950,
            is_exclusive: true,
            rarity: 3, // Epic
        },
    ];

    for item in standard_items {
        ctx.db.item_definition().insert(item);
    }

    for item in premium_items {
        ctx.db.item_definition().insert(item);
    }

    log::info!("Item definitions initialized ({} standard, {} premium)", 5, 14);
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

// ============================================================================
// PHASE 9: PREMIUM SYSTEM REDUCERS
// ============================================================================

/// Initialize or get a player's wallet (creates if doesn't exist)
fn get_or_create_wallet(ctx: &ReducerContext, identity: Identity) -> PlayerWallet {
    if let Some(wallet) = ctx.db.player_wallet().identity().find(identity) {
        wallet
    } else {
        let new_wallet = PlayerWallet {
            identity,
            premium_currency: 0,
            lifetime_purchased: 0,
            last_purchase_at: ctx.timestamp,
        };
        ctx.db.player_wallet().insert(new_wallet.clone());
        new_wallet
    }
}

/// Add premium currency to player's wallet (called after IAP verification)
#[reducer]
pub fn add_premium_currency(ctx: &ReducerContext, amount: u64, receipt_id: String) {
    let wallet = get_or_create_wallet(ctx, ctx.sender);

    ctx.db.player_wallet().identity().update(PlayerWallet {
        premium_currency: wallet.premium_currency + amount,
        lifetime_purchased: wallet.lifetime_purchased + amount,
        last_purchase_at: ctx.timestamp,
        ..wallet
    });

    // Log the transaction for audit
    ctx.db.premium_transaction().insert(PremiumTransaction {
        transaction_id: 0,
        buyer_identity: ctx.sender,
        item_id: "currency_purchase".to_string(),
        currency_spent: 0, // No currency spent, currency was added
        transaction_type: "currency_add".to_string(),
        timestamp: ctx.timestamp,
        receipt_id,
    });

    log::info!("Added {} premium currency to {:?}", amount, ctx.sender);
}

/// Purchase a premium item with premium currency
#[reducer]
pub fn purchase_premium_item(ctx: &ReducerContext, item_id: String) {
    // Verify item exists and is premium
    let item_def = match ctx.db.item_definition().item_id().find(&item_id) {
        Some(def) if def.is_premium && def.premium_currency_price > 0 => def,
        Some(_) => {
            log::warn!("Item {} is not a premium item or not for sale", item_id);
            return;
        }
        None => {
            log::warn!("Item {} not found", item_id);
            return;
        }
    };

    // Check if player already owns this item (for exclusive items)
    if item_def.is_exclusive {
        let already_owned = ctx.db.premium_ownership().iter()
            .any(|o| o.owner_identity == ctx.sender && o.item_id == item_id);
        if already_owned {
            log::warn!("Player {:?} already owns exclusive item {}", ctx.sender, item_id);
            return;
        }
    }

    // Check wallet balance
    let wallet = get_or_create_wallet(ctx, ctx.sender);
    let price = item_def.premium_currency_price as u64;

    if wallet.premium_currency < price {
        log::warn!("Insufficient premium currency for {:?}: has {}, needs {}",
            ctx.sender, wallet.premium_currency, price);
        return;
    }

    // Deduct currency
    ctx.db.player_wallet().identity().update(PlayerWallet {
        premium_currency: wallet.premium_currency - price,
        ..wallet
    });

    // Create transaction record
    let tx_id = format!("tx_{}_{}", ctx.timestamp.to_micros_since_unix_epoch(), ctx.sender);
    ctx.db.premium_transaction().insert(PremiumTransaction {
        transaction_id: 0,
        buyer_identity: ctx.sender,
        item_id: item_id.clone(),
        currency_spent: item_def.premium_currency_price,
        transaction_type: "purchase".to_string(),
        timestamp: ctx.timestamp,
        receipt_id: tx_id.clone(),
    });

    // Create permanent ownership record
    ctx.db.premium_ownership().insert(PremiumOwnership {
        ownership_id: 0,
        owner_identity: ctx.sender,
        item_id: item_id.clone(),
        acquired_at: ctx.timestamp,
        transaction_id: tx_id,
        is_gift: false,
        gifted_by: None,
    });

    // Add to inventory
    add_premium_to_inventory(ctx, ctx.sender, &item_id, &item_def);

    log::info!("Player {:?} purchased premium item {}", ctx.sender, item_id);
}

/// Helper to add a premium item to player's inventory
fn add_premium_to_inventory(ctx: &ReducerContext, identity: Identity, item_id: &str, item_def: &ItemDefinition) {
    // Check for existing stack (for consumables)
    if item_def.max_stack > 1 {
        for entry in ctx.db.inventory_item().iter() {
            if entry.owner_identity == identity && entry.item_id == item_id {
                let new_qty = (entry.quantity + 1).min(item_def.max_stack);
                ctx.db.inventory_item().entry_id().update(InventoryItem {
                    quantity: new_qty,
                    ..entry
                });
                return;
            }
        }
    }

    // Find next available slot
    let mut used_slots: Vec<u32> = ctx.db.inventory_item().iter()
        .filter(|e| e.owner_identity == identity)
        .map(|e| e.slot_index)
        .collect();
    used_slots.sort();

    let next_slot = (0..100u32).find(|s| !used_slots.contains(s)).unwrap_or(0);

    ctx.db.inventory_item().insert(InventoryItem {
        entry_id: 0,
        owner_identity: identity,
        item_id: item_id.to_string(),
        quantity: 1,
        slot_index: next_slot,
    });
}

/// Gift a premium item to another player (costs the gifter)
#[reducer]
pub fn gift_premium_item(ctx: &ReducerContext, item_id: String, recipient: Identity) {
    // Can't gift to yourself
    if recipient == ctx.sender {
        log::warn!("Cannot gift items to yourself");
        return;
    }

    // Verify recipient exists
    if ctx.db.player().identity().find(recipient).is_none() {
        log::warn!("Recipient player not found: {:?}", recipient);
        return;
    }

    // Verify item exists and is premium
    let item_def = match ctx.db.item_definition().item_id().find(&item_id) {
        Some(def) if def.is_premium && def.premium_currency_price > 0 => def,
        Some(_) => {
            log::warn!("Item {} is not giftable", item_id);
            return;
        }
        None => {
            log::warn!("Item {} not found", item_id);
            return;
        }
    };

    // Check if recipient already owns this exclusive item
    if item_def.is_exclusive {
        let already_owned = ctx.db.premium_ownership().iter()
            .any(|o| o.owner_identity == recipient && o.item_id == item_id);
        if already_owned {
            log::warn!("Recipient {:?} already owns exclusive item {}", recipient, item_id);
            return;
        }
    }

    // Check wallet balance (gifts cost same as purchase)
    let wallet = get_or_create_wallet(ctx, ctx.sender);
    let price = item_def.premium_currency_price as u64;

    if wallet.premium_currency < price {
        log::warn!("Insufficient premium currency for gift");
        return;
    }

    // Deduct currency from gifter
    ctx.db.player_wallet().identity().update(PlayerWallet {
        premium_currency: wallet.premium_currency - price,
        ..wallet
    });

    // Create transaction record
    let tx_id = format!("gift_{}_{}", ctx.timestamp.to_micros_since_unix_epoch(), ctx.sender);
    ctx.db.premium_transaction().insert(PremiumTransaction {
        transaction_id: 0,
        buyer_identity: ctx.sender,
        item_id: item_id.clone(),
        currency_spent: item_def.premium_currency_price,
        transaction_type: "gift".to_string(),
        timestamp: ctx.timestamp,
        receipt_id: tx_id.clone(),
    });

    // Create ownership record for recipient
    ctx.db.premium_ownership().insert(PremiumOwnership {
        ownership_id: 0,
        owner_identity: recipient,
        item_id: item_id.clone(),
        acquired_at: ctx.timestamp,
        transaction_id: tx_id,
        is_gift: true,
        gifted_by: Some(ctx.sender),
    });

    // Add to recipient's inventory
    add_premium_to_inventory(ctx, recipient, &item_id, &item_def);

    log::info!("Player {:?} gifted {} to {:?}", ctx.sender, item_id, recipient);
}

/// Reclaim all owned premium items after client reinstall
/// This restores premium items that were lost due to local data deletion
#[reducer]
pub fn reclaim_premium_items(ctx: &ReducerContext) {
    let mut reclaimed_count = 0;

    // Find all premium items this player owns
    let owned_items: Vec<PremiumOwnership> = ctx.db.premium_ownership().iter()
        .filter(|o| o.owner_identity == ctx.sender)
        .collect();

    for ownership in owned_items {
        // Get item definition
        let item_def = match ctx.db.item_definition().item_id().find(&ownership.item_id) {
            Some(def) => def,
            None => continue,
        };

        // For exclusive items (non-consumables), check if already in inventory
        if item_def.is_exclusive && item_def.max_stack == 1 {
            let already_in_inventory = ctx.db.inventory_item().iter()
                .any(|i| i.owner_identity == ctx.sender && i.item_id == ownership.item_id);

            if already_in_inventory {
                continue; // Already have it
            }
        }

        // Add to inventory
        add_premium_to_inventory(ctx, ctx.sender, &ownership.item_id, &item_def);
        reclaimed_count += 1;
    }

    log::info!("Player {:?} reclaimed {} premium items", ctx.sender, reclaimed_count);
}

/// Admin reducer to grant premium items (for support, promotions, etc.)
#[reducer]
pub fn admin_grant_premium_item(ctx: &ReducerContext, recipient: Identity, item_id: String, reason: String) {
    // In production, this should check admin permissions
    // For now, we log the grant action

    let item_def = match ctx.db.item_definition().item_id().find(&item_id) {
        Some(def) if def.is_premium => def,
        _ => {
            log::warn!("Item {} not found or not premium", item_id);
            return;
        }
    };

    // Create transaction record
    let tx_id = format!("grant_{}_{}", ctx.timestamp.to_micros_since_unix_epoch(), ctx.sender);
    ctx.db.premium_transaction().insert(PremiumTransaction {
        transaction_id: 0,
        buyer_identity: ctx.sender, // Admin who granted
        item_id: item_id.clone(),
        currency_spent: 0, // Free grant
        transaction_type: "grant".to_string(),
        timestamp: ctx.timestamp,
        receipt_id: format!("{}:{}", tx_id, reason),
    });

    // Create ownership record
    ctx.db.premium_ownership().insert(PremiumOwnership {
        ownership_id: 0,
        owner_identity: recipient,
        item_id: item_id.clone(),
        acquired_at: ctx.timestamp,
        transaction_id: tx_id,
        is_gift: false,
        gifted_by: None,
    });

    // Add to inventory
    add_premium_to_inventory(ctx, recipient, &item_id, &item_def);

    log::info!("Admin {:?} granted {} to {:?} (reason: {})", ctx.sender, item_id, recipient, reason);
}

/// Get player's wallet balance (read-only query would be better, but reducer works)
#[reducer]
pub fn get_wallet_balance(ctx: &ReducerContext) {
    let wallet = get_or_create_wallet(ctx, ctx.sender);
    log::info!("Wallet balance for {:?}: {} premium currency", ctx.sender, wallet.premium_currency);
}
