// Copyright 2026 Eon Project. All rights reserved.

#include "Interactables/InteractableChest.h"
#include "EonCharacter.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AInteractableChest::AInteractableChest()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    ChestRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ChestRoot"));
    RootComponent = ChestRoot;

    // Create chest base
    ChestBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBase"));
    ChestBase->SetupAttachment(ChestRoot);

    // Create chest lid (rotates to open)
    ChestLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLid"));
    ChestLid->SetupAttachment(ChestRoot);
}

void AInteractableChest::BeginPlay()
{
    Super::BeginPlay();

    InitialLidRotation = ChestLid->GetRelativeRotation();

    UE_LOG(LogTemp, Log, TEXT("InteractableChest: %s initialized with %d item types"),
           *ChestName.ToString(), Contents.Num());
}

void AInteractableChest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Animate lid
    if (!FMath::IsNearlyEqual(CurrentLidAngle, TargetLidAngle, 0.1f))
    {
        float InterpSpeed = OpenAngle / FMath::Max(OpenAnimDuration, 0.1f);
        CurrentLidAngle = FMath::FInterpConstantTo(CurrentLidAngle, TargetLidAngle, DeltaTime, InterpSpeed);
        ChestLid->SetRelativeRotation(InitialLidRotation + FRotator(CurrentLidAngle, 0.0f, 0.0f));
    }
}

void AInteractableChest::Open()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        TargetLidAngle = -OpenAngle; // Rotate backwards to open
        UE_LOG(LogTemp, Log, TEXT("InteractableChest: Opening %s"), *ChestName.ToString());
    }
}

void AInteractableChest::Close()
{
    if (bIsOpen)
    {
        bIsOpen = false;
        TargetLidAngle = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("InteractableChest: Closing %s"), *ChestName.ToString());
    }
}

void AInteractableChest::Refill(const TArray<FChestItem>& NewContents)
{
    Contents = NewContents;
    bIsEmpty = false;
    Close();
    UE_LOG(LogTemp, Log, TEXT("InteractableChest: %s refilled with %d item types"),
           *ChestName.ToString(), Contents.Num());
}

void AInteractableChest::GiveItemsToPlayer(AEonCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("InteractableChest: Player has no inventory component"));
        return;
    }

    // Give all items
    for (const FChestItem& Item : Contents)
    {
        if (Item.ItemDefId > 0 && Item.Quantity > 0)
        {
            Inventory->AddItem(Item.ItemDefId, Item.Quantity);
            UE_LOG(LogTemp, Log, TEXT("InteractableChest: Gave player item %llu x%d"),
                   Item.ItemDefId, Item.Quantity);
        }
    }

    bIsEmpty = true;

    if (bSingleUse)
    {
        Contents.Empty();
    }
    else
    {
        StartRespawnTimer();
    }
}

void AInteractableChest::StartRespawnTimer()
{
    // Store original contents before clearing
    TArray<FChestItem> OriginalContents = Contents;

    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        [this, OriginalContents]()
        {
            Refill(OriginalContents);
        },
        RespawnTime,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("InteractableChest: %s will respawn in %.1f seconds"),
           *ChestName.ToString(), RespawnTime);
}

FInteractionInfo AInteractableChest::GetInteractionInfo_Implementation() const
{
    FInteractionInfo Info;
    Info.DisplayName = ChestName;

    if (bIsEmpty)
    {
        Info.ActionVerb = FText::FromString(TEXT("Empty"));
        Info.bIsAvailable = false;
    }
    else
    {
        Info.ActionVerb = FText::FromString(TEXT("Open"));
        Info.bIsAvailable = true;
    }

    Info.InteractionDuration = 0.0f;
    Info.Priority = 5; // Higher priority than doors

    return Info;
}

bool AInteractableChest::CanInteract_Implementation(AEonCharacter* Interactor) const
{
    return Interactor != nullptr && !bIsEmpty;
}

void AInteractableChest::OnInteract_Implementation(AEonCharacter* Interactor)
{
    if (!Interactor || bIsEmpty)
    {
        return;
    }

    Open();
    GiveItemsToPlayer(Interactor);
}

void AInteractableChest::OnInteractionStart_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableChest::OnInteractionCancelled_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableChest::OnInteractionComplete_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableChest::OnFocusBegin_Implementation(AEonCharacter* Interactor)
{
    UE_LOG(LogTemp, Verbose, TEXT("InteractableChest: %s focused"), *ChestName.ToString());
}

void AInteractableChest::OnFocusEnd_Implementation(AEonCharacter* Interactor)
{
    UE_LOG(LogTemp, Verbose, TEXT("InteractableChest: %s unfocused"), *ChestName.ToString());
}
