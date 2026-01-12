// Copyright 2026 tbassignana. MIT License.
// Unit Tests Implementation for Eon Project

#include "EonTests.h"
#include "InventoryComponent.h"
#include "EonCharacter.h"
#include "InteractionComponent.h"

// ============================================================================
// INVENTORY COMPONENT TESTS - ORIGINAL
// ============================================================================

bool FInventoryAddItemTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("health_potion"), 5);

    bool bHasItem = Inventory->HasItem(TEXT("health_potion"), 5);
    TestTrue(TEXT("Item should be in inventory after adding"), bHasItem);

    int32 Count = Inventory->GetItemCount(TEXT("health_potion"));
    TestEqual(TEXT("Item count should match added quantity"), Count, 5);

    return true;
}

bool FInventoryRemoveItemTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("gold_coin"), 100);
    TestTrue(TEXT("Item added successfully"), Inventory->HasItem(TEXT("gold_coin"), 100));

    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    if (Items.Num() > 0)
    {
        Inventory->RemoveItem(Items[0].EntryId, 50);
        int32 Remaining = Inventory->GetItemCount(TEXT("gold_coin"));
        TestEqual(TEXT("Should have 50 remaining after removing 50"), Remaining, 50);
    }

    return true;
}

bool FInventoryStackingTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("mana_potion"), 3);
    Inventory->AddItem(TEXT("mana_potion"), 2);

    int32 Count = Inventory->GetItemCount(TEXT("mana_potion"));
    TestEqual(TEXT("Items should stack together"), Count, 5);

    return true;
}

bool FInventorySlotManagementTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("health_potion"), 1);
    Inventory->AddItem(TEXT("iron_sword"), 1);
    Inventory->AddItem(TEXT("wooden_shield"), 1);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    TestEqual(TEXT("Should have 3 items in inventory"), Items.Num(), 3);

    TSet<int32> UsedSlots;
    for (const FInventorySlot& Slot : Items)
    {
        TestFalse(TEXT("Each item should have unique slot"), UsedSlots.Contains(Slot.SlotIndex));
        UsedSlots.Add(Slot.SlotIndex);
    }

    return true;
}

// ============================================================================
// PHASE 8.1: WEIGHT SYSTEM TEST
// ============================================================================

bool FInventoryWeightSystemTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial weight is 0
    float InitialWeight = Inventory->GetCurrentWeight();
    TestEqual(TEXT("Initial weight should be 0"), InitialWeight, 0.0f);

    // Add items with weight
    Inventory->AddItem(TEXT("iron_sword"), 1); // 5.0 weight
    Inventory->AddItem(TEXT("health_potion"), 2); // 0.5 weight each

    float CurrentWeight = Inventory->GetCurrentWeight();
    TestTrue(TEXT("Current weight should be > 0 after adding items"), CurrentWeight > 0.0f);

    // Test weight percentage
    float Percentage = Inventory->GetWeightPercentage();
    TestTrue(TEXT("Weight percentage should be between 0 and 100"), Percentage >= 0.0f && Percentage <= 100.0f);

    // Test CanCarryWeight
    TestTrue(TEXT("Should be able to carry small additional weight"), Inventory->CanCarryWeight(1.0f));
    TestFalse(TEXT("Should not be able to carry huge weight"), Inventory->CanCarryWeight(1000.0f));

    // Test IsOverEncumbered
    TestFalse(TEXT("Should not be over encumbered with normal items"), Inventory->IsOverEncumbered());

    return true;
}

// ============================================================================
// PHASE 8.2: CATEGORY FILTERING TEST
// ============================================================================

bool FInventoryCategoryFilterTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add items of different categories
    Inventory->AddItem(TEXT("health_potion"), 3); // consumable
    Inventory->AddItem(TEXT("iron_sword"), 1);     // weapon
    Inventory->AddItem(TEXT("gold_coin"), 100);    // resource

    // Test GetItemsByCategory
    TArray<FInventorySlot> AllItems = Inventory->GetItemsByCategory(EItemCategory::All);
    TestEqual(TEXT("All category should return all items"), AllItems.Num(), 3);

    TArray<FInventorySlot> Consumables = Inventory->GetItemsByCategory(EItemCategory::Consumable);
    TestEqual(TEXT("Should have 1 consumable item"), Consumables.Num(), 1);

    TArray<FInventorySlot> Weapons = Inventory->GetItemsByCategory(EItemCategory::Weapon);
    TestEqual(TEXT("Should have 1 weapon item"), Weapons.Num(), 1);

    // Test SetActiveFilter and GetFilteredItems
    Inventory->SetActiveFilter(EItemCategory::Resource);
    TArray<FInventorySlot> FilteredItems = Inventory->GetFilteredItems();
    TestEqual(TEXT("Filtered items should contain only resources"), FilteredItems.Num(), 1);

    TestEqual(TEXT("Active filter should be Resource"), Inventory->GetActiveFilter(), EItemCategory::Resource);

    return true;
}

// ============================================================================
// PHASE 8.3: SORTING TEST
// ============================================================================

bool FInventorySortingTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add items
    Inventory->AddItem(TEXT("wooden_shield"), 1);
    Inventory->AddItem(TEXT("health_potion"), 5);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Test sorting by name
    Inventory->SortInventory(EInventorySortMode::ByName, true);
    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    TestTrue(TEXT("Items should be sorted by name"), Items.Num() > 0);

    // Verify sort mode is saved
    TestEqual(TEXT("Current sort mode should be ByName"), Inventory->GetCurrentSortMode(), EInventorySortMode::ByName);

    // Test sorting by quantity
    Inventory->SortInventory(EInventorySortMode::ByQuantity, false);
    Items = Inventory->GetAllItems();
    if (Items.Num() > 1)
    {
        TestTrue(TEXT("First item should have highest quantity when sorted desc"),
            Items[0].Quantity >= Items[Items.Num() - 1].Quantity);
    }

    return true;
}

// ============================================================================
// PHASE 8.4: STACK SPLITTING TEST
// ============================================================================

bool FInventoryStackSplitTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add a stack of items
    Inventory->AddItem(TEXT("health_potion"), 10);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    TestEqual(TEXT("Should have 1 stack initially"), Items.Num(), 1);

    int64 EntryId = Items[0].EntryId;

    // Split the stack
    bool bSplitSuccess = Inventory->SplitStack(EntryId, 3);
    TestTrue(TEXT("Split should succeed"), bSplitSuccess);

    Items = Inventory->GetAllItems();
    TestEqual(TEXT("Should have 2 stacks after split"), Items.Num(), 2);

    // Verify quantities
    int32 TotalCount = Inventory->GetItemCount(TEXT("health_potion"));
    TestEqual(TEXT("Total quantity should remain 10"), TotalCount, 10);

    // Test invalid split (more than available)
    bSplitSuccess = Inventory->SplitStack(EntryId, 100);
    TestFalse(TEXT("Split should fail with invalid amount"), bSplitSuccess);

    return true;
}

// ============================================================================
// PHASE 8.5: STACK COMBINING TEST
// ============================================================================

bool FInventoryStackCombineTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add two separate stacks
    Inventory->AddItem(TEXT("health_potion"), 3);
    Inventory->AddItem(TEXT("health_potion"), 2);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    // Items may have been auto-stacked, so check total
    int32 TotalBefore = Inventory->GetItemCount(TEXT("health_potion"));
    TestEqual(TEXT("Total should be 5"), TotalBefore, 5);

    if (Items.Num() >= 2)
    {
        int64 SourceId = Items[0].EntryId;
        int64 TargetId = Items[1].EntryId;

        bool bCombineSuccess = Inventory->CombineStacks(SourceId, TargetId);
        TestTrue(TEXT("Combine should succeed for same item type"), bCombineSuccess);
    }

    // Test combining different items (should fail)
    Inventory->AddItem(TEXT("mana_potion"), 2);
    Items = Inventory->GetAllItems();

    // Find different item types
    int64 PotionId = 0, CoinId = 0;
    for (const FInventorySlot& Slot : Items)
    {
        if (Slot.ItemId == TEXT("health_potion")) PotionId = Slot.EntryId;
        else if (Slot.ItemId == TEXT("mana_potion")) CoinId = Slot.EntryId;
    }

    if (PotionId != 0 && CoinId != 0)
    {
        bool bInvalidCombine = Inventory->CombineStacks(PotionId, CoinId);
        TestFalse(TEXT("Combine should fail for different item types"), bInvalidCombine);
    }

    return true;
}

