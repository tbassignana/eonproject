// Copyright 2026 tbassignana. MIT License.

#include "EonPlayerController.h"
#include "SpaceTimeDBManager.h"
#include "EonCharacter.h"
#include "InventoryComponent.h"
#include "WorldItemPickup.h"
#include "UI/EonHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEonPlayerController::AEonPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	DetectPlatform();

	// Get SpaceTimeDB manager and connect (only if enabled)
	if (bEnableSpaceTimeDB)
	{
		if (USpaceTimeDBManager* Manager = GetSpaceTimeDBManager())
		{
			Manager->OnConnected.AddDynamic(this, &AEonPlayerController::OnSpaceTimeDBConnected);
			FSpaceTimeDBConfig Config;
			Manager->Connect(Config);
		}
	}
}

void AEonPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Sync position at configured interval
	LastSyncTime += DeltaTime;
	if (LastSyncTime >= PositionSyncInterval)
	{
		SyncPlayerPosition();
		LastSyncTime = 0.0f;
	}
}

void AEonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

#if PLATFORM_IOS
	// iOS-specific input setup
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableTouchEvents = true;
#else
	bShowMouseCursor = false;
	bEnableClickEvents = true;
	bEnableTouchEvents = false;
#endif
}

USpaceTimeDBManager* AEonPlayerController::GetSpaceTimeDBManager() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<USpaceTimeDBManager>();
	}
	return nullptr;
}

void AEonPlayerController::SyncPlayerPosition()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (USpaceTimeDBManager* Manager = GetSpaceTimeDBManager())
		{
			if (Manager->IsConnected())
			{
				FVector Location = ControlledPawn->GetActorLocation();
				FRotator Rotation = ControlledPawn->GetActorRotation();
				Manager->UpdatePlayerPosition(Location, Rotation);
			}
		}
	}
}

void AEonPlayerController::DetectPlatform()
{
#if PLATFORM_IOS
	bIsMobileDevice = true;
#elif PLATFORM_MAC
	bIsMobileDevice = false;
#else
	bIsMobileDevice = false;
#endif
}

void AEonPlayerController::OnSpaceTimeDBConnected()
{
	UE_LOG(LogTemp, Log, TEXT("SpaceTimeDB: Connected! Registering player..."));

	if (USpaceTimeDBManager* Manager = GetSpaceTimeDBManager())
	{
		// Register this player with a username
		FString Username = FString::Printf(TEXT("Player_%d"), FMath::RandRange(1000, 9999));
		Manager->RegisterPlayer(Username);
		Manager->SetPlayerOnline(true);
	}
}

void AEonPlayerController::ShowInventoryUI(bool bShow)
{
	// UI implementation will be handled by widget system
	UE_LOG(LogTemp, Log, TEXT("ShowInventoryUI: %s"), bShow ? TEXT("true") : TEXT("false"));
}

void AEonPlayerController::ShowInstanceBrowser(bool bShow)
{
	UE_LOG(LogTemp, Log, TEXT("ShowInstanceBrowser: %s"), bShow ? TEXT("true") : TEXT("false"));
}

void AEonPlayerController::ShowCreateInstanceDialog()
{
	UE_LOG(LogTemp, Log, TEXT("ShowCreateInstanceDialog"));
}

void AEonPlayerController::SpawnTestItem(const FString& ItemId, int32 Quantity)
{
	if (!GetPawn()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLocation = GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * 200.0f;
	SpawnLocation.Z += 50.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWorldItemPickup* Pickup = World->SpawnActor<AWorldItemPickup>(
		AWorldItemPickup::StaticClass(),
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Pickup)
	{
		Pickup->Initialize(0, ItemId, Quantity, ItemId);
		UE_LOG(LogTemp, Log, TEXT("Debug: Spawned %d x %s at %s"), Quantity, *ItemId, *SpawnLocation.ToString());
	}
}

void AEonPlayerController::GiveItem(const FString& ItemId, int32 Quantity)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UInventoryComponent* Inventory = ControlledPawn->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->AddItem(ItemId, Quantity);
			UE_LOG(LogTemp, Log, TEXT("Debug: Gave %d x %s to player inventory"), Quantity, *ItemId);
		}
	}
}

void AEonPlayerController::ListInventory()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UInventoryComponent* Inventory = ControlledPawn->FindComponentByClass<UInventoryComponent>())
		{
			TArray<FInventorySlot> Items = Inventory->GetAllItems();
			UE_LOG(LogTemp, Log, TEXT("=== Inventory (%d items) ==="), Items.Num());
			for (const FInventorySlot& Slot : Items)
			{
				UE_LOG(LogTemp, Log, TEXT("  [%d] %s x%d (ID: %lld)"),
					Slot.SlotIndex, *Slot.DisplayName, Slot.Quantity, Slot.EntryId);
			}
			UE_LOG(LogTemp, Log, TEXT("========================"));
		}
	}
}
