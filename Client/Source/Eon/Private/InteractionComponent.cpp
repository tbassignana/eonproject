// Copyright 2026 Eon Project. All rights reserved.

#include "InteractionComponent.h"
#include "EonCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("InteractionComponent: BeginPlay"));
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update interaction progress if currently interacting
    if (bIsInteracting)
    {
        UpdateInteractionProgress(DeltaTime);
    }
    else
    {
        // Scan for interactables at configured rate
        TimeSinceLastScan += DeltaTime;
        if (TimeSinceLastScan >= ScanRate)
        {
            TimeSinceLastScan = 0.0f;
            ScanForInteractables();
        }
    }
}

void UInteractionComponent::ScanForInteractables()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get camera/view location and direction
    FVector StartLocation;
    FRotator ViewRotation;

    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (OwnerPawn && OwnerPawn->GetController())
    {
        OwnerPawn->GetController()->GetPlayerViewPoint(StartLocation, ViewRotation);
    }
    else
    {
        StartLocation = Owner->GetActorLocation();
        ViewRotation = Owner->GetActorRotation();
    }

    FVector EndLocation = StartLocation + ViewRotation.Vector() * InteractionRange;

    // Sphere trace for interactables
    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(Owner);

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        StartLocation,
        EndLocation,
        InteractionRadius,
        UEngineTypes::ConvertToTraceType(InteractionChannel),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResults,
        true
    );

    // Find best interactable from hits
    AActor* BestInteractable = nullptr;
    int32 BestPriority = INT_MIN;
    float BestDistance = FLT_MAX;

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor)
            {
                continue;
            }

            // Check if actor implements interactable interface
            if (!HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
            {
                continue;
            }

            // Get interface and check if interaction is possible
            IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitActor);
            if (!Interactable)
            {
                // Try execute interface call for non-native implementations
                bool bCanInteract = false;
                if (HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
                {
                    bCanInteract = IInteractableInterface::Execute_CanInteract(HitActor, Cast<AEonCharacter>(Owner));
                }
                if (!bCanInteract)
                {
                    continue;
                }
            }
            else if (!Interactable->Execute_CanInteract(HitActor, Cast<AEonCharacter>(Owner)))
            {
                continue;
            }

            // Get interaction info for priority
            FInteractionInfo Info = IInteractableInterface::Execute_GetInteractionInfo(HitActor);

            // Prefer higher priority, then closer distance
            float Distance = Hit.Distance;
            if (Info.Priority > BestPriority ||
                (Info.Priority == BestPriority && Distance < BestDistance))
            {
                BestInteractable = HitActor;
                BestPriority = Info.Priority;
                BestDistance = Distance;
            }
        }
    }

    // Update current interactable
    SetCurrentInteractable(BestInteractable);
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewInteractable)
{
    if (CurrentInteractable == NewInteractable)
    {
        return;
    }

    AActor* Owner = GetOwner();
    AEonCharacter* Character = Cast<AEonCharacter>(Owner);

    // Notify old interactable of focus loss
    if (CurrentInteractable)
    {
        if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_OnFocusEnd(CurrentInteractable, Character);
        }
        OnInteractableLost.Broadcast();
    }

    CurrentInteractable = NewInteractable;

    // Notify new interactable of focus
    if (CurrentInteractable)
    {
        if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_OnFocusBegin(CurrentInteractable, Character);
            CachedInteractionInfo = IInteractableInterface::Execute_GetInteractionInfo(CurrentInteractable);
        }
        OnInteractableFound.Broadcast(CurrentInteractable);

        UE_LOG(LogTemp, Verbose, TEXT("InteractionComponent: Found interactable %s"),
               *CurrentInteractable->GetName());
    }
}

bool UInteractionComponent::TryInteract()
{
    if (!CanInteract())
    {
        return false;
    }

    AActor* Owner = GetOwner();
    AEonCharacter* Character = Cast<AEonCharacter>(Owner);

    // Get interaction duration
    CachedInteractionInfo = IInteractableInterface::Execute_GetInteractionInfo(CurrentInteractable);
    CurrentInteractionDuration = CachedInteractionInfo.InteractionDuration;

    if (CurrentInteractionDuration <= 0.0f)
    {
        // Instant interaction
        IInteractableInterface::Execute_OnInteract(CurrentInteractable, Character);
        OnInteractionCompleted.Broadcast(CurrentInteractable);

        UE_LOG(LogTemp, Log, TEXT("InteractionComponent: Instant interaction with %s"),
               *CurrentInteractable->GetName());
    }
    else
    {
        // Timed interaction
        bIsInteracting = true;
        InteractionProgress = 0.0f;
        IInteractableInterface::Execute_OnInteractionStart(CurrentInteractable, Character);
        OnInteractionStarted.Broadcast(CurrentInteractable, CurrentInteractionDuration);

        UE_LOG(LogTemp, Log, TEXT("InteractionComponent: Started timed interaction (%.1fs) with %s"),
               CurrentInteractionDuration, *CurrentInteractable->GetName());
    }

    return true;
}

void UInteractionComponent::CancelInteraction()
{
    if (!bIsInteracting)
    {
        return;
    }

    AActor* Owner = GetOwner();
    AEonCharacter* Character = Cast<AEonCharacter>(Owner);

    bIsInteracting = false;
    InteractionProgress = 0.0f;

    if (CurrentInteractable && CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
        IInteractableInterface::Execute_OnInteractionCancelled(CurrentInteractable, Character);
    }

    OnInteractionCancelled.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("InteractionComponent: Cancelled interaction"));
}

void UInteractionComponent::UpdateInteractionProgress(float DeltaTime)
{
    if (!bIsInteracting || CurrentInteractionDuration <= 0.0f)
    {
        return;
    }

    InteractionProgress += DeltaTime / CurrentInteractionDuration;
    OnInteractionProgress.Broadcast(InteractionProgress);

    if (InteractionProgress >= 1.0f)
    {
        // Interaction complete
        bIsInteracting = false;
        InteractionProgress = 1.0f;

        AActor* Owner = GetOwner();
        AEonCharacter* Character = Cast<AEonCharacter>(Owner);

        if (CurrentInteractable && CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_OnInteractionComplete(CurrentInteractable, Character);
        }

        OnInteractionCompleted.Broadcast(CurrentInteractable);

        UE_LOG(LogTemp, Log, TEXT("InteractionComponent: Completed timed interaction with %s"),
               CurrentInteractable ? *CurrentInteractable->GetName() : TEXT("None"));
    }
}

bool UInteractionComponent::CanInteract() const
{
    if (!CurrentInteractable || bIsInteracting)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    AEonCharacter* Character = Cast<AEonCharacter>(Owner);

    if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
        return IInteractableInterface::Execute_CanInteract(CurrentInteractable, Character);
    }

    return false;
}

FInteractionInfo UInteractionComponent::GetCurrentInteractionInfo() const
{
    if (CurrentInteractable && CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
        return IInteractableInterface::Execute_GetInteractionInfo(CurrentInteractable);
    }
    return FInteractionInfo();
}
