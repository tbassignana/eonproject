// Copyright 2026 tbassignana. MIT License.

#include "InventoryComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize quick slots
	QuickSlots.SetNum(NumQuickSlots);
	for (int32 i = 0; i < NumQuickSlots; ++i)
	{
		QuickSlots[i] = 0; // 0 = empty
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize quick slots array if not done in constructor
	if (QuickSlots.Num() != NumQuickSlots)
	{
		QuickSlots.SetNum(NumQuickSlots);
		for (int32 i = 0; i < NumQuickSlots; ++i)
		{
			QuickSlots[i] = 0;
		}
	}

	// Subscribe to inventory updates from SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->OnInventoryUpdated.AddDynamic(this, &UInventoryComponent::OnInventoryDataReceived);
		}
	}

	// Try to load local save if exists
	LoadInventoryFromLocal();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ============================================================================
// ORIGINAL FUNCTIONS
// ============================================================================

void UInventoryComponent::AddItem(const FString& ItemId, int32 Quantity)
{
	// Try SpaceTimeDB first
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			if (Manager->IsConnected())
			{
				Manager->AddItemToInventory(ItemId, Quantity);
				LogTransaction(TEXT("Add"), ItemId, Quantity, true, TEXT("Server request sent"));
				return;
			}
		}
	}

	// Local-only mode: add directly to inventory
	AddItemLocal(ItemId, Quantity);
}

void UInventoryComponent::RemoveItem(int64 EntryId, int32 Quantity)
{
	// Check if item is locked
	if (IsItemLocked(EntryId))
	{
		const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
		FString ItemId = Slot ? Slot->ItemId : TEXT("Unknown");
		LogTransaction(TEXT("Remove"), ItemId, Quantity, false, TEXT("Item is locked"));
		return;
	}

	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->RemoveItemFromInventory(EntryId, Quantity);
			const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
			LogTransaction(TEXT("Remove"), Slot ? Slot->ItemId : TEXT("Unknown"), Quantity, true);
			return;
		}
	}

	// Local fallback
	RemoveItemLocal(EntryId, Quantity);
}

void UInventoryComponent::UseItem(int64 EntryId)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (!Slot) return;

	// Check if item is broken
	if (Slot->bHasDurability && Slot->CurrentDurability <= 0.0f)
	{
		LogTransaction(TEXT("Use"), Slot->ItemId, 1, false, TEXT("Item is broken"));
		return;
	}

	if (Slot->ItemType == TEXT("consumable"))
	{
		if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
			{
				Manager->UseConsumable(EntryId);
				LogTransaction(TEXT("Use"), Slot->ItemId, 1, true);
			}
		}
	}
}

void UInventoryComponent::MoveItem(int64 EntryId, int32 NewSlotIndex)
{
	if (NewSlotIndex < 0 || NewSlotIndex >= MaxSlots) return;

	for (FInventorySlot& Slot : Items)
	{
		if (Slot.EntryId == EntryId)
		{
			for (FInventorySlot& OtherSlot : Items)
			{
				if (OtherSlot.SlotIndex == NewSlotIndex && OtherSlot.EntryId != EntryId)
				{
					OtherSlot.SlotIndex = Slot.SlotIndex;
					break;
				}
			}
			Slot.SlotIndex = NewSlotIndex;
			break;
		}
	}

	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Move"), TEXT(""), 0, true, FString::Printf(TEXT("Moved to slot %d"), NewSlotIndex));
}

FInventorySlot UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	const FInventorySlot* Found = Items.FindByPredicate([SlotIndex](const FInventorySlot& S) {
		return S.SlotIndex == SlotIndex;
	});

	return Found ? *Found : FInventorySlot();
}

bool UInventoryComponent::HasItem(const FString& ItemId, int32 MinQuantity) const
{
	return GetItemCount(ItemId) >= MinQuantity;
}

int32 UInventoryComponent::GetItemCount(const FString& ItemId) const
{
	int32 Total = 0;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.ItemId == ItemId)
		{
			Total += Slot.Quantity;
		}
	}
	return Total;
}

void UInventoryComponent::OnInventoryDataReceived(const FString& JsonData)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	FInventorySlot NewSlot;
	JsonObject->TryGetNumberField(TEXT("entry_id"), NewSlot.EntryId);
	JsonObject->TryGetStringField(TEXT("item_id"), NewSlot.ItemId);
	JsonObject->TryGetNumberField(TEXT("quantity"), NewSlot.Quantity);
	JsonObject->TryGetNumberField(TEXT("slot_index"), NewSlot.SlotIndex);
	JsonObject->TryGetStringField(TEXT("item_type"), NewSlot.ItemType);
	JsonObject->TryGetStringField(TEXT("display_name"), NewSlot.DisplayName);
	JsonObject->TryGetStringField(TEXT("description"), NewSlot.Description);

	// Phase 8 fields
	double Weight = 0.0;
	if (JsonObject->TryGetNumberField(TEXT("weight"), Weight))
	{
		NewSlot.Weight = static_cast<float>(Weight);
	}

	int32 RarityInt = 0;
	if (JsonObject->TryGetNumberField(TEXT("rarity"), RarityInt))
	{
		NewSlot.Rarity = static_cast<EItemRarity>(FMath::Clamp(RarityInt, 0, 4));
	}

	double Durability = 100.0;
	if (JsonObject->TryGetNumberField(TEXT("durability"), Durability))
	{
		NewSlot.CurrentDurability = static_cast<float>(Durability);
		NewSlot.bHasDurability = true;
	}

	bool bFound = false;
	for (FInventorySlot& Slot : Items)
	{
		if (Slot.EntryId == NewSlot.EntryId)
		{
			Slot = NewSlot;
			bFound = true;
			break;
		}
	}

	if (!bFound && !NewSlot.ItemId.IsEmpty())
	{
		Items.Add(NewSlot);
	}

	Items.RemoveAll([](const FInventorySlot& S) { return S.Quantity <= 0; });

	if (bAutoSortEnabled)
	{
		AutoSort();
	}

	OnInventoryChanged.Broadcast();
}

