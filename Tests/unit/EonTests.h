// Copyright 2026 tbassignana. MIT License.
// Unit Tests for Eon Project Core Components

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// ============================================================================
// INVENTORY COMPONENT TESTS
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
