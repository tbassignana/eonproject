// Copyright 2026 tbassignana. MIT License.
// Unit Tests for Eon Project Core Components

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// ============================================================================
// INVENTORY COMPONENT TESTS - ORIGINAL
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAddItemTest,
    "Eon.Inventory.AddItem",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRemoveItemTest,
    "Eon.Inventory.RemoveItem",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryStackingTest,
    "Eon.Inventory.Stacking",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventorySlotManagementTest,
    "Eon.Inventory.SlotManagement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// PHASE 8 INVENTORY TESTS
// ============================================================================

// 8.1 Weight System
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryWeightSystemTest,
    "Eon.Inventory.Phase8.WeightSystem",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.2 Category Filtering
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCategoryFilterTest,
    "Eon.Inventory.Phase8.CategoryFilter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.3 Sorting
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventorySortingTest,
    "Eon.Inventory.Phase8.Sorting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.4 Stack Splitting
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryStackSplitTest,
    "Eon.Inventory.Phase8.StackSplit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.5 Stack Combining
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryStackCombineTest,
    "Eon.Inventory.Phase8.StackCombine",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.6 Quick Slots
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryQuickSlotsTest,
    "Eon.Inventory.Phase8.QuickSlots",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.7 Tooltips
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTooltipsTest,
    "Eon.Inventory.Phase8.Tooltips",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.8 Rarity System
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRarityTest,
    "Eon.Inventory.Phase8.Rarity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.9 Durability System
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryDurabilityTest,
    "Eon.Inventory.Phase8.Durability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.10 Auto-Sort
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAutoSortTest,
    "Eon.Inventory.Phase8.AutoSort",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.11 Search
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventorySearchTest,
    "Eon.Inventory.Phase8.Search",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.12 Equipment Slots
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryEquipmentTest,
    "Eon.Inventory.Phase8.Equipment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.13 Item Comparison
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryComparisonTest,
    "Eon.Inventory.Phase8.Comparison",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.14 Bulk Operations
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryBulkOpsTest,
    "Eon.Inventory.Phase8.BulkOperations",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.15 Local Persistence
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryPersistenceTest,
    "Eon.Inventory.Phase8.Persistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.16 Overflow Handling
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryOverflowTest,
    "Eon.Inventory.Phase8.Overflow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.17 Item Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryValidationTest,
    "Eon.Inventory.Phase8.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.18 Transaction Logging
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransactionLogTest,
    "Eon.Inventory.Phase8.TransactionLog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.19 Capacity Expansion
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCapacityTest,
    "Eon.Inventory.Phase8.Capacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// 8.20 Favorites and Locking
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryFavoritesLockTest,
    "Eon.Inventory.Phase8.FavoritesLock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// CHARACTER TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterHealthTest,
    "Eon.Character.Health",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDamageTest,
    "Eon.Character.Damage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterHealTest,
    "Eon.Character.Heal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

// ============================================================================
// INTERACTION TESTS
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionScanTest,
    "Eon.Interaction.Scan",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionPromptTest,
    "Eon.Interaction.Prompt",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