// ============================================================================
// PHASE 8.1: WEIGHT SYSTEM
// ============================================================================

float UInventoryComponent::GetCurrentWeight() const
{
	float TotalWeight = 0.0f;
	for (const FInventorySlot& Slot : Items)
	{
		TotalWeight += Slot.GetTotalWeight();
	}
	return TotalWeight;
}

float UInventoryComponent::GetWeightPercentage() const
{
	if (MaxCarryCapacity <= 0.0f) return 0.0f;
	return (GetCurrentWeight() / MaxCarryCapacity) * 100.0f;
}

bool UInventoryComponent::CanCarryWeight(float AdditionalWeight) const
{
	return (GetCurrentWeight() + AdditionalWeight) <= MaxCarryCapacity;
}

bool UInventoryComponent::IsOverEncumbered() const
{
	return GetCurrentWeight() > MaxCarryCapacity;
}

// ============================================================================
// PHASE 8.2: CATEGORY FILTERING
// ============================================================================

TArray<FInventorySlot> UInventoryComponent::GetItemsByCategory(EItemCategory Category) const
{
	if (Category == EItemCategory::All)
	{
		return Items;
	}

	TArray<FInventorySlot> FilteredItems;
	FString CategoryStr;

	switch (Category)
	{
		case EItemCategory::Consumable: CategoryStr = TEXT("consumable"); break;
		case EItemCategory::Weapon: CategoryStr = TEXT("weapon"); break;
		case EItemCategory::Armor: CategoryStr = TEXT("armor"); break;
		case EItemCategory::Accessory: CategoryStr = TEXT("accessory"); break;
		case EItemCategory::Resource: CategoryStr = TEXT("resource"); break;
		case EItemCategory::Quest: CategoryStr = TEXT("quest"); break;
		case EItemCategory::Misc: CategoryStr = TEXT("misc"); break;
		default: return Items;
	}

	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.ItemType.ToLower() == CategoryStr)
		{
			FilteredItems.Add(Slot);
		}
	}

	return FilteredItems;
}

void UInventoryComponent::SetActiveFilter(EItemCategory Category)
{
	ActiveFilter = Category;
	OnInventoryChanged.Broadcast();
}

TArray<FInventorySlot> UInventoryComponent::GetFilteredItems() const
{
	TArray<FInventorySlot> Result = GetItemsByCategory(ActiveFilter);

	// Also apply search query if set
	if (!CurrentSearchQuery.IsEmpty())
	{
		Result = Result.FilterByPredicate([this](const FInventorySlot& Slot) {
			return Slot.DisplayName.Contains(CurrentSearchQuery, ESearchCase::IgnoreCase) ||
			       Slot.ItemId.Contains(CurrentSearchQuery, ESearchCase::IgnoreCase);
		});
	}

	return Result;
}

// ============================================================================
// PHASE 8.3: SORTING
// ============================================================================

void UInventoryComponent::SortInventory(EInventorySortMode SortMode, bool bAscending)
{
	CurrentSortMode = SortMode;

	if (SortMode == EInventorySortMode::None)
	{
		return;
	}

	Items.Sort([SortMode, bAscending](const FInventorySlot& A, const FInventorySlot& B) {
		int32 Comparison = 0;

		switch (SortMode)
		{
			case EInventorySortMode::ByName:
				Comparison = A.DisplayName.Compare(B.DisplayName);
				break;
			case EInventorySortMode::ByType:
				Comparison = A.ItemType.Compare(B.ItemType);
				break;
			case EInventorySortMode::ByQuantity:
				Comparison = A.Quantity - B.Quantity;
				break;
			case EInventorySortMode::ByRarity:
				Comparison = static_cast<int32>(A.Rarity) - static_cast<int32>(B.Rarity);
				break;
			case EInventorySortMode::ByWeight:
				Comparison = (A.GetTotalWeight() < B.GetTotalWeight()) ? -1 : (A.GetTotalWeight() > B.GetTotalWeight()) ? 1 : 0;
				break;
			default:
				break;
		}

		return bAscending ? (Comparison < 0) : (Comparison > 0);
	});

	ReassignSlotIndices();
	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Sort"), TEXT(""), 0, true, FString::Printf(TEXT("Sorted by mode %d"), static_cast<int32>(SortMode)));
}

// ============================================================================
// PHASE 8.4: STACK SPLITTING
// ============================================================================

bool UInventoryComponent::SplitStack(int64 EntryId, int32 SplitAmount)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (!Slot || SplitAmount <= 0 || SplitAmount >= Slot->Quantity)
	{
		LogTransaction(TEXT("Split"), TEXT(""), SplitAmount, false, TEXT("Invalid split parameters"));
		return false;
	}

	if (Items.Num() >= MaxSlots)
	{
		LogTransaction(TEXT("Split"), Slot->ItemId, SplitAmount, false, TEXT("No empty slots available"));
		return false;
	}

	// Reduce original stack
	Slot->Quantity -= SplitAmount;

	// Create new stack
	FInventorySlot NewSlot = *Slot;
	NewSlot.EntryId = NextLocalEntryId++;
	NewSlot.Quantity = SplitAmount;
	NewSlot.SlotIndex = FindFirstEmptySlotIndex();

	Items.Add(NewSlot);
	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Split"), Slot->ItemId, SplitAmount, true);
	return true;
}

// ============================================================================
// PHASE 8.5: STACK COMBINING
// ============================================================================

