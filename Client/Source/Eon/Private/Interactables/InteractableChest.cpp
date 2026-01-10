// Copyright 2026 tbassignana. MIT License.

#include "Interactables/InteractableChest.h"
#include "InventoryComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableChest::AInteractableChest()
{
	PrimaryActorTick.bCanEverTick = false;

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	RootComponent = ChestMesh;

	// Generate unique ID if not set
	ChestId = FGuid::NewGuid().ToString();
}

void AInteractableChest::BeginPlay()
{
	Super::BeginPlay();

	// Query SpaceTimeDB for existing state
	// This would check if chest has already been opened in this instance
}

bool AInteractableChest::CanInteract(AActor* Interactor) const
{
	if (bIsOpen) return false;

	// Check if locked and player has key
	if (bIsLocked && !RequiredKeyItem.IsEmpty())
	{
		if (APawn* Pawn = Cast<APawn>(Interactor))
		{
			if (UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>())
			{
				return Inventory->HasItem(RequiredKeyItem, 1);
			}
		}
		return false;
	}

	return true;
}

void AInteractableChest::OnInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor)) return;

	// Use key if required
	if (bIsLocked && bDestroyKeyOnUse)
	{
		if (APawn* Pawn = Cast<APawn>(Interactor))
		{
			if (UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>())
			{
				// Find and remove the key - simplified
				// In full implementation, would find exact entry_id
			}
		}
	}

	OpenChest(Interactor);
}

void AInteractableChest::OnBeginFocus(AActor* Interactor)
{
	// Highlight effect would go here
	UE_LOG(LogTemp, Log, TEXT("Chest: Begin focus"));
}

void AInteractableChest::OnEndFocus(AActor* Interactor)
{
	// Remove highlight effect
	UE_LOG(LogTemp, Log, TEXT("Chest: End focus"));
}

FString AInteractableChest::GetInteractionPrompt() const
{
	if (bIsOpen) return TEXT("");

	if (bIsLocked && !RequiredKeyItem.IsEmpty())
	{
		return FString::Printf(TEXT("Unlock (requires %s)"), *RequiredKeyItem);
	}

	return TEXT("Open Chest");
}

void AInteractableChest::SetLocked(bool bLocked, const FString& KeyItem)
{
	bIsLocked = bLocked;
	RequiredKeyItem = KeyItem;
}

void AInteractableChest::OpenChest(AActor* Interactor)
{
	if (bIsOpen) return;

	bIsOpen = true;
	UE_LOG(LogTemp, Log, TEXT("Chest: Opened by %s"), *Interactor->GetName());

	// Animate lid opening would go here

	if (!bHasBeenLooted)
	{
		SpawnLoot(Interactor);
		bHasBeenLooted = true;
	}

	SyncStateToServer();
}

void AInteractableChest::SpawnLoot(AActor* Interactor)
{
	APawn* Pawn = Cast<APawn>(Interactor);
	if (!Pawn) return;

	UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return;

	for (const FChestLoot& Loot : LootTable)
	{
		// Roll for drop chance
		float Roll = FMath::FRand();
		if (Roll <= Loot.DropChance)
		{
			Inventory->AddItem(Loot.ItemId, Loot.Quantity);
			UE_LOG(LogTemp, Log, TEXT("Chest: Gave %d x %s"), Loot.Quantity, *Loot.ItemId);
		}
	}
}

void AInteractableChest::SyncStateToServer()
{
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			FString StateJson = FString::Printf(TEXT("{\"is_open\":%s,\"has_been_looted\":%s}"),
				bIsOpen ? TEXT("true") : TEXT("false"),
				bHasBeenLooted ? TEXT("true") : TEXT("false"));

			// Manager->SetInteractableState(ChestId, InstanceId, bIsOpen, StateJson);
		}
	}
}
