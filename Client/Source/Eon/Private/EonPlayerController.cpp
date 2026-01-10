// Copyright 2026 tbassignana. MIT License.

#include "EonPlayerController.h"
#include "SpaceTimeDBManager.h"
#include "EonCharacter.h"
#include "UI/EonHUD.h"
#include "Kismet/GameplayStatics.h"

AEonPlayerController::AEonPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	DetectPlatform();

	// Get SpaceTimeDB manager and connect
	if (USpaceTimeDBManager* Manager = GetSpaceTimeDBManager())
	{
		FSpaceTimeDBConfig Config;
		Manager->Connect(Config);
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
