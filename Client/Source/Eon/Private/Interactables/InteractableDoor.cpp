// Copyright 2026 tbassignana. MIT License.

#include "Interactables/InteractableDoor.h"
#include "InventoryComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AInteractableDoor::AInteractableDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	RootComponent = DoorFrame;

	DoorPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanel"));
	DoorPanel->SetupAttachment(DoorFrame);

	DoorId = FGuid::NewGuid().ToString();
}

void AInteractableDoor::BeginPlay()
{
	Super::BeginPlay();
	InitialRotation = DoorPanel->GetRelativeRotation();
}

void AInteractableDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update door rotation animation
	if (DoorState == EDoorState::Opening || DoorState == EDoorState::Closing)
	{
		UpdateDoorRotation(DeltaTime);
	}

	// Auto close timer
	if (bAutoClose && DoorState == EDoorState::Open)
	{
		AutoCloseTimer += DeltaTime;
		if (AutoCloseTimer >= AutoCloseDelay)
		{
			Close();
		}
	}
}

bool AInteractableDoor::CanInteract(AActor* Interactor) const
{
	// Can always try to interact, but locked doors need key
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

void AInteractableDoor::OnInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor)) return;

	// Use key if required (first time only)
	if (bIsLocked && bDestroyKeyOnUse && DoorState == EDoorState::Closed)
	{
		bIsLocked = false; // Unlock permanently after using key
		// Key removal would happen here
	}

	Toggle();
}

void AInteractableDoor::OnBeginFocus(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("Door: Begin focus"));
}

void AInteractableDoor::OnEndFocus(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("Door: End focus"));
}

FString AInteractableDoor::GetInteractionPrompt() const
{
	if (bIsLocked && !RequiredKeyItem.IsEmpty())
	{
		return FString::Printf(TEXT("Unlock (requires %s)"), *RequiredKeyItem);
	}

	return (DoorState == EDoorState::Closed || DoorState == EDoorState::Closing)
		? TEXT("Open Door")
		: TEXT("Close Door");
}

void AInteractableDoor::SetLocked(bool bLocked, const FString& KeyItem)
{
	bIsLocked = bLocked;
	RequiredKeyItem = KeyItem;
}

void AInteractableDoor::Open()
{
	if (DoorState == EDoorState::Open || DoorState == EDoorState::Opening) return;

	DoorState = EDoorState::Opening;
	TargetAngle = OpenAngle;
	AutoCloseTimer = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Door: Opening"));
}

void AInteractableDoor::Close()
{
	if (DoorState == EDoorState::Closed || DoorState == EDoorState::Closing) return;

	DoorState = EDoorState::Closing;
	TargetAngle = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Door: Closing"));
}

void AInteractableDoor::Toggle()
{
	if (DoorState == EDoorState::Closed || DoorState == EDoorState::Closing)
	{
		Open();
	}
	else
	{
		Close();
	}
	SyncStateToServer();
}

void AInteractableDoor::UpdateDoorRotation(float DeltaTime)
{
	float Delta = OpenSpeed * DeltaTime * 100.0f;

	if (DoorState == EDoorState::Opening)
	{
		CurrentAngle = FMath::FInterpTo(CurrentAngle, TargetAngle, DeltaTime, OpenSpeed);
		if (FMath::IsNearlyEqual(CurrentAngle, OpenAngle, 0.5f))
		{
			CurrentAngle = OpenAngle;
			DoorState = EDoorState::Open;
		}
	}
	else if (DoorState == EDoorState::Closing)
	{
		CurrentAngle = FMath::FInterpTo(CurrentAngle, 0.0f, DeltaTime, OpenSpeed);
		if (FMath::IsNearlyEqual(CurrentAngle, 0.0f, 0.5f))
		{
			CurrentAngle = 0.0f;
			DoorState = EDoorState::Closed;
		}
	}

	FRotator NewRotation = InitialRotation;
	NewRotation.Yaw += CurrentAngle;
	DoorPanel->SetRelativeRotation(NewRotation);
}

void AInteractableDoor::SyncStateToServer()
{
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			bool bOpen = (DoorState == EDoorState::Open || DoorState == EDoorState::Opening);
			FString StateJson = FString::Printf(TEXT("{\"is_open\":%s,\"is_locked\":%s}"),
				bOpen ? TEXT("true") : TEXT("false"),
				bIsLocked ? TEXT("true") : TEXT("false"));

			// Manager->SetInteractableState(DoorId, InstanceId, bOpen, StateJson);
		}
	}
}
