// Copyright 2026 tbassignana. MIT License.

#include "InventoryComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Subscribe to inventory updates from SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			// Bind to inventory updates (will receive data once connected)
			Manager->OnInventoryUpdated.AddDynamic(this, &UInventoryComponent::OnInventoryDataReceived);
		}
	}
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

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
				return;
			}
		}
	}

	// Local-only mode: add directly to inventory
	AddItemLocal(ItemId, Quantity);
}

void UInventoryComponent::RemoveItem(int64 EntryId, int32 Quantity)
{
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->RemoveItemFromInventory(EntryId, Quantity);
		}
	}
}

void UInventoryComponent::UseItem(int64 EntryId)
{
	// Find the item first
	FInventorySlot* Slot = Items.FindByPredicate([EntryId](const FInventorySlot& S) {
		return S.EntryId == EntryId;
	});

	if (!Slot) return;

	// Check if item is consumable
	if (Slot->ItemType == TEXT("consumable"))
	{
		if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
			{
				Manager->UseConsumable(EntryId);
			}
		}
	}
}

void UInventoryComponent::MoveItem(int64 EntryId, int32 NewSlotIndex)
{
	if (NewSlotIndex < 0 || NewSlotIndex >= MaxSlots) return;

	// Local update for responsiveness
	for (FInventorySlot& Slot : Items)
	{
		if (Slot.EntryId == EntryId)
		{
			// Swap if destination has item
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

	// Server will sync the actual state
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

	// Parse the inventory update
	FInventorySlot NewSlot;
	JsonObject->TryGetNumberField(TEXT("entry_id"), NewSlot.EntryId);
	JsonObject->TryGetStringField(TEXT("item_id"), NewSlot.ItemId);
	JsonObject->TryGetNumberField(TEXT("quantity"), NewSlot.Quantity);
	JsonObject->TryGetNumberField(TEXT("slot_index"), NewSlot.SlotIndex);

	// Update or add slot
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

	// Remove if quantity is 0
	Items.RemoveAll([](const FInventorySlot& S) { return S.Quantity <= 0; });

	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::RefreshFromServer()
{
	// Trigger a re-subscribe to get fresh data
}

void UInventoryComponent::AddItemLocal(const FString& ItemId, int32 Quantity)
{
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
				UE_LOG(LogTemp, Log, TEXT("Inventory: Added %d x %s to existing stack"), ToAdd, *ItemId);
				return;
			}
		}
	}

	// Need a new slot
	if (Items.Num() < MaxSlots && Quantity > 0)
	{
		FInventorySlot NewSlot;
		NewSlot.EntryId = NextLocalEntryId++;
		NewSlot.ItemId = ItemId;
		NewSlot.DisplayName = ItemId; // Would look up from item definitions
		NewSlot.Quantity = Quantity;
		NewSlot.MaxStack = 99; // Default max stack
		NewSlot.SlotIndex = Items.Num();
		NewSlot.ItemType = TEXT("misc"); // Would look up from item definitions

		Items.Add(NewSlot);
		OnInventoryChanged.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Inventory: Added new item %s (x%d) at slot %d"), *ItemId, Quantity, NewSlot.SlotIndex);
	}
	else if (Quantity > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory: No room for %s"), *ItemId);
	}
}

void UInventoryComponent::RemoveItemLocal(int64 EntryId, int32 Quantity)
{
	for (int32 i = Items.Num() - 1; i >= 0; --i)
	{
		if (Items[i].EntryId == EntryId)
		{
			Items[i].Quantity -= Quantity;
			if (Items[i].Quantity <= 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Inventory: Removed item %s"), *Items[i].ItemId);
				Items.RemoveAt(i);
			}
			OnInventoryChanged.Broadcast();
			return;
		}
	}
}
