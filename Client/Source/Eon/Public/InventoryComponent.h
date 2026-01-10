// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 EntryId = 0;

	UPROPERTY(BlueprintReadOnly)
	FString ItemId;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxStack = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	FString ItemType;

	bool IsEmpty() const { return ItemId.IsEmpty(); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(const FString& ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(int64 EntryId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int64 EntryId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MoveItem(int64 EntryId, int32 NewSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventorySlot> GetAllItems() const { return Items; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetItemAtSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(const FString& ItemId, int32 MinQuantity = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(const FString& ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxSlots() const { return MaxSlots; }

	// Called when server updates inventory data
	UFUNCTION()
	void OnInventoryDataReceived(const FString& JsonData);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxSlots = 20;

private:
	UPROPERTY()
	TArray<FInventorySlot> Items;

	int64 NextLocalEntryId = 1; // For local-only mode

	void RefreshFromServer();
	void AddItemLocal(const FString& ItemId, int32 Quantity);
	void RemoveItemLocal(int64 EntryId, int32 Quantity);
};