bool UInventoryComponent::CombineStacks(int64 SourceEntryId, int64 TargetEntryId)
{
	if (SourceEntryId == TargetEntryId) return false;

	FInventorySlot* Source = FindItemByEntryId(SourceEntryId);
	FInventorySlot* Target = FindItemByEntryId(TargetEntryId);

	if (!Source || !Target || Source->ItemId != Target->ItemId)
	{
		LogTransaction(TEXT("Combine"), TEXT(""), 0, false, TEXT("Items cannot be combined"));
		return false;
	}

	int32 SpaceAvailable = Target->MaxStack - Target->Quantity;
	int32 ToTransfer = FMath::Min(Source->Quantity, SpaceAvailable);

	if (ToTransfer <= 0)
	{
		LogTransaction(TEXT("Combine"), Source->ItemId, 0, false, TEXT("Target stack is full"));
		return false;
	}

	Target->Quantity += ToTransfer;
	Source->Quantity -= ToTransfer;

	// Remove source if empty
	if (Source->Quantity <= 0)
	{
		Items.RemoveAll([SourceEntryId](const FInventorySlot& S) { return S.EntryId == SourceEntryId; });
	}

	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Combine"), Source->ItemId, ToTransfer, true);
	return true;
}

// ============================================================================
// PHASE 8.6: QUICK SLOTS / HOTBAR
// ============================================================================

void UInventoryComponent::AssignToQuickSlot(int64 EntryId, int32 QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= NumQuickSlots) return;

	// Verify item exists
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	if (!Slot) return;

	// Clear any existing assignment for this item
	for (int32 i = 0; i < QuickSlots.Num(); ++i)
	{
		if (QuickSlots[i] == EntryId)
		{
			QuickSlots[i] = 0;
		}
	}

	QuickSlots[QuickSlotIndex] = EntryId;
	OnQuickSlotChanged.Broadcast(QuickSlotIndex, EntryId);
	LogTransaction(TEXT("AssignQuickSlot"), Slot->ItemId, 1, true, FString::Printf(TEXT("Slot %d"), QuickSlotIndex));
}

void UInventoryComponent::ClearQuickSlot(int32 QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= NumQuickSlots) return;

	QuickSlots[QuickSlotIndex] = 0;
	OnQuickSlotChanged.Broadcast(QuickSlotIndex, 0);
}

void UInventoryComponent::UseQuickSlot(int32 QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= NumQuickSlots) return;

	int64 EntryId = QuickSlots[QuickSlotIndex];
	if (EntryId != 0)
	{
		UseItem(EntryId);
	}
}

FInventorySlot UInventoryComponent::GetQuickSlotItem(int32 QuickSlotIndex) const
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= NumQuickSlots)
	{
		return FInventorySlot();
	}

	int64 EntryId = QuickSlots[QuickSlotIndex];
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	return Slot ? *Slot : FInventorySlot();
}

// ============================================================================
// PHASE 8.7: TOOLTIPS
// ============================================================================

FItemTooltip UInventoryComponent::GetItemTooltip(int64 EntryId) const
{
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	FItemTooltip Tooltip;

	if (!Slot) return Tooltip;

	Tooltip.Name = Slot->DisplayName;
	Tooltip.Rarity = Slot->Rarity;
	Tooltip.Type = Slot->ItemType;
	Tooltip.Description = Slot->Description;
	Tooltip.Weight = Slot->Weight;
	Tooltip.bIsFavorite = Slot->bIsFavorite;
	Tooltip.bIsLocked = Slot->bIsLocked;

	// Build stat lines
	for (const auto& Stat : Slot->Stats)
	{
		Tooltip.StatLines.Add(FString::Printf(TEXT("%s: %.1f"), *Stat.Key, Stat.Value));
	}

	// Durability text
	if (Slot->bHasDurability)
	{
		Tooltip.DurabilityText = FString::Printf(TEXT("%.0f / %.0f"), Slot->CurrentDurability, Slot->MaxDurability);
	}

	return Tooltip;
}

FItemTooltip UInventoryComponent::GetItemTooltipBySlot(int32 SlotIndex) const
{
	FInventorySlot Slot = GetItemAtSlot(SlotIndex);
	if (Slot.IsEmpty()) return FItemTooltip();

	return GetItemTooltip(Slot.EntryId);
}

// ============================================================================
// PHASE 8.8: RARITY SYSTEM
// ============================================================================

TArray<FInventorySlot> UInventoryComponent::GetItemsByRarity(EItemRarity MinRarity) const
{
	TArray<FInventorySlot> Result;
	for (const FInventorySlot& Slot : Items)
	{
		if (static_cast<int32>(Slot.Rarity) >= static_cast<int32>(MinRarity))
		{
			Result.Add(Slot);
		}
	}
	return Result;
}

FLinearColor UInventoryComponent::GetRarityColor(EItemRarity Rarity)
{
	switch (Rarity)
	{
		case EItemRarity::Common:    return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray
		case EItemRarity::Uncommon:  return FLinearColor(0.0f, 0.8f, 0.0f, 1.0f); // Green
		case EItemRarity::Rare:      return FLinearColor(0.0f, 0.4f, 1.0f, 1.0f); // Blue
		case EItemRarity::Epic:      return FLinearColor(0.6f, 0.0f, 0.8f, 1.0f); // Purple
		case EItemRarity::Legendary: return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange
		default: return FLinearColor::White;
	}
}

FString UInventoryComponent::GetRarityName(EItemRarity Rarity)
{
	switch (Rarity)
	{
		case EItemRarity::Common:    return TEXT("Common");
		case EItemRarity::Uncommon:  return TEXT("Uncommon");
		case EItemRarity::Rare:      return TEXT("Rare");
		case EItemRarity::Epic:      return TEXT("Epic");
		case EItemRarity::Legendary: return TEXT("Legendary");
		default: return TEXT("Unknown");
	}
}

// ============================================================================
// PHASE 8.9: DURABILITY SYSTEM
// ============================================================================

void UInventoryComponent::ReduceDurability(int64 EntryId, float Amount)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (!Slot || !Slot->bHasDurability) return;

	float OldDurability = Slot->CurrentDurability;
	Slot->CurrentDurability = FMath::Max(0.0f, Slot->CurrentDurability - Amount);

	if (Slot->CurrentDurability != OldDurability)
	{
		OnItemDurabilityChanged.Broadcast(EntryId, Slot->CurrentDurability);
		OnInventoryChanged.Broadcast();
	}
}