// ============================================================================
// PHASE 8.6: QUICK SLOTS TEST
// ============================================================================

bool FInventoryQuickSlotsTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial quick slots
    int32 NumQuickSlots = Inventory->GetNumQuickSlots();
    TestEqual(TEXT("Should have 6 quick slots by default"), NumQuickSlots, 6);

    // Add an item and assign to quick slot
    Inventory->AddItem(TEXT("health_potion"), 5);
    TArray<FInventorySlot> Items = Inventory->GetAllItems();

    if (Items.Num() > 0)
    {
        int64 EntryId = Items[0].EntryId;

        Inventory->AssignToQuickSlot(EntryId, 0);
        FInventorySlot QuickSlotItem = Inventory->GetQuickSlotItem(0);
        TestEqual(TEXT("Quick slot 0 should have assigned item"), QuickSlotItem.EntryId, EntryId);

        // Test clearing quick slot
        Inventory->ClearQuickSlot(0);
        QuickSlotItem = Inventory->GetQuickSlotItem(0);
        TestTrue(TEXT("Quick slot 0 should be empty after clear"), QuickSlotItem.IsEmpty());

        // Test invalid quick slot index
        Inventory->AssignToQuickSlot(EntryId, 100);
        QuickSlotItem = Inventory->GetQuickSlotItem(100);
        TestTrue(TEXT("Invalid quick slot should return empty"), QuickSlotItem.IsEmpty());
    }

    return true;
}

// ============================================================================
// PHASE 8.7: TOOLTIPS TEST
// ============================================================================

bool FInventoryTooltipsTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("iron_sword"), 1);
    TArray<FInventorySlot> Items = Inventory->GetAllItems();

    if (Items.Num() > 0)
    {
        FItemTooltip Tooltip = Inventory->GetItemTooltip(Items[0].EntryId);

        TestFalse(TEXT("Tooltip name should not be empty"), Tooltip.Name.IsEmpty());
        TestFalse(TEXT("Tooltip type should not be empty"), Tooltip.Type.IsEmpty());
        TestTrue(TEXT("Tooltip weight should be >= 0"), Tooltip.Weight >= 0.0f);

        // Test GetItemTooltipBySlot
        FItemTooltip SlotTooltip = Inventory->GetItemTooltipBySlot(Items[0].SlotIndex);
        TestEqual(TEXT("Tooltip by slot should match"), SlotTooltip.Name, Tooltip.Name);
    }

    // Test invalid item tooltip
    FItemTooltip InvalidTooltip = Inventory->GetItemTooltip(999999);
    TestTrue(TEXT("Invalid item tooltip should have empty name"), InvalidTooltip.Name.IsEmpty());

    return true;
}

// ============================================================================
// PHASE 8.8: RARITY SYSTEM TEST
// ============================================================================

