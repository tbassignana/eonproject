// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableInterface.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStarted, AActor*, Interactable, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionProgress, float, Progress);

/**
 * UInteractionComponent - Component that handles player interaction with world objects
 *
 * Attach this to the player character to enable interaction with IInteractableInterface actors.
 * Performs sphere traces to find interactables and manages the interaction state machine.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionComponent();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float ScanRate = 0.1f; // How often to scan for interactables

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractableFound OnInteractableFound;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractableLost OnInteractableLost;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionStarted OnInteractionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionCompleted OnInteractionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionCancelled OnInteractionCancelled;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionProgress OnInteractionProgress;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    AActor* CurrentInteractable = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bIsInteracting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    float InteractionProgress = 0.0f;

    // Attempt to interact with the current interactable
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TryInteract();

    // Cancel current interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CancelInteraction();

    // Check if we can interact with something
    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool CanInteract() const;

    // Get info about current interactable
    UFUNCTION(BlueprintPure, Category = "Interaction")
    FInteractionInfo GetCurrentInteractionInfo() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Scan for interactables
    void ScanForInteractables();

    // Set new current interactable
    void SetCurrentInteractable(AActor* NewInteractable);

    // Update timed interaction progress
    void UpdateInteractionProgress(float DeltaTime);

    // Timer handle for scan rate
    float TimeSinceLastScan = 0.0f;

    // Cached interaction info
    UPROPERTY()
    FInteractionInfo CachedInteractionInfo;

    // Duration for current interaction
    float CurrentInteractionDuration = 0.0f;
};