void UInventoryComponent::RepairItem(int64 EntryId, float Amount)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (!Slot || !Slot->bHasDurability) return;

	float OldDurability = Slot->CurrentDurability;
	Slot->CurrentDurability = FMath::Min(Slot->MaxDurability, Slot->CurrentDurability + Amount);

	if (Slot->CurrentDurability != OldDurability)
	{
		OnItemDurabilityChanged.Broadcast(EntryId, Slot->CurrentDurability);
		OnInventoryChanged.Broadcast();
		LogTransaction(TEXT("Repair"), Slot->ItemId, 1, true, FString::Printf(TEXT("Repaired %.0f"), Amount));
	}
}

void UInventoryComponent::FullyRepairItem(int64 EntryId)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (!Slot || !Slot->bHasDurability) return;

	RepairItem(EntryId, Slot->MaxDurability);
}

float UInventoryComponent::GetDurabilityPercentage(int64 EntryId) const
{
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	if (!Slot || !Slot->bHasDurability || Slot->MaxDurability <= 0.0f) return 100.0f;

	return (Slot->CurrentDurability / Slot->MaxDurability) * 100.0f;
}

bool UInventoryComponent::IsItemBroken(int64 EntryId) const
{
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	if (!Slot || !Slot->bHasDurability) return false;

	return Slot->CurrentDurability <= 0.0f;
}

TArray<FInventorySlot> UInventoryComponent::GetItemsNeedingRepair(float DurabilityThreshold) const
{
	TArray<FInventorySlot> Result;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.bHasDurability && GetDurabilityPercentage(Slot.EntryId) <= DurabilityThreshold)
		{
			Result.Add(Slot);
		}
	}
	return Result;
}

// ============================================================================
// PHASE 8.10: AUTO-SORT
// ============================================================================

void UInventoryComponent::AutoSort()
{
	// Default auto-sort: by type, then by name
	Items.Sort([](const FInventorySlot& A, const FInventorySlot& B) {
		if (A.ItemType != B.ItemType)
		{
			return A.ItemType < B.ItemType;
		}
		return A.DisplayName < B.DisplayName;
	});

	ReassignSlotIndices();
}

// ============================================================================
// PHASE 8.11: SEARCH/FILTER BY NAME
// ============================================================================

TArray<FInventorySlot> UInventoryComponent::SearchItems(const FString& SearchQuery) const
{
	if (SearchQuery.IsEmpty())
	{
		return Items;
	}

	TArray<FInventorySlot> Result;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.DisplayName.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
		    Slot.ItemId.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
		    Slot.Description.Contains(SearchQuery, ESearchCase::IgnoreCase))
		{
			Result.Add(Slot);
		}
	}
	return Result;
}

void UInventoryComponent::SetSearchQuery(const FString& Query)
{
	CurrentSearchQuery = Query;
	OnInventoryChanged.Broadcast();
}

// ============================================================================
// PHASE 8.12: EQUIPMENT SLOTS
// ============================================================================

bool UInventoryComponent::EquipItem(int64 EntryId, EEquipmentSlot TargetSlot)
{
	if (TargetSlot == EEquipmentSlot::None) return false;

	FInventorySlot* ItemSlot = FindItemByEntryId(EntryId);
	if (!ItemSlot) return false;

	// Check if item can be equipped to this slot
	if (!CanEquipToSlot(EntryId, TargetSlot))
	{
		LogTransaction(TEXT("Equip"), ItemSlot->ItemId, 1, false, TEXT("Cannot equip to this slot"));
		return false;
	}

	// Unequip current item in slot if any
	if (EquippedItems.Contains(TargetSlot))
	{
		UnequipItem(TargetSlot);
	}

	// Equip the item
	EquippedItems.Add(TargetSlot, *ItemSlot);
	OnEquipmentChanged.Broadcast(TargetSlot);
	LogTransaction(TEXT("Equip"), ItemSlot->ItemId, 1, true);
	return true;
}

bool UInventoryComponent::UnequipItem(EEquipmentSlot Slot)
{
	if (!EquippedItems.Contains(Slot)) return false;

	FInventorySlot RemovedItem = EquippedItems[Slot];
	EquippedItems.Remove(Slot);
	OnEquipmentChanged.Broadcast(Slot);
	LogTransaction(TEXT("Unequip"), RemovedItem.ItemId, 1, true);
	return true;
}

FEquippedItem UInventoryComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	FEquippedItem Result;
	Result.Slot = Slot;

	if (const FInventorySlot* Found = EquippedItems.Find(Slot))
	{
		Result.Item = *Found;
	}

	return Result;
}

TArray<FEquippedItem> UInventoryComponent::GetAllEquippedItems() const
{
	TArray<FEquippedItem> Result;
	for (const auto& Pair : EquippedItems)
	{
		FEquippedItem Item;
		Item.Slot = Pair.Key;
		Item.Item = Pair.Value;
		Result.Add(Item);
	}
	return Result;
}

bool UInventoryComponent::IsSlotEquipped(EEquipmentSlot Slot) const
{
	return EquippedItems.Contains(Slot) && !EquippedItems[Slot].IsEmpty();
}

bool UInventoryComponent::CanEquipToSlot(int64 EntryId, EEquipmentSlot Slot) const
{
	const FInventorySlot* ItemSlot = FindItemByEntryIdConst(EntryId);
	if (!ItemSlot) return false;

	// If item has specific equip slot, must match
	if (ItemSlot->EquipSlot != EEquipmentSlot::None)
	{
		return ItemSlot->EquipSlot == Slot;
	}

	// Otherwise, check by item type
	FString Type = ItemSlot->ItemType.ToLower();

	if (Type == TEXT("weapon"))
	{
		return Slot == EEquipmentSlot::MainHand || Slot == EEquipmentSlot::OffHand;
	}
	else if (Type == TEXT("armor"))
	{
		return Slot == EEquipmentSlot::Head || Slot == EEquipmentSlot::Chest ||
		       Slot == EEquipmentSlot::Legs || Slot == EEquipmentSlot::Feet;
	}
	else if (Type == TEXT("accessory"))
	{
		return Slot == EEquipmentSlot::Accessory1 || Slot == EEquipmentSlot::Accessory2;
	}

	return false;
}

