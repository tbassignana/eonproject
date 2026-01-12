// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

// ============================================================================
// PHASE 8: ENUMS
// ============================================================================

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	All UMETA(DisplayName = "All"),
	Consumable UMETA(DisplayName = "Consumable"),
	Weapon UMETA(DisplayName = "Weapon"),
	Armor UMETA(DisplayName = "Armor"),
	Accessory UMETA(DisplayName = "Accessory"),
	Resource UMETA(DisplayName = "Resource"),
	Quest UMETA(DisplayName = "Quest"),
	Misc UMETA(DisplayName = "Misc")
};

UENUM(BlueprintType)
enum class EInventorySortMode : uint8
{
	None UMETA(DisplayName = "None"),
	ByName UMETA(DisplayName = "By Name"),
	ByType UMETA(DisplayName = "By Type"),
	ByQuantity UMETA(DisplayName = "By Quantity"),
	ByRarity UMETA(DisplayName = "By Rarity"),
	ByWeight UMETA(DisplayName = "By Weight")
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None UMETA(DisplayName = "None"),
	MainHand UMETA(DisplayName = "Main Hand"),
	OffHand UMETA(DisplayName = "Off Hand"),
	Head UMETA(DisplayName = "Head"),
	Chest UMETA(DisplayName = "Chest"),
	Legs UMETA(DisplayName = "Legs"),
	Feet UMETA(DisplayName = "Feet"),
	Accessory1 UMETA(DisplayName = "Accessory 1"),
	Accessory2 UMETA(DisplayName = "Accessory 2")
};

// ============================================================================
// PHASE 8: STRUCTS
// ============================================================================

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

	// Phase 8.1: Weight system
	UPROPERTY(BlueprintReadOnly)
	float Weight = 0.0f;

	// Phase 8.8: Rarity system
	UPROPERTY(BlueprintReadOnly)
	EItemRarity Rarity = EItemRarity::Common;

	// Phase 8.9: Durability system
	UPROPERTY(BlueprintReadOnly)
	float MaxDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasDurability = false;

	// Phase 8.12: Equipment slot compatibility
	UPROPERTY(BlueprintReadOnly)
	EEquipmentSlot EquipSlot = EEquipmentSlot::None;

	// Phase 8.20: Favorites and locking
	UPROPERTY(BlueprintReadOnly)
	bool bIsFavorite = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocked = false;

	// Phase 8.7: Tooltip info
	UPROPERTY(BlueprintReadOnly)
	FString Description;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, float> Stats;

	bool IsEmpty() const { return ItemId.IsEmpty(); }
	float GetTotalWeight() const { return Weight * Quantity; }
};

USTRUCT(BlueprintType)
struct FEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EEquipmentSlot Slot = EEquipmentSlot::None;

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot Item;

	bool IsEmpty() const { return Item.IsEmpty(); }
};

USTRUCT(BlueprintType)
struct FItemComparison
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot ItemA;

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot ItemB;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, float> StatDifferences;

	UPROPERTY(BlueprintReadOnly)
	float WeightDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 RarityDifference = 0;
};

USTRUCT(BlueprintType)
struct FInventoryTransaction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	FString Action;

	UPROPERTY(BlueprintReadOnly)
	FString ItemId;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FString Details;
};

USTRUCT(BlueprintType)
struct FItemTooltip
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	EItemRarity Rarity = EItemRarity::Common;

	UPROPERTY(BlueprintReadOnly)
	FString Type;

	UPROPERTY(BlueprintReadOnly)
	FString Description;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> StatLines;

	UPROPERTY(BlueprintReadOnly)
	float Weight = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FString DurabilityText;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFavorite = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocked = false;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryOverflow, const FString&, ItemId, int32, OverflowAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, EEquipmentSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionLogged, const FInventoryTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCapacityChanged, int32, NewCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDurabilityChanged, int64, EntryId, float, NewDurability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotChanged, int32, SlotIndex, int64, EntryId);