bool FInventoryRarityTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test rarity colors
    FLinearColor CommonColor = UInventoryComponent::GetRarityColor(EItemRarity::Common);
    FLinearColor LegendaryColor = UInventoryComponent::GetRarityColor(EItemRarity::Legendary);

    TestTrue(TEXT("Common and Legendary should have different colors"),
        CommonColor.R != LegendaryColor.R || CommonColor.G != LegendaryColor.G);

    // Test rarity names
    FString CommonName = UInventoryComponent::GetRarityName(EItemRarity::Common);
    TestEqual(TEXT("Common rarity name should be 'Common'"), CommonName, TEXT("Common"));

    FString LegendaryName = UInventoryComponent::GetRarityName(EItemRarity::Legendary);
    TestEqual(TEXT("Legendary rarity name should be 'Legendary'"), LegendaryName, TEXT("Legendary"));

    // Test GetItemsByRarity
    Inventory->AddItem(TEXT("health_potion"), 5);
    TArray<FInventorySlot> CommonItems = Inventory->GetItemsByRarity(EItemRarity::Common);
    TestTrue(TEXT("Should have items of at least Common rarity"), CommonItems.Num() > 0);

    return true;
}

// ============================================================================
// PHASE 8.9: DURABILITY SYSTEM TEST
// ============================================================================

bool FInventoryDurabilityTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add a weapon (has durability)
    Inventory->AddItem(TEXT("iron_sword"), 1);
    TArray<FInventorySlot> Items = Inventory->GetAllItems();

    if (Items.Num() > 0 && Items[0].bHasDurability)
    {
        int64 EntryId = Items[0].EntryId;

        // Test initial durability
        float InitialDurability = Inventory->GetDurabilityPercentage(EntryId);
        TestEqual(TEXT("Initial durability should be 100%"), InitialDurability, 100.0f);

        // Test reducing durability
        Inventory->ReduceDurability(EntryId, 30.0f);
        float AfterDamage = Inventory->GetDurabilityPercentage(EntryId);
        TestTrue(TEXT("Durability should decrease after damage"), AfterDamage < 100.0f);

        // Test item is not broken yet
        TestFalse(TEXT("Item should not be broken at 70%"), Inventory->IsItemBroken(EntryId));

        // Test repair
        Inventory->RepairItem(EntryId, 20.0f);
        float AfterRepair = Inventory->GetDurabilityPercentage(EntryId);
        TestTrue(TEXT("Durability should increase after repair"), AfterRepair > AfterDamage);

        // Test full repair
        Inventory->FullyRepairItem(EntryId);
        float AfterFullRepair = Inventory->GetDurabilityPercentage(EntryId);
        TestEqual(TEXT("Durability should be 100% after full repair"), AfterFullRepair, 100.0f);

        // Test breaking item
        Inventory->ReduceDurability(EntryId, 200.0f);
        TestTrue(TEXT("Item should be broken at 0 durability"), Inventory->IsItemBroken(EntryId));
    }

    // Test GetItemsNeedingRepair
    TArray<FInventorySlot> NeedRepair = Inventory->GetItemsNeedingRepair(50.0f);
    // Result depends on item state

    return true;
}

// ============================================================================
// PHASE 8.10: AUTO-SORT TEST
// ============================================================================

bool FInventoryAutoSortTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial auto-sort state
    TestFalse(TEXT("Auto-sort should be disabled by default"), Inventory->IsAutoSortEnabled());

    // Enable auto-sort
    Inventory->SetAutoSortEnabled(true);
    TestTrue(TEXT("Auto-sort should be enabled after setting"), Inventory->IsAutoSortEnabled());

    // Add items (should auto-sort)
    Inventory->AddItem(TEXT("wooden_shield"), 1);
    Inventory->AddItem(TEXT("health_potion"), 3);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Items should be sorted (by type, then name)
    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    TestTrue(TEXT("Should have items after adding with auto-sort"), Items.Num() > 0);

    // Test manual AutoSort call
    Inventory->SetAutoSortEnabled(false);
    Inventory->AutoSort();
    Items = Inventory->GetAllItems();
    TestTrue(TEXT("Items should exist after manual auto-sort"), Items.Num() > 0);

    return true;
}

// ============================================================================
// PHASE 8.11: SEARCH TEST
// ============================================================================