// ============================================================================
// PHASE 8.13: ITEM COMPARISON
// ============================================================================

FItemComparison UInventoryComponent::CompareItems(int64 EntryIdA, int64 EntryIdB) const
{
	FItemComparison Comparison;

	const FInventorySlot* SlotA = FindItemByEntryIdConst(EntryIdA);
	const FInventorySlot* SlotB = FindItemByEntryIdConst(EntryIdB);

	if (SlotA) Comparison.ItemA = *SlotA;
	if (SlotB) Comparison.ItemB = *SlotB;

	if (!SlotA || !SlotB) return Comparison;

	Comparison.WeightDifference = SlotA->Weight - SlotB->Weight;
	Comparison.RarityDifference = static_cast<int32>(SlotA->Rarity) - static_cast<int32>(SlotB->Rarity);

	// Compare stats
	TSet<FString> AllStats;
	for (const auto& Stat : SlotA->Stats) AllStats.Add(Stat.Key);
	for (const auto& Stat : SlotB->Stats) AllStats.Add(Stat.Key);

	for (const FString& StatName : AllStats)
	{
		float ValueA = SlotA->Stats.Contains(StatName) ? SlotA->Stats[StatName] : 0.0f;
		float ValueB = SlotB->Stats.Contains(StatName) ? SlotB->Stats[StatName] : 0.0f;
		Comparison.StatDifferences.Add(StatName, ValueA - ValueB);
	}

	return Comparison;
}

FItemComparison UInventoryComponent::CompareWithEquipped(int64 EntryId, EEquipmentSlot Slot) const
{
	FEquippedItem Equipped = GetEquippedItem(Slot);
	if (Equipped.IsEmpty())
	{
		FItemComparison Comparison;
		const FInventorySlot* ItemSlot = FindItemByEntryIdConst(EntryId);
		if (ItemSlot) Comparison.ItemA = *ItemSlot;
		return Comparison;
	}

	return CompareItems(EntryId, Equipped.Item.EntryId);
}

// ============================================================================
// PHASE 8.14: BULK OPERATIONS
// ============================================================================

int32 UInventoryComponent::AddItemsBulk(const TArray<FString>& ItemIds, const TArray<int32>& Quantities)
{
	int32 SuccessCount = 0;
	int32 Count = FMath::Min(ItemIds.Num(), Quantities.Num());

	for (int32 i = 0; i < Count; ++i)
	{
		AddItem(ItemIds[i], Quantities[i]);
		++SuccessCount;
	}

	return SuccessCount;
}

int32 UInventoryComponent::RemoveItemsBulk(const TArray<int64>& EntryIds, const TArray<int32>& Quantities)
{
	int32 SuccessCount = 0;
	int32 Count = FMath::Min(EntryIds.Num(), Quantities.Num());

	for (int32 i = 0; i < Count; ++i)
	{
		if (!IsItemLocked(EntryIds[i]))
		{
			RemoveItem(EntryIds[i], Quantities[i]);
			++SuccessCount;
		}
	}

	return SuccessCount;
}

int32 UInventoryComponent::RemoveAllOfItem(const FString& ItemId)
{
	int32 RemovedCount = 0;

	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].ItemId == ItemId && !Items[i].bIsLocked)
		{
			RemovedCount += Items[i].Quantity;
			LogTransaction(TEXT("RemoveAll"), ItemId, Items[i].Quantity, true);
			Items.RemoveAt(i);
		}
	}

	if (RemovedCount > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return RemovedCount;
}

void UInventoryComponent::ClearInventory(bool bIncludeLocked)
{
	if (bIncludeLocked)
	{
		Items.Empty();
	}
	else
	{
		Items.RemoveAll([](const FInventorySlot& Slot) { return !Slot.bIsLocked; });
	}

	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Clear"), TEXT(""), 0, true, bIncludeLocked ? TEXT("All items") : TEXT("Unlocked items only"));
}

// ============================================================================
// PHASE 8.15: LOCAL PERSISTENCE
// ============================================================================

bool UInventoryComponent::SaveInventoryToLocal()
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> ItemsArray;

	for (const FInventorySlot& Slot : Items)
	{
		TSharedPtr<FJsonObject> ItemObj = MakeShareable(new FJsonObject());
		ItemObj->SetNumberField(TEXT("entry_id"), Slot.EntryId);
		ItemObj->SetStringField(TEXT("item_id"), Slot.ItemId);
		ItemObj->SetStringField(TEXT("display_name"), Slot.DisplayName);
		ItemObj->SetNumberField(TEXT("quantity"), Slot.Quantity);
		ItemObj->SetNumberField(TEXT("max_stack"), Slot.MaxStack);
		ItemObj->SetNumberField(TEXT("slot_index"), Slot.SlotIndex);
		ItemObj->SetStringField(TEXT("item_type"), Slot.ItemType);
		ItemObj->SetNumberField(TEXT("weight"), Slot.Weight);
		ItemObj->SetNumberField(TEXT("rarity"), static_cast<int32>(Slot.Rarity));
		ItemObj->SetNumberField(TEXT("durability"), Slot.CurrentDurability);
		ItemObj->SetNumberField(TEXT("max_durability"), Slot.MaxDurability);
		ItemObj->SetBoolField(TEXT("has_durability"), Slot.bHasDurability);
		ItemObj->SetBoolField(TEXT("is_favorite"), Slot.bIsFavorite);
		ItemObj->SetBoolField(TEXT("is_locked"), Slot.bIsLocked);
		ItemObj->SetStringField(TEXT("description"), Slot.Description);

		ItemsArray.Add(MakeShareable(new FJsonValueObject(ItemObj)));
	}

	RootObject->SetArrayField(TEXT("items"), ItemsArray);
	RootObject->SetNumberField(TEXT("next_entry_id"), NextLocalEntryId);
	RootObject->SetNumberField(TEXT("capacity_level"), CapacityLevel);
	RootObject->SetNumberField(TEXT("max_slots"), MaxSlots);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	bool bSuccess = FFileHelper::SaveStringToFile(OutputString, *GetSaveFilePath());
	LogTransaction(TEXT("Save"), TEXT(""), Items.Num(), bSuccess);
	return bSuccess;
}

