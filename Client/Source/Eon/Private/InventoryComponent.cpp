// Copyright 2026 Eon Project. All rights reserved.

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize empty inventory
    Items.Empty();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FInventoryItem UInventoryComponent::GetItemById(int64 ItemId) const
{
    int32 Index = FindItemIndex(ItemId);
    if (Index != INDEX_NONE)
    {
        return Items[Index];
    }
    return FInventoryItem();
}

FInventoryItem UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
    for (const FInventoryItem& Item : Items)
    {
        if (Item.SlotIndex == SlotIndex)
        {
            return Item;
        }
    }
    return FInventoryItem();
}

bool UInventoryComponent::HasItem(int64 ItemDefId) const
{
    return FindItemIndexByDefId(ItemDefId) != INDEX_NONE;
}

int32 UInventoryComponent::GetItemQuantity(int64 ItemDefId) const
{
    int32 Index = FindItemIndexByDefId(ItemDefId);
    if (Index != INDEX_NONE)
    {
        return Items[Index].Quantity;
    }
    return 0;
}

void UInventoryComponent::AddItem(int64 ItemDefId, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return;
    }

    // Check if we already have an item with this definition
    int32 Index = FindItemIndexByDefId(ItemDefId);
    if (Index != INDEX_NONE)
    {
        // Add to existing stack
        Items[Index].Quantity += Quantity;
        OnInventoryUpdated.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("Added %d to existing item (DefId: %lld), new total: %d"),
               Quantity, ItemDefId, Items[Index].Quantity);
    }
    else
    {
        // Create new item
        FInventoryItem NewItem;
        NewItem.ItemId = FMath::Rand() * 1000000 + Items.Num(); // Temporary local ID
        NewItem.ItemDefId = ItemDefId;
        NewItem.Quantity = Quantity;
        NewItem.SlotIndex = Items.Num();
        NewItem.ItemName = FString::Printf(TEXT("Item_%lld"), ItemDefId); // Placeholder name

        Items.Add(NewItem);
        OnItemAdded.Broadcast(NewItem);
        OnInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("Added new item (DefId: %lld) x%d"), ItemDefId, Quantity);
    }
}

void UInventoryComponent::RemoveItem(int64 ItemDefId, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return;
    }

    int32 Index = FindItemIndexByDefId(ItemDefId);
    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot remove item - not found (DefId: %lld)"), ItemDefId);
        return;
    }

    FInventoryItem& Item = Items[Index];
    if (Quantity >= Item.Quantity)
    {
        // Remove entire stack
        int64 ItemId = Item.ItemId;
        Items.RemoveAt(Index);
        OnItemRemoved.Broadcast(ItemId);
        UE_LOG(LogTemp, Log, TEXT("Removed entire item stack (DefId: %lld)"), ItemDefId);
    }
    else
    {
        // Reduce quantity
        Item.Quantity -= Quantity;
        UE_LOG(LogTemp, Log, TEXT("Removed %d from item (DefId: %lld), remaining: %d"),
               Quantity, ItemDefId, Item.Quantity);
    }

    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::UseItem(int64 ItemId)
{
    int32 Index = FindItemIndex(ItemId);
    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot use item - item not found: %lld"), ItemId);
        return;
    }

    const FInventoryItem& Item = Items[Index];
    if (!Item.bIsConsumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot use item - not consumable: %s"), *Item.ItemName);
        return;
    }

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("use_item", ItemId);

    UE_LOG(LogTemp, Log, TEXT("Using item: %s (ID: %lld)"), *Item.ItemName, ItemId);

    // Optimistic update - will be corrected by server response
    OnItemUsed.Broadcast(Item);
}

void UInventoryComponent::DropItem(int64 ItemId, int32 Quantity)
{
    int32 Index = FindItemIndex(ItemId);
    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot drop item - item not found: %lld"), ItemId);
        return;
    }

    const FInventoryItem& Item = Items[Index];
    if (Quantity > Item.Quantity)
    {
        Quantity = Item.Quantity;
    }

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("remove_inventory_item", ItemId, Quantity);

    UE_LOG(LogTemp, Log, TEXT("Dropping %d x %s (ID: %lld)"), Quantity, *Item.ItemName, ItemId);
}

void UInventoryComponent::HandleInventoryUpdate(const TArray<FInventoryItem>& NewItems)
{
    Items = NewItems;
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Inventory updated: %d items"), Items.Num());
}

void UInventoryComponent::HandleItemAdded(const FInventoryItem& Item)
{
    // Check if we already have this item (update quantity)
    int32 ExistingIndex = FindItemIndex(Item.ItemId);
    if (ExistingIndex != INDEX_NONE)
    {
        Items[ExistingIndex] = Item;
    }
    else
    {
        Items.Add(Item);
    }

    OnItemAdded.Broadcast(Item);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Item added: %s x%d"), *Item.ItemName, Item.Quantity);
}

void UInventoryComponent::HandleItemRemoved(int64 ItemId)
{
    int32 Index = FindItemIndex(ItemId);
    if (Index != INDEX_NONE)
    {
        Items.RemoveAt(Index);
        OnItemRemoved.Broadcast(ItemId);
        OnInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("Item removed: ID %lld"), ItemId);
    }
}

void UInventoryComponent::HandleItemQuantityChanged(int64 ItemId, int32 NewQuantity)
{
    int32 Index = FindItemIndex(ItemId);
    if (Index != INDEX_NONE)
    {
        if (NewQuantity <= 0)
        {
            HandleItemRemoved(ItemId);
        }
        else
        {
            Items[Index].Quantity = NewQuantity;
            OnInventoryUpdated.Broadcast();

            UE_LOG(LogTemp, Log, TEXT("Item quantity changed: ID %lld, new qty: %d"), ItemId, NewQuantity);
        }
    }
}

int32 UInventoryComponent::FindItemIndex(int64 ItemId) const
{
    for (int32 i = 0; i < Items.Num(); ++i)
    {
        if (Items[i].ItemId == ItemId)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindItemIndexByDefId(int64 ItemDefId) const
{
    for (int32 i = 0; i < Items.Num(); ++i)
    {
        if (Items[i].ItemDefId == ItemDefId)
        {
            return i;
        }
    }
    return INDEX_NONE;
}