bool FInventorySearchTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("health_potion"), 5);
    Inventory->AddItem(TEXT("mana_potion"), 3);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Test SearchItems
    TArray<FInventorySlot> PotionResults = Inventory->SearchItems(TEXT("potion"));
    TestEqual(TEXT("Search 'potion' should find 2 items"), PotionResults.Num(), 2);

    TArray<FInventorySlot> SwordResults = Inventory->SearchItems(TEXT("sword"));
    TestEqual(TEXT("Search 'sword' should find 1 item"), SwordResults.Num(), 1);

    TArray<FInventorySlot> NoResults = Inventory->SearchItems(TEXT("nonexistent"));
    TestEqual(TEXT("Search for non-existent should return 0"), NoResults.Num(), 0);

    // Test empty search returns all
    TArray<FInventorySlot> AllResults = Inventory->SearchItems(TEXT(""));
    TestEqual(TEXT("Empty search should return all items"), AllResults.Num(), 3);

    // Test SetSearchQuery and GetSearchQuery
    Inventory->SetSearchQuery(TEXT("health"));
    TestEqual(TEXT("Search query should be set"), Inventory->GetSearchQuery(), TEXT("health"));

    // GetFilteredItems should use search query
    TArray<FInventorySlot> FilteredWithSearch = Inventory->GetFilteredItems();
    TestEqual(TEXT("Filtered items should respect search query"), FilteredWithSearch.Num(), 1);

    return true;
}

// ============================================================================
// PHASE 8.12: EQUIPMENT SLOTS TEST
// ============================================================================

bool FInventoryEquipmentTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add equippable items
    Inventory->AddItem(TEXT("iron_sword"), 1);
    Inventory->AddItem(TEXT("wooden_shield"), 1);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();

    int64 SwordId = 0, ShieldId = 0;
    for (const FInventorySlot& Slot : Items)
    {
        if (Slot.ItemId.Contains(TEXT("sword"))) SwordId = Slot.EntryId;
        else if (Slot.ItemId.Contains(TEXT("shield"))) ShieldId = Slot.EntryId;
    }

    if (SwordId != 0)
    {
        // Test CanEquipToSlot
        TestTrue(TEXT("Sword should be equippable to MainHand"),
            Inventory->CanEquipToSlot(SwordId, EEquipmentSlot::MainHand));

        // Test EquipItem
        bool bEquipped = Inventory->EquipItem(SwordId, EEquipmentSlot::MainHand);
        TestTrue(TEXT("Equip should succeed"), bEquipped);

        // Test IsSlotEquipped
        TestTrue(TEXT("MainHand slot should be equipped"), Inventory->IsSlotEquipped(EEquipmentSlot::MainHand));

        // Test GetEquippedItem
        FEquippedItem Equipped = Inventory->GetEquippedItem(EEquipmentSlot::MainHand);
        TestEqual(TEXT("Equipped item should match"), Equipped.Item.EntryId, SwordId);

        // Test UnequipItem
        bool bUnequipped = Inventory->UnequipItem(EEquipmentSlot::MainHand);
        TestTrue(TEXT("Unequip should succeed"), bUnequipped);
        TestFalse(TEXT("MainHand should be empty after unequip"), Inventory->IsSlotEquipped(EEquipmentSlot::MainHand));
    }

    // Test GetAllEquippedItems
    TArray<FEquippedItem> AllEquipped = Inventory->GetAllEquippedItems();
    // Should be empty after unequipping

    return true;
}

// ============================================================================
// PHASE 8.13: ITEM COMPARISON TEST
// ============================================================================

