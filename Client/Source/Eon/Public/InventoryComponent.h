// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

/**
 * FInventoryItem - Represents a single item in inventory
 */
USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int64 ItemId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int64 ItemDefId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 SlotIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FString ItemName;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FString ItemDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    bool bIsConsumable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    float Damage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    float HealAmount = 0.0f;

    bool IsValid() const { return ItemId > 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FInventoryItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, int64, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, const FInventoryItem&, Item);

/**
 * UInventoryComponent - Manages player inventory state
 *
 * This component:
 * - Maintains local cache of inventory from SpaceTimeDB
 * - Handles inventory UI data binding
 * - Provides functions to use/discard items
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemRemoved OnItemRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemUsed OnItemUsed;

    // Inventory access
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetAllItems() const { return Items; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount() const { return Items.Num(); }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetItemById(int64 ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetItemAtSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(int64 ItemDefId) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemQuantity(int64 ItemDefId) const;

    // Local inventory management (for single-player/testing)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(int64 ItemDefId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int64 ItemDefId, int32 Quantity = 1);

    // Inventory actions (these call SpaceTimeDB reducers)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseItem(int64 ItemId);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropItem(int64 ItemId, int32 Quantity = 1);

    // Called by network manager when inventory updates arrive
    void HandleInventoryUpdate(const TArray<FInventoryItem>& NewItems);
    void HandleItemAdded(const FInventoryItem& Item);
    void HandleItemRemoved(int64 ItemId);
    void HandleItemQuantityChanged(int64 ItemId, int32 NewQuantity);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<FInventoryItem> Items;

    // Find item index by ID
    int32 FindItemIndex(int64 ItemId) const;
    int32 FindItemIndexByDefId(int64 ItemDefId) const;
};
