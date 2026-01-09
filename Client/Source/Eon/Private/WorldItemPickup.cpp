// Copyright 2026 Eon Project. All rights reserved.

#include "WorldItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EonCharacter.h"

AWorldItemPickup::AWorldItemPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(InteractionRadius);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = CollisionSphere;

    // Create visual mesh
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Bind overlap event
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWorldItemPickup::OnOverlapBegin);
}

void AWorldItemPickup::BeginPlay()
{
    Super::BeginPlay();

    OriginalLocation = GetActorLocation();
    BobTime = FMath::RandRange(0.0f, 2.0f * PI); // Random start phase
}

void AWorldItemPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsCollected)
    {
        return;
    }

    // Bob up and down
    BobTime += DeltaTime * BobSpeed;
    float BobOffset = FMath::Sin(BobTime) * BobHeight;
    SetActorLocation(OriginalLocation + FVector(0, 0, BobOffset));

    // Rotate
    AddActorLocalRotation(FRotator(0, RotationSpeed * DeltaTime, 0));
}

void AWorldItemPickup::InitFromWorldItem(int64 InWorldItemId, int64 InItemDefId, int32 InQuantity, const FString& InItemName)
{
    WorldItemId = InWorldItemId;
    ItemDefId = InItemDefId;
    Quantity = InQuantity;
    ItemName = InItemName;

    UE_LOG(LogTemp, Log, TEXT("Initialized world item pickup: %s (ID: %lld, DefID: %lld, Qty: %d)"),
           *ItemName, WorldItemId, ItemDefId, Quantity);
}

void AWorldItemPickup::Collect(AActor* Collector)
{
    if (bIsCollected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item already collected: %lld"), WorldItemId);
        return;
    }

    AEonCharacter* Character = Cast<AEonCharacter>(Collector);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collector is not an EonCharacter"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Collecting item: %s (ID: %lld)"), *ItemName, WorldItemId);

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("collect_world_item", WorldItemId);

    // Optimistic visual feedback - will be confirmed by server
    // The server will update is_collected, which will trigger MarkCollected
}

void AWorldItemPickup::MarkCollected()
{
    bIsCollected = true;

    // Disable visuals and collision
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("Item marked as collected: %lld"), WorldItemId);

    // Could destroy after a delay or keep for respawn logic
    // SetLifeSpan(1.0f);
}

void AWorldItemPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                      bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsCollected)
    {
        return;
    }

    // Check if the overlapping actor is a player character
    if (AEonCharacter* Character = Cast<AEonCharacter>(OtherActor))
    {
        // Auto-collect on overlap (could also require input)
        Collect(Character);
    }
}