bool UInventoryComponent::LoadInventoryFromLocal()
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *GetSaveFilePath()))
	{
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	Items.Empty();

	const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
	if (RootObject->TryGetArrayField(TEXT("items"), ItemsArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *ItemsArray)
		{
			TSharedPtr<FJsonObject> ItemObj = Value->AsObject();
			if (!ItemObj.IsValid()) continue;

			FInventorySlot Slot;
			ItemObj->TryGetNumberField(TEXT("entry_id"), Slot.EntryId);
			ItemObj->TryGetStringField(TEXT("item_id"), Slot.ItemId);
			ItemObj->TryGetStringField(TEXT("display_name"), Slot.DisplayName);
			ItemObj->TryGetNumberField(TEXT("quantity"), Slot.Quantity);
			ItemObj->TryGetNumberField(TEXT("max_stack"), Slot.MaxStack);
			ItemObj->TryGetNumberField(TEXT("slot_index"), Slot.SlotIndex);
			ItemObj->TryGetStringField(TEXT("item_type"), Slot.ItemType);

			double TempDouble;
			if (ItemObj->TryGetNumberField(TEXT("weight"), TempDouble))
				Slot.Weight = static_cast<float>(TempDouble);
			if (ItemObj->TryGetNumberField(TEXT("durability"), TempDouble))
				Slot.CurrentDurability = static_cast<float>(TempDouble);
			if (ItemObj->TryGetNumberField(TEXT("max_durability"), TempDouble))
				Slot.MaxDurability = static_cast<float>(TempDouble);

			int32 RarityInt;
			if (ItemObj->TryGetNumberField(TEXT("rarity"), RarityInt))
				Slot.Rarity = static_cast<EItemRarity>(FMath::Clamp(RarityInt, 0, 4));

			ItemObj->TryGetBoolField(TEXT("has_durability"), Slot.bHasDurability);
			ItemObj->TryGetBoolField(TEXT("is_favorite"), Slot.bIsFavorite);
			ItemObj->TryGetBoolField(TEXT("is_locked"), Slot.bIsLocked);
			ItemObj->TryGetStringField(TEXT("description"), Slot.Description);

			Items.Add(Slot);
		}
	}

	RootObject->TryGetNumberField(TEXT("next_entry_id"), NextLocalEntryId);
	RootObject->TryGetNumberField(TEXT("capacity_level"), CapacityLevel);
	RootObject->TryGetNumberField(TEXT("max_slots"), MaxSlots);

	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("Load"), TEXT(""), Items.Num(), true);
	return true;
}

FString UInventoryComponent::GetSaveFilePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("Inventory.json");
}

bool UInventoryComponent::HasLocalSave() const
{
	return FPlatformFileManager::Get().GetPlatformFile().FileExists(*GetSaveFilePath());
}

// ============================================================================
// PHASE 8.16: OVERFLOW HANDLING
// ============================================================================

bool UInventoryComponent::ClaimOverflowItem(int32 OverflowIndex)
{
	if (OverflowIndex < 0 || OverflowIndex >= OverflowItems.Num()) return false;
	if (Items.Num() >= MaxSlots) return false;

	FInventorySlot Item = OverflowItems[OverflowIndex];
	OverflowItems.RemoveAt(OverflowIndex);

	Item.SlotIndex = FindFirstEmptySlotIndex();
	Items.Add(Item);

	OnInventoryChanged.Broadcast();
	LogTransaction(TEXT("ClaimOverflow"), Item.ItemId, Item.Quantity, true);
	return true;
}

void UInventoryComponent::ClearOverflow()
{
	OverflowItems.Empty();
}

// ============================================================================
// PHASE 8.17: ITEM VALIDATION
// ============================================================================

bool UInventoryComponent::ValidateItem(const FInventorySlot& Item) const
{
	if (Item.ItemId.IsEmpty()) return false;
	if (Item.Quantity <= 0) return false;
	if (Item.Quantity > Item.MaxStack) return false;
	if (Item.SlotIndex < 0 || Item.SlotIndex >= MaxSlots) return false;
	if (Item.Weight < 0.0f) return false;
	if (Item.bHasDurability && Item.MaxDurability <= 0.0f) return false;

	return true;
}

bool UInventoryComponent::ValidateInventory() const
{
	TSet<int32> UsedSlots;
	TSet<int64> UsedEntryIds;

	for (const FInventorySlot& Slot : Items)
	{
		if (!ValidateItem(Slot)) return false;

		// Check for duplicate slots
		if (UsedSlots.Contains(Slot.SlotIndex)) return false;
		UsedSlots.Add(Slot.SlotIndex);

		// Check for duplicate entry IDs
		if (UsedEntryIds.Contains(Slot.EntryId)) return false;
		UsedEntryIds.Add(Slot.EntryId);
	}

	return true;
}