// ============================================================================
// INVENTORY COMPONENT
// ============================================================================

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================================================
	// ORIGINAL FUNCTIONS
	// ========================================================================

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

	UFUNCTION()
	void OnInventoryDataReceived(const FString& JsonData);

	// ========================================================================
	// PHASE 8.1: WEIGHT SYSTEM
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
	float GetMaxCarryCapacity() const { return MaxCarryCapacity; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
	float GetWeightPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
	bool CanCarryWeight(float AdditionalWeight) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
	bool IsOverEncumbered() const;

	// ========================================================================
	// PHASE 8.2: CATEGORY FILTERING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Filter")
	TArray<FInventorySlot> GetItemsByCategory(EItemCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Filter")
	void SetActiveFilter(EItemCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Filter")
	EItemCategory GetActiveFilter() const { return ActiveFilter; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Filter")
	TArray<FInventorySlot> GetFilteredItems() const;

	// ========================================================================
	// PHASE 8.3: SORTING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	void SortInventory(EInventorySortMode SortMode, bool bAscending = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	EInventorySortMode GetCurrentSortMode() const { return CurrentSortMode; }

	// ========================================================================
	// PHASE 8.4: STACK SPLITTING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Stacks")
	bool SplitStack(int64 EntryId, int32 SplitAmount);

	// ========================================================================
	// PHASE 8.5: STACK COMBINING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Stacks")
	bool CombineStacks(int64 SourceEntryId, int64 TargetEntryId);

	// ========================================================================
	// PHASE 8.6: QUICK SLOTS / HOTBAR
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	void AssignToQuickSlot(int64 EntryId, int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	void ClearQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	void UseQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	FInventorySlot GetQuickSlotItem(int32 QuickSlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	TArray<int64> GetQuickSlots() const { return QuickSlots; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlots")
	int32 GetNumQuickSlots() const { return NumQuickSlots; }

	// ========================================================================
	// PHASE 8.7: TOOLTIPS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltips")
	FItemTooltip GetItemTooltip(int64 EntryId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltips")
	FItemTooltip GetItemTooltipBySlot(int32 SlotIndex) const;

	// ========================================================================
	// PHASE 8.8: RARITY SYSTEM
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Rarity")
	TArray<FInventorySlot> GetItemsByRarity(EItemRarity MinRarity) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Rarity")
	static FLinearColor GetRarityColor(EItemRarity Rarity);

	UFUNCTION(BlueprintPure, Category = "Inventory|Rarity")
	static FString GetRarityName(EItemRarity Rarity);

	// ========================================================================
	// PHASE 8.9: DURABILITY SYSTEM
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	void ReduceDurability(int64 EntryId, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	void RepairItem(int64 EntryId, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	void FullyRepairItem(int64 EntryId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	float GetDurabilityPercentage(int64 EntryId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	bool IsItemBroken(int64 EntryId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	TArray<FInventorySlot> GetItemsNeedingRepair(float DurabilityThreshold = 25.0f) const;

	// ========================================================================
	// PHASE 8.10: AUTO-SORT
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	void AutoSort();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	void SetAutoSortEnabled(bool bEnabled) { bAutoSortEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sort")
	bool IsAutoSortEnabled() const { return bAutoSortEnabled; }

	// ========================================================================
	// PHASE 8.11: SEARCH/FILTER BY NAME
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Search")
	TArray<FInventorySlot> SearchItems(const FString& SearchQuery) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Search")
	void SetSearchQuery(const FString& Query);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Search")
	FString GetSearchQuery() const { return CurrentSearchQuery; }

	// ========================================================================
	// PHASE 8.12: EQUIPMENT SLOTS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int64 EntryId, EEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	FEquippedItem GetEquippedItem(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	TArray<FEquippedItem> GetAllEquippedItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotEquipped(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool CanEquipToSlot(int64 EntryId, EEquipmentSlot Slot) const;

	// ========================================================================
	// PHASE 8.13: ITEM COMPARISON
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Compare")
	FItemComparison CompareItems(int64 EntryIdA, int64 EntryIdB) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Compare")
	FItemComparison CompareWithEquipped(int64 EntryId, EEquipmentSlot Slot) const;

	// ========================================================================
	// PHASE 8.14: BULK OPERATIONS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bulk")
	int32 AddItemsBulk(const TArray<FString>& ItemIds, const TArray<int32>& Quantities);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bulk")
	int32 RemoveItemsBulk(const TArray<int64>& EntryIds, const TArray<int32>& Quantities);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bulk")
	int32 RemoveAllOfItem(const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bulk")
	void ClearInventory(bool bIncludeLocked = false);

	// ========================================================================
	// PHASE 8.15: LOCAL PERSISTENCE
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
	bool SaveInventoryToLocal();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
	bool LoadInventoryFromLocal();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
	FString GetSaveFilePath() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
	bool HasLocalSave() const;

	// ========================================================================
	// PHASE 8.16: OVERFLOW HANDLING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Overflow")
	int32 GetOverflowCount() const { return OverflowItems.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Overflow")
	TArray<FInventorySlot> GetOverflowItems() const { return OverflowItems; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Overflow")
	bool ClaimOverflowItem(int32 OverflowIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Overflow")
	void ClearOverflow();

	// ========================================================================
	// PHASE 8.17: ITEM VALIDATION
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Validation")
	bool ValidateItem(const FInventorySlot& Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Validation")
	bool ValidateInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Validation")
	TArray<FString> GetValidationErrors() const;

	// ========================================================================
	// PHASE 8.18: TRANSACTION LOGGING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Logging")
	TArray<FInventoryTransaction> GetTransactionHistory(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Logging")
	void ClearTransactionHistory();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Logging")
	void SetTransactionLoggingEnabled(bool bEnabled) { bTransactionLoggingEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Logging")
	bool IsTransactionLoggingEnabled() const { return bTransactionLoggingEnabled; }

	// ========================================================================
	// PHASE 8.19: CAPACITY EXPANSION
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Capacity")
	bool ExpandCapacity(int32 AdditionalSlots);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Capacity")
	int32 GetCapacityLevel() const { return CapacityLevel; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Capacity")
	int32 GetMaxCapacityLevel() const { return MaxCapacityLevel; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Capacity")
	int32 GetSlotsForLevel(int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Capacity")
	bool CanExpandCapacity() const;

	// ========================================================================
	// PHASE 8.20: FAVORITES & LOCKING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Inventory|Favorites")
	void ToggleFavorite(int64 EntryId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Favorites")
	void SetFavorite(int64 EntryId, bool bFavorite);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Favorites")
	TArray<FInventorySlot> GetFavoriteItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lock")
	void ToggleLock(int64 EntryId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lock")
	void SetLocked(int64 EntryId, bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lock")
	TArray<FInventorySlot> GetLockedItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Lock")
	bool IsItemLocked(int64 EntryId) const;

	// ========================================================================
	// DELEGATES
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryOverflow OnInventoryOverflow;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnTransactionLogged OnTransactionLogged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnCapacityChanged OnCapacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemDurabilityChanged OnItemDurabilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnQuickSlotChanged OnQuickSlotChanged;

protected:
	// ========================================================================
	// CONFIGURABLE PROPERTIES
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 MaxSlots = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Weight")
	float MaxCarryCapacity = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|QuickSlots")
	int32 NumQuickSlots = 6;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Capacity")
	int32 SlotsPerCapacityLevel = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Capacity")
	int32 MaxCapacityLevel = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Logging")
	int32 MaxTransactionLogSize = 100;

private:
	// ========================================================================
	// INTERNAL STATE
	// ========================================================================

	UPROPERTY()
	TArray<FInventorySlot> Items;

	UPROPERTY()
	TArray<FInventorySlot> OverflowItems;

	UPROPERTY()
	TMap<EEquipmentSlot, FInventorySlot> EquippedItems;

	UPROPERTY()
	TArray<int64> QuickSlots;

	UPROPERTY()
	TArray<FInventoryTransaction> TransactionLog;

	int64 NextLocalEntryId = 1;
	EItemCategory ActiveFilter = EItemCategory::All;
	EInventorySortMode CurrentSortMode = EInventorySortMode::None;
	FString CurrentSearchQuery;
	bool bAutoSortEnabled = false;
	bool bTransactionLoggingEnabled = true;
	int32 CapacityLevel = 0;

	// ========================================================================
	// INTERNAL HELPERS
	// ========================================================================

	void RefreshFromServer();
	void AddItemLocal(const FString& ItemId, int32 Quantity);
	void RemoveItemLocal(int64 EntryId, int32 Quantity);
	void LogTransaction(const FString& Action, const FString& ItemId, int32 Quantity, bool bSuccess, const FString& Details = TEXT(""));
	FInventorySlot* FindItemByEntryId(int64 EntryId);
	const FInventorySlot* FindItemByEntryIdConst(int64 EntryId) const;
	int32 FindFirstEmptySlotIndex() const;
	void ReassignSlotIndices();
	FInventorySlot CreateItemSlot(const FString& ItemId, int32 Quantity);
};
