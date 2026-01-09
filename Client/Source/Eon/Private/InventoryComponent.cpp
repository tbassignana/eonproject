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