TArray<FString> UInventoryComponent::GetValidationErrors() const
{
	TArray<FString> Errors;
	TSet<int32> UsedSlots;
	TSet<int64> UsedEntryIds;

	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.ItemId.IsEmpty())
			Errors.Add(FString::Printf(TEXT("Entry %lld: Empty item ID"), Slot.EntryId));
		if (Slot.Quantity <= 0)
			Errors.Add(FString::Printf(TEXT("Entry %lld: Invalid quantity %d"), Slot.EntryId, Slot.Quantity));
		if (Slot.Quantity > Slot.MaxStack)
			Errors.Add(FString::Printf(TEXT("Entry %lld: Quantity %d exceeds max stack %d"), Slot.EntryId, Slot.Quantity, Slot.MaxStack));
		if (Slot.SlotIndex < 0 || Slot.SlotIndex >= MaxSlots)
			Errors.Add(FString::Printf(TEXT("Entry %lld: Invalid slot index %d"), Slot.EntryId, Slot.SlotIndex));

		if (UsedSlots.Contains(Slot.SlotIndex))
			Errors.Add(FString::Printf(TEXT("Duplicate slot index: %d"), Slot.SlotIndex));
		UsedSlots.Add(Slot.SlotIndex);

		if (UsedEntryIds.Contains(Slot.EntryId))
			Errors.Add(FString::Printf(TEXT("Duplicate entry ID: %lld"), Slot.EntryId));
		UsedEntryIds.Add(Slot.EntryId);
	}

	return Errors;
}

// ============================================================================
// PHASE 8.18: TRANSACTION LOGGING
// ============================================================================

TArray<FInventoryTransaction> UInventoryComponent::GetTransactionHistory(int32 MaxEntries) const
{
	int32 StartIndex = FMath::Max(0, TransactionLog.Num() - MaxEntries);
	TArray<FInventoryTransaction> Result;

	for (int32 i = StartIndex; i < TransactionLog.Num(); ++i)
	{
		Result.Add(TransactionLog[i]);
	}

	return Result;
}

void UInventoryComponent::ClearTransactionHistory()
{
	TransactionLog.Empty();
}

void UInventoryComponent::LogTransaction(const FString& Action, const FString& ItemId, int32 Quantity, bool bSuccess, const FString& Details)
{
	if (!bTransactionLoggingEnabled) return;

	FInventoryTransaction Transaction;
	Transaction.Timestamp = FDateTime::Now();
	Transaction.Action = Action;
	Transaction.ItemId = ItemId;
	Transaction.Quantity = Quantity;
	Transaction.bSuccess = bSuccess;
	Transaction.Details = Details;

	TransactionLog.Add(Transaction);

	// Trim log if too large
	if (TransactionLog.Num() > MaxTransactionLogSize)
	{
		TransactionLog.RemoveAt(0, TransactionLog.Num() - MaxTransactionLogSize);
	}

	OnTransactionLogged.Broadcast(Transaction);

	UE_LOG(LogTemp, Log, TEXT("Inventory Transaction: %s %s x%d - %s %s"),
		*Action, *ItemId, Quantity, bSuccess ? TEXT("Success") : TEXT("Failed"), *Details);
}

// ============================================================================
// PHASE 8.19: CAPACITY EXPANSION
// ============================================================================

bool UInventoryComponent::ExpandCapacity(int32 AdditionalSlots)
{
	if (!CanExpandCapacity()) return false;

	MaxSlots += AdditionalSlots;
	CapacityLevel++;

	OnCapacityChanged.Broadcast(MaxSlots);
	LogTransaction(TEXT("ExpandCapacity"), TEXT(""), AdditionalSlots, true,
		FString::Printf(TEXT("New capacity: %d slots"), MaxSlots));
	return true;
}

int32 UInventoryComponent::GetSlotsForLevel(int32 Level) const
{
	return 20 + (Level * SlotsPerCapacityLevel);
}

bool UInventoryComponent::CanExpandCapacity() const
{
	return CapacityLevel < MaxCapacityLevel;
}

// ============================================================================
// PHASE 8.20: FAVORITES & LOCKING
// ============================================================================

void UInventoryComponent::ToggleFavorite(int64 EntryId)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (Slot)
	{
		Slot->bIsFavorite = !Slot->bIsFavorite;
		OnInventoryChanged.Broadcast();
	}
}

void UInventoryComponent::SetFavorite(int64 EntryId, bool bFavorite)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (Slot && Slot->bIsFavorite != bFavorite)
	{
		Slot->bIsFavorite = bFavorite;
		OnInventoryChanged.Broadcast();
	}
}

TArray<FInventorySlot> UInventoryComponent::GetFavoriteItems() const
{
	TArray<FInventorySlot> Result;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.bIsFavorite)
		{
			Result.Add(Slot);
		}
	}
	return Result;
}

void UInventoryComponent::ToggleLock(int64 EntryId)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (Slot)
	{
		Slot->bIsLocked = !Slot->bIsLocked;
		OnInventoryChanged.Broadcast();
	}
}

void UInventoryComponent::SetLocked(int64 EntryId, bool bLocked)
{
	FInventorySlot* Slot = FindItemByEntryId(EntryId);
	if (Slot && Slot->bIsLocked != bLocked)
	{
		Slot->bIsLocked = bLocked;
		OnInventoryChanged.Broadcast();
	}
}

TArray<FInventorySlot> UInventoryComponent::GetLockedItems() const
{
	TArray<FInventorySlot> Result;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.bIsLocked)
		{
			Result.Add(Slot);
		}
	}
	return Result;
}

bool UInventoryComponent::IsItemLocked(int64 EntryId) const
{
	const FInventorySlot* Slot = FindItemByEntryIdConst(EntryId);
	return Slot ? Slot->bIsLocked : false;
}

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

void UInventoryComponent::RefreshFromServer()
{
	// Trigger a re-subscribe to get fresh data
}

