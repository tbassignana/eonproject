// Copyright 2026 tbassignana. MIT License.
// Unit Tests Implementation for Eon Project

#include "EonTests.h"
#include "InventoryComponent.h"
#include "EonCharacter.h"
#include "InteractionComponent.h"

// ============================================================================
// INVENTORY COMPONENT TESTS
// ============================================================================

bool FInventoryAddItemTest::RunTest(const FString& Parameters)
{
    // Create a temporary inventory component for testing
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test adding an item
    Inventory->AddItem(TEXT("health_potion"), 5);

    // Verify item was added
    bool bHasItem = Inventory->HasItem(TEXT("health_potion"), 5);
    TestTrue(TEXT("Item should be in inventory after adding"), bHasItem);

    // Test item count
    int32 Count = Inventory->GetItemCount(TEXT("health_potion"));
    TestEqual(TEXT("Item count should match added quantity"), Count, 5);

    return true;
}

bool FInventoryRemoveItemTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add item first
    Inventory->AddItem(TEXT("gold_coin"), 100);
    TestTrue(TEXT("Item added successfully"), Inventory->HasItem(TEXT("gold_coin"), 100));

    // Get the entry to remove (simplified - would need actual entry ID)
    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    if (Items.Num() > 0)
    {
        // Remove some items
        Inventory->RemoveItem(Items[0].EntryId, 50);
        int32 Remaining = Inventory->GetItemCount(TEXT("gold_coin"));
        TestEqual(TEXT("Should have 50 remaining after removing 50"), Remaining, 50);
    }

    return true;
}

bool FInventoryStackingTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add items in multiple calls
    Inventory->AddItem(TEXT("mana_potion"), 3);
    Inventory->AddItem(TEXT("mana_potion"), 2);

    // Should stack to 5
    int32 Count = Inventory->GetItemCount(TEXT("mana_potion"));
    TestEqual(TEXT("Items should stack together"), Count, 5);

    return true;
}

bool FInventorySlotManagementTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add multiple different items
    Inventory->AddItem(TEXT("health_potion"), 1);
    Inventory->AddItem(TEXT("iron_sword"), 1);
    Inventory->AddItem(TEXT("wooden_shield"), 1);

    // Check slot assignments
    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    TestEqual(TEXT("Should have 3 items in inventory"), Items.Num(), 3);

    // Verify each item has unique slot
    TSet<int32> UsedSlots;
    for (const FInventorySlot& Slot : Items)
    {
        TestFalse(TEXT("Each item should have unique slot"), UsedSlots.Contains(Slot.SlotIndex));
        UsedSlots.Add(Slot.SlotIndex);
    }

    return true;
}

// ============================================================================
// CHARACTER TESTS
// ============================================================================

bool FCharacterHealthTest::RunTest(const FString& Parameters)
{
    // Test health initialization
    AEonCharacter* Character = NewObject<AEonCharacter>();

    float MaxHealth = Character->GetMaxHealth();
    float CurrentHealth = Character->GetHealth();

    TestEqual(TEXT("Initial health should equal max health"), CurrentHealth, MaxHealth);
    TestEqual(TEXT("Max health should be 100"), MaxHealth, 100.0f);

    return true;
}

bool FCharacterDamageTest::RunTest(const FString& Parameters)
{
    AEonCharacter* Character = NewObject<AEonCharacter>();

    float InitialHealth = Character->GetHealth();
    Character->ApplyDamage(25.0f);

    float NewHealth = Character->GetHealth();
    TestEqual(TEXT("Health should decrease by damage amount"), NewHealth, InitialHealth - 25.0f);

    // Test death
    Character->ApplyDamage(1000.0f);
    TestTrue(TEXT("Character should be dead after lethal damage"), Character->IsDead());

    return true;
}

bool FCharacterHealTest::RunTest(const FString& Parameters)
{
    AEonCharacter* Character = NewObject<AEonCharacter>();

    // Damage first
    Character->ApplyDamage(50.0f);
    float DamagedHealth = Character->GetHealth();

    // Heal
    Character->Heal(25.0f);
    float HealedHealth = Character->GetHealth();

    TestEqual(TEXT("Health should increase by heal amount"), HealedHealth, DamagedHealth + 25.0f);

    // Test heal capping at max
    Character->Heal(1000.0f);
    TestEqual(TEXT("Health should not exceed max"), Character->GetHealth(), Character->GetMaxHealth());

    return true;
}

// ============================================================================
// INTERACTION TESTS
// ============================================================================

bool FInteractionScanTest::RunTest(const FString& Parameters)
{
    UInteractionComponent* Interaction = NewObject<UInteractionComponent>();

    // Without any interactables nearby, current should be null
    AActor* Current = Interaction->GetCurrentInteractable();
    TestNull(TEXT("Should have no interactable initially"), Current);

    return true;
}

bool FInteractionPromptTest::RunTest(const FString& Parameters)
{
    UInteractionComponent* Interaction = NewObject<UInteractionComponent>();

    // Without interactable, prompt should be empty
    FString Prompt = Interaction->GetInteractionPrompt();
    TestTrue(TEXT("Prompt should be empty with no interactable"), Prompt.IsEmpty());

    return true;
}