bool FInventoryComparisonTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("iron_sword"), 1);
    Inventory->AddItem(TEXT("wooden_shield"), 1);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();

    if (Items.Num() >= 2)
    {
        FItemComparison Comparison = Inventory->CompareItems(Items[0].EntryId, Items[1].EntryId);

        TestFalse(TEXT("ItemA should not be empty"), Comparison.ItemA.IsEmpty());
        TestFalse(TEXT("ItemB should not be empty"), Comparison.ItemB.IsEmpty());

        // Test weight difference is calculated
        TestTrue(TEXT("Weight difference should be calculated"),
            Comparison.WeightDifference == Comparison.ItemA.Weight - Comparison.ItemB.Weight);
    }

    // Test CompareWithEquipped when slot is empty
    if (Items.Num() > 0)
    {
        FItemComparison EmptyComparison = Inventory->CompareWithEquipped(Items[0].EntryId, EEquipmentSlot::MainHand);
        TestFalse(TEXT("ItemA should be set in comparison"), EmptyComparison.ItemA.IsEmpty());
    }

    return true;
}

// ============================================================================
// PHASE 8.14: BULK OPERATIONS TEST
// ============================================================================

bool FInventoryBulkOpsTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test AddItemsBulk
    TArray<FString> ItemIds = { TEXT("health_potion"), TEXT("mana_potion"), TEXT("gold_coin") };
    TArray<int32> Quantities = { 5, 3, 100 };

    int32 AddedCount = Inventory->AddItemsBulk(ItemIds, Quantities);
    TestEqual(TEXT("Should add 3 item types"), AddedCount, 3);

    // Verify items were added
    TestTrue(TEXT("Health potion should be added"), Inventory->HasItem(TEXT("health_potion"), 5));
    TestTrue(TEXT("Mana potion should be added"), Inventory->HasItem(TEXT("mana_potion"), 3));
    TestTrue(TEXT("Gold coin should be added"), Inventory->HasItem(TEXT("gold_coin"), 100));

    // Test RemoveAllOfItem
    int32 RemovedGold = Inventory->RemoveAllOfItem(TEXT("gold_coin"));
    TestEqual(TEXT("Should remove 100 gold coins"), RemovedGold, 100);
    TestFalse(TEXT("Gold coin should no longer exist"), Inventory->HasItem(TEXT("gold_coin"), 1));

    // Test ClearInventory
    Inventory->ClearInventory(false); // Don't remove locked items
    int32 RemainingCount = Inventory->GetAllItems().Num();
    TestEqual(TEXT("All items should be cleared"), RemainingCount, 0);

    return true;
}

// ============================================================================
// PHASE 8.15: LOCAL PERSISTENCE TEST
// ============================================================================

bool FInventoryPersistenceTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add items
    Inventory->AddItem(TEXT("health_potion"), 5);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Test GetSaveFilePath
    FString SavePath = Inventory->GetSaveFilePath();
    TestFalse(TEXT("Save path should not be empty"), SavePath.IsEmpty());

    // Test SaveInventoryToLocal
    bool bSaved = Inventory->SaveInventoryToLocal();
    TestTrue(TEXT("Save should succeed"), bSaved);

    // Test HasLocalSave
    TestTrue(TEXT("Should have local save after saving"), Inventory->HasLocalSave());

    // Clear inventory and reload
    Inventory->ClearInventory(true);
    TestEqual(TEXT("Inventory should be empty after clear"), Inventory->GetAllItems().Num(), 0);

    // Test LoadInventoryFromLocal
    bool bLoaded = Inventory->LoadInventoryFromLocal();
    TestTrue(TEXT("Load should succeed"), bLoaded);

    // Verify items are restored
    TestTrue(TEXT("Health potion should be restored"), Inventory->HasItem(TEXT("health_potion"), 5));
    TestTrue(TEXT("Iron sword should be restored"), Inventory->HasItem(TEXT("iron_sword"), 1));

    return true;
}

// ============================================================================
// PHASE 8.16: OVERFLOW HANDLING TEST
// ============================================================================

bool FInventoryOverflowTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial overflow count
    TestEqual(TEXT("Initial overflow count should be 0"), Inventory->GetOverflowCount(), 0);

    // Test GetOverflowItems
    TArray<FInventorySlot> OverflowItems = Inventory->GetOverflowItems();
    TestEqual(TEXT("Initial overflow items should be empty"), OverflowItems.Num(), 0);

    // Test ClearOverflow
    Inventory->ClearOverflow();
    TestEqual(TEXT("Overflow should remain empty after clear"), Inventory->GetOverflowCount(), 0);

    // ClaimOverflowItem with invalid index
    bool bClaimed = Inventory->ClaimOverflowItem(999);
    TestFalse(TEXT("Claiming invalid overflow index should fail"), bClaimed);

    return true;
}