void UInventoryComponent::AddItemLocal(const FString& ItemId, int32 Quantity)
{
	// Check weight capacity
	FInventorySlot TempSlot = CreateItemSlot(ItemId, Quantity);
	if (!CanCarryWeight(TempSlot.GetTotalWeight()))
	{
		// Add to overflow
		TempSlot.SlotIndex = -1;
		OverflowItems.Add(TempSlot);
		OnInventoryOverflow.Broadcast(ItemId, Quantity);
		LogTransaction(TEXT("Add"), ItemId, Quantity, false, TEXT("Exceeded weight capacity - added to overflow"));
		return;
	}

	// Check if item already exists and is stackable
	for (FInventorySlot& Slot : Items)
	{
		if (Slot.ItemId == ItemId && Slot.Quantity < Slot.MaxStack)
		{
			int32 SpaceLeft = Slot.MaxStack - Slot.Quantity;
			int32 ToAdd = FMath::Min(Quantity, SpaceLeft);
			Slot.Quantity += ToAdd;
			Quantity -= ToAdd;

			if (Quantity <= 0)
			{
				OnInventoryChanged.Broadcast();
				if (bAutoSortEnabled) AutoSort();
				LogTransaction(TEXT("Add"), ItemId, ToAdd, true, TEXT("Stacked"));
				return;
			}
		}
	}

	// Need a new slot
	if (Items.Num() < MaxSlots && Quantity > 0)
	{
		FInventorySlot NewSlot = CreateItemSlot(ItemId, Quantity);
		NewSlot.SlotIndex = FindFirstEmptySlotIndex();

		Items.Add(NewSlot);
		OnInventoryChanged.Broadcast();
		if (bAutoSortEnabled) AutoSort();
		LogTransaction(TEXT("Add"), ItemId, Quantity, true);
	}
	else if (Quantity > 0)
	{
		// Overflow
		FInventorySlot OverflowSlot = CreateItemSlot(ItemId, Quantity);
		OverflowSlot.SlotIndex = -1;
		OverflowItems.Add(OverflowSlot);
		OnInventoryOverflow.Broadcast(ItemId, Quantity);
		LogTransaction(TEXT("Add"), ItemId, Quantity, false, TEXT("No room - added to overflow"));
	}
}

void UInventoryComponent::RemoveItemLocal(int64 EntryId, int32 Quantity)
{
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].EntryId == EntryId)
		{
			FString ItemId = Items[i].ItemId;
			Items[i].Quantity -= Quantity;
			if (Items[i].Quantity <= 0)
			{
				Items.RemoveAt(i);
			}
			OnInventoryChanged.Broadcast();
			LogTransaction(TEXT("Remove"), ItemId, Quantity, true);
			return;
		}
	}
}

FInventorySlot* UInventoryComponent::FindItemByEntryId(int64 EntryId)
{
	return Items.FindByPredicate([EntryId](const FInventorySlot& S) {
		return S.EntryId == EntryId;
	});
}

const FInventorySlot* UInventoryComponent::FindItemByEntryIdConst(int64 EntryId) const
{
	return Items.FindByPredicate([EntryId](const FInventorySlot& S) {
		return S.EntryId == EntryId;
	});
}

int32 UInventoryComponent::FindFirstEmptySlotIndex() const
{
	TSet<int32> UsedSlots;
	for (const FInventorySlot& Slot : Items)
	{
		UsedSlots.Add(Slot.SlotIndex);
	}

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (!UsedSlots.Contains(i))
		{
			return i;
		}
	}

	return Items.Num();
}

void UInventoryComponent::ReassignSlotIndices()
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		Items[i].SlotIndex = i;
	}
}

FInventorySlot UInventoryComponent::CreateItemSlot(const FString& ItemId, int32 Quantity)
{
	FInventorySlot NewSlot;
	NewSlot.EntryId = NextLocalEntryId++;
	NewSlot.ItemId = ItemId;
	NewSlot.DisplayName = ItemId; // Would look up from item definitions
	NewSlot.Quantity = Quantity;
	NewSlot.MaxStack = 99; // Default max stack
	NewSlot.ItemType = TEXT("misc"); // Would look up from item definitions
	NewSlot.Weight = 1.0f; // Default weight
	NewSlot.Rarity = EItemRarity::Common;

	// Specific item defaults
	if (ItemId.Contains(TEXT("potion")))
	{
		NewSlot.ItemType = TEXT("consumable");
		NewSlot.MaxStack = 10;
		NewSlot.Weight = 0.5f;
	}
	else if (ItemId.Contains(TEXT("sword")) || ItemId.Contains(TEXT("axe")))
	{
		NewSlot.ItemType = TEXT("weapon");
		NewSlot.MaxStack = 1;
		NewSlot.Weight = 5.0f;
		NewSlot.bHasDurability = true;
		NewSlot.EquipSlot = EEquipmentSlot::MainHand;
	}
	else if (ItemId.Contains(TEXT("shield")))
	{
		NewSlot.ItemType = TEXT("weapon");
		NewSlot.MaxStack = 1;
		NewSlot.Weight = 4.0f;
		NewSlot.bHasDurability = true;
		NewSlot.EquipSlot = EEquipmentSlot::OffHand;
	}
	else if (ItemId.Contains(TEXT("helm")) || ItemId.Contains(TEXT("hat")))
	{
		NewSlot.ItemType = TEXT("armor");
		NewSlot.MaxStack = 1;
		NewSlot.Weight = 2.0f;
		NewSlot.bHasDurability = true;
		NewSlot.EquipSlot = EEquipmentSlot::Head;
	}
	else if (ItemId.Contains(TEXT("coin")) || ItemId.Contains(TEXT("gold")))
	{
		NewSlot.ItemType = TEXT("resource");
		NewSlot.MaxStack = 999;
		NewSlot.Weight = 0.01f;
	}
	else if (ItemId.Contains(TEXT("ring")) || ItemId.Contains(TEXT("amulet")))
	{
		NewSlot.ItemType = TEXT("accessory");
		NewSlot.MaxStack = 1;
		NewSlot.Weight = 0.1f;
		NewSlot.EquipSlot = EEquipmentSlot::Accessory1;
	}

	return NewSlot;
}
