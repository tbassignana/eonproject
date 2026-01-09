// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// Forward declaration
class AEonCharacter;

/**
 * FInteractionInfo - Information about an interaction
 */
USTRUCT(BlueprintType)
struct FInteractionInfo
{
    GENERATED_BODY()

    // Display name for interaction prompt
    UPROPERTY(BlueprintReadWrite)
    FText DisplayName;

    // Action verb (e.g., "Open", "Pick Up", "Talk")
    UPROPERTY(BlueprintReadWrite)
    FText ActionVerb = FText::FromString(TEXT("Interact"));

    // Whether interaction is currently available
    UPROPERTY(BlueprintReadWrite)
    bool bIsAvailable = true;

    // Optional interaction duration (0 = instant)
    UPROPERTY(BlueprintReadWrite)
    float InteractionDuration = 0.0f;

    // Priority for when multiple interactables overlap
    UPROPERTY(BlueprintReadWrite)
    int32 Priority = 0;
};

// This class does not need to be modified
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IInteractableInterface - Interface for objects that can be interacted with
 *
 * Implement this interface on any actor that the player can interact with:
 * - Doors, switches, levers
 * - NPCs for dialogue
 * - Containers (chests, crates)
 * - Pickup items
 * - Readable objects (signs, notes)
 */
class EON_API IInteractableInterface
{
    GENERATED_BODY()

public:
    // Get information about this interaction
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FInteractionInfo GetInteractionInfo() const;

    // Check if interaction is currently possible
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AEonCharacter* Interactor) const;

    // Begin interaction (for instant interactions, this completes immediately)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteract(AEonCharacter* Interactor);

    // Called when interaction starts (for timed interactions)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractionStart(AEonCharacter* Interactor);

    // Called when interaction is cancelled
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractionCancelled(AEonCharacter* Interactor);

    // Called when interaction completes
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractionComplete(AEonCharacter* Interactor);

    // Called when player looks at/highlights this object
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnFocusBegin(AEonCharacter* Interactor);

    // Called when player looks away from this object
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnFocusEnd(AEonCharacter* Interactor);
};