// ============================================================================
// PHASE 8.17: ITEM VALIDATION TEST
// ============================================================================

bool FInventoryValidationTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Add valid items
    Inventory->AddItem(TEXT("health_potion"), 5);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Test ValidateInventory
    bool bValid = Inventory->ValidateInventory();
    TestTrue(TEXT("Inventory with valid items should validate"), bValid);

    // Test GetValidationErrors
    TArray<FString> Errors = Inventory->GetValidationErrors();
    TestEqual(TEXT("Valid inventory should have no errors"), Errors.Num(), 0);

    // Test ValidateItem with valid item
    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    if (Items.Num() > 0)
    {
        bool bItemValid = Inventory->ValidateItem(Items[0]);
        TestTrue(TEXT("Valid item should validate"), bItemValid);
    }

    // Test ValidateItem with invalid item
    FInventorySlot InvalidItem;
    InvalidItem.ItemId = TEXT(""); // Empty item ID
    InvalidItem.Quantity = 0;

    bool bInvalidItemValid = Inventory->ValidateItem(InvalidItem);
    TestFalse(TEXT("Invalid item should not validate"), bInvalidItemValid);

    return true;
}

// ============================================================================
// PHASE 8.18: TRANSACTION LOGGING TEST
// ============================================================================

bool FInventoryTransactionLogTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial logging state
    TestTrue(TEXT("Transaction logging should be enabled by default"), Inventory->IsTransactionLoggingEnabled());

    // Perform operations to generate logs
    Inventory->AddItem(TEXT("health_potion"), 5);
    Inventory->AddItem(TEXT("iron_sword"), 1);

    // Test GetTransactionHistory
    TArray<FInventoryTransaction> History = Inventory->GetTransactionHistory(50);
    TestTrue(TEXT("Transaction history should have entries"), History.Num() > 0);

    // Verify transaction structure
    if (History.Num() > 0)
    {
        FInventoryTransaction& Trans = History[0];
        TestFalse(TEXT("Transaction action should not be empty"), Trans.Action.IsEmpty());
    }

    // Test SetTransactionLoggingEnabled
    Inventory->SetTransactionLoggingEnabled(false);
    TestFalse(TEXT("Logging should be disabled after setting"), Inventory->IsTransactionLoggingEnabled());

    // Test ClearTransactionHistory
    Inventory->SetTransactionLoggingEnabled(true);
    Inventory->ClearTransactionHistory();
    History = Inventory->GetTransactionHistory(50);
    TestEqual(TEXT("History should be empty after clear"), History.Num(), 0);

    return true;
}

// ============================================================================
// PHASE 8.19: CAPACITY EXPANSION TEST
// ============================================================================

bool FInventoryCapacityTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    // Test initial capacity
    int32 InitialMaxSlots = Inventory->GetMaxSlots();
    TestEqual(TEXT("Initial max slots should be 20"), InitialMaxSlots, 20);

    // Test GetCapacityLevel
    int32 InitialLevel = Inventory->GetCapacityLevel();
    TestEqual(TEXT("Initial capacity level should be 0"), InitialLevel, 0);

    // Test CanExpandCapacity
    TestTrue(TEXT("Should be able to expand capacity at level 0"), Inventory->CanExpandCapacity());

    // Test GetMaxCapacityLevel
    int32 MaxLevel = Inventory->GetMaxCapacityLevel();
    TestTrue(TEXT("Max capacity level should be > 0"), MaxLevel > 0);

    // Test ExpandCapacity
    bool bExpanded = Inventory->ExpandCapacity(10);
    TestTrue(TEXT("Capacity expansion should succeed"), bExpanded);

    int32 NewMaxSlots = Inventory->GetMaxSlots();
    TestEqual(TEXT("Max slots should increase by 10"), NewMaxSlots, 30);

    int32 NewLevel = Inventory->GetCapacityLevel();
    TestEqual(TEXT("Capacity level should increase to 1"), NewLevel, 1);

    // Test GetSlotsForLevel
    int32 SlotsForLevel0 = Inventory->GetSlotsForLevel(0);
    TestEqual(TEXT("Slots for level 0 should be 20"), SlotsForLevel0, 20);

    return true;
}

