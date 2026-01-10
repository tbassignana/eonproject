// Copyright 2026 tbassignana. MIT License.

#include "WorldItemPickup.h"
#include "InventoryComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AWorldItemPickup::AWorldItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// Disable collision for visual-only pickup
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // Interaction channel
}

void AWorldItemPickup::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
}

void AWorldItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCollected)
	{
		UpdateAnimation(DeltaTime);
	}
}

bool AWorldItemPickup::CanInteract(AActor* Interactor) const
{
	return !bIsCollected && !ItemId.IsEmpty();
}

void AWorldItemPickup::OnInteract(AActor* Interactor)
{
	if (!CanInteract(Interactor)) return;
	Collect(Interactor);
}

void AWorldItemPickup::OnBeginFocus(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("WorldItem: Begin focus on %s"), *DisplayName);
	// Highlight effect
}

void AWorldItemPickup::OnEndFocus(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("WorldItem: End focus"));
	// Remove highlight
}

FString AWorldItemPickup::GetInteractionPrompt() const
{
	if (Quantity > 1)
	{
		return FString::Printf(TEXT("Pick up %s (x%d)"), *DisplayName, Quantity);
	}
	return FString::Printf(TEXT("Pick up %s"), *DisplayName);
}

void AWorldItemPickup::Initialize(int64 InWorldItemId, const FString& InItemId, int32 InQuantity)
{
	WorldItemId = InWorldItemId;
	ItemId = InItemId;
	Quantity = InQuantity;

	// Would load item definition to get display name, mesh, etc.
	DisplayName = ItemId; // Placeholder
}

void AWorldItemPickup::UpdateAnimation(float DeltaTime)
{
	FVector NewLocation = InitialLocation;
	FRotator NewRotation = GetActorRotation();

	if (bBobUpAndDown)
	{
		BobTime += DeltaTime * BobSpeed;
		NewLocation.Z += FMath::Sin(BobTime) * BobHeight;
	}

	if (bRotate)
	{
		NewRotation.Yaw += RotationSpeed * DeltaTime;
	}

	SetActorLocation(NewLocation);
	SetActorRotation(NewRotation);
}

void AWorldItemPickup::Collect(AActor* Interactor)
{
	if (bIsCollected) return;

	bIsCollected = true;

	// Notify server via SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->CollectWorldItem(WorldItemId);
		}
	}

	// Add to local inventory
	if (APawn* Pawn = Cast<APawn>(Interactor))
	{
		if (UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->AddItem(ItemId, Quantity);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WorldItem: Collected %d x %s"), Quantity, *ItemId);

	// Destroy with fade/particle effect would go here
	Destroy();
}
