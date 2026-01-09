// Copyright 2026 Eon Project. All rights reserved.

#include "Interactables/InteractableDoor.h"
#include "EonCharacter.h"
#include "InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

AInteractableDoor::AInteractableDoor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
    RootComponent = DoorRoot;

    // Create door frame (static)
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(DoorRoot);

    // Create door panel (rotates)
    DoorPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanel"));
    DoorPanel->SetupAttachment(DoorRoot);
}

void AInteractableDoor::BeginPlay()
{
    Super::BeginPlay();

    InitialRotation = DoorPanel->GetRelativeRotation();

    if (bStartsOpen)
    {
        bIsOpen = true;
        CurrentAngle = OpenAngle;
        TargetAngle = OpenAngle;
        DoorPanel->SetRelativeRotation(InitialRotation + FRotator(0.0f, CurrentAngle, 0.0f));
    }

    UE_LOG(LogTemp, Log, TEXT("InteractableDoor: %s initialized (Open: %s, Locked: %s)"),
           *DoorName.ToString(), bIsOpen ? TEXT("Yes") : TEXT("No"), bIsLocked ? TEXT("Yes") : TEXT("No"));
}

void AInteractableDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Smoothly interpolate door angle
    if (!FMath::IsNearlyEqual(CurrentAngle, TargetAngle, 0.1f))
    {
        CurrentAngle = FMath::FInterpTo(CurrentAngle, TargetAngle, DeltaTime, OpenSpeed);
        DoorPanel->SetRelativeRotation(InitialRotation + FRotator(0.0f, CurrentAngle, 0.0f));
    }
}

void AInteractableDoor::Open()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        TargetAngle = OpenAngle;
        UE_LOG(LogTemp, Log, TEXT("InteractableDoor: Opening %s"), *DoorName.ToString());
    }
}

void AInteractableDoor::Close()
{
    if (bIsOpen)
    {
        bIsOpen = false;
        TargetAngle = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("InteractableDoor: Closing %s"), *DoorName.ToString());
    }
}

void AInteractableDoor::Toggle()
{
    if (bIsOpen)
    {
        Close();
    }
    else
    {
        Open();
    }
}

void AInteractableDoor::Unlock()
{
    bIsLocked = false;
    UE_LOG(LogTemp, Log, TEXT("InteractableDoor: %s unlocked"), *DoorName.ToString());
}

FInteractionInfo AInteractableDoor::GetInteractionInfo_Implementation() const
{
    FInteractionInfo Info;
    Info.DisplayName = DoorName;

    if (bIsLocked)
    {
        Info.ActionVerb = FText::FromString(TEXT("Unlock"));
        Info.bIsAvailable = false; // Will be checked dynamically in CanInteract
    }
    else if (bIsOpen)
    {
        Info.ActionVerb = FText::FromString(TEXT("Close"));
        Info.bIsAvailable = true;
    }
    else
    {
        Info.ActionVerb = FText::FromString(TEXT("Open"));
        Info.bIsAvailable = true;
    }

    Info.InteractionDuration = 0.0f; // Instant
    Info.Priority = 0;

    return Info;
}

bool AInteractableDoor::CanInteract_Implementation(AEonCharacter* Interactor) const
{
    if (!Interactor)
    {
        return false;
    }

    // If locked, check if player has the key
    if (bIsLocked && RequiredKeyItemId > 0)
    {
        UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
        if (!Inventory || !Inventory->HasItem(RequiredKeyItemId))
        {
            return false;
        }
    }
    else if (bIsLocked)
    {
        return false; // Locked with no key defined = permanently locked
    }

    return true;
}

void AInteractableDoor::OnInteract_Implementation(AEonCharacter* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    // If locked with key, consume key and unlock
    if (bIsLocked && RequiredKeyItemId > 0)
    {
        UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
        if (Inventory && Inventory->HasItem(RequiredKeyItemId))
        {
            Inventory->RemoveItem(RequiredKeyItemId, 1);
            Unlock();
            UE_LOG(LogTemp, Log, TEXT("InteractableDoor: %s used key to unlock"), *Interactor->GetName());
        }
        return;
    }

    // Toggle door state
    Toggle();
}

void AInteractableDoor::OnInteractionStart_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableDoor::OnInteractionCancelled_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableDoor::OnInteractionComplete_Implementation(AEonCharacter* Interactor)
{
    // Not used for instant interactions
}

void AInteractableDoor::OnFocusBegin_Implementation(AEonCharacter* Interactor)
{
    // Could add highlight effect here
    UE_LOG(LogTemp, Verbose, TEXT("InteractableDoor: %s focused"), *DoorName.ToString());
}

void AInteractableDoor::OnFocusEnd_Implementation(AEonCharacter* Interactor)
{
    // Could remove highlight effect here
    UE_LOG(LogTemp, Verbose, TEXT("InteractableDoor: %s unfocused"), *DoorName.ToString());
}