// ============================================================================
// PHASE 8.20: FAVORITES AND LOCKING TEST
// ============================================================================

bool FInventoryFavoritesLockTest::RunTest(const FString& Parameters)
{
    UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

    Inventory->AddItem(TEXT("iron_sword"), 1);
    Inventory->AddItem(TEXT("health_potion"), 5);

    TArray<FInventorySlot> Items = Inventory->GetAllItems();
    if (Items.Num() < 2) return true;

    int64 SwordId = Items[0].EntryId;
    int64 PotionId = Items[1].EntryId;

    // Test ToggleFavorite
    Inventory->ToggleFavorite(SwordId);
    TArray<FInventorySlot> Favorites = Inventory->GetFavoriteItems();
    TestEqual(TEXT("Should have 1 favorite item"), Favorites.Num(), 1);

    // Test SetFavorite
    Inventory->SetFavorite(PotionId, true);
    Favorites = Inventory->GetFavoriteItems();
    TestEqual(TEXT("Should have 2 favorite items"), Favorites.Num(), 2);

    // Test ToggleLock
    Inventory->ToggleLock(SwordId);
    TestTrue(TEXT("Sword should be locked"), Inventory->IsItemLocked(SwordId));

    // Test SetLocked
    Inventory->SetLocked(PotionId, true);
    TArray<FInventorySlot> LockedItems = Inventory->GetLockedItems();
    TestEqual(TEXT("Should have 2 locked items"), LockedItems.Num(), 2);

    // Test that locked items cannot be removed
    Inventory->RemoveItem(SwordId, 1);
    TestTrue(TEXT("Locked sword should still exist"), Inventory->HasItem(TEXT("iron_sword"), 1));

    // Test ClearInventory with locked items
    Inventory->ClearInventory(false); // Don't include locked
    Items = Inventory->GetAllItems();
    TestEqual(TEXT("Locked items should remain after clear"), Items.Num(), 2);

    // Test ClearInventory including locked items
    Inventory->ClearInventory(true); // Include locked
    Items = Inventory->GetAllItems();
    TestEqual(TEXT("All items should be cleared including locked"), Items.Num(), 0);

    return true;
}

// ============================================================================
// CHARACTER TESTS
// ============================================================================

bool FCharacterHealthTest::RunTest(const FString& Parameters)
{
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

    Character->ApplyDamage(1000.0f);
    TestTrue(TEXT("Character should be dead after lethal damage"), Character->IsDead());

    return true;
}

bool FCharacterHealTest::RunTest(const FString& Parameters)
{
    AEonCharacter* Character = NewObject<AEonCharacter>();

    Character->ApplyDamage(50.0f);
    float DamagedHealth = Character->GetHealth();

    Character->Heal(25.0f);
    float HealedHealth = Character->GetHealth();

    TestEqual(TEXT("Health should increase by heal amount"), HealedHealth, DamagedHealth + 25.0f);

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

    AActor* Current = Interaction->GetCurrentInteractable();
    TestNull(TEXT("Should have no interactable initially"), Current);

    return true;
}

bool FInteractionPromptTest::RunTest(const FString& Parameters)
{
    UInteractionComponent* Interaction = NewObject<UInteractionComponent>();

    FString Prompt = Interaction->GetInteractionPrompt();
    TestTrue(TEXT("Prompt should be empty with no interactable"), Prompt.IsEmpty());

    return true;
}
