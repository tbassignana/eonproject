// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UENUM(BlueprintType)
enum class EInteractableType : uint8
{
	Generic,
	Chest,
	Door,
	Pickup,
	NPC,
	Switch
};

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class EON_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// Check if interaction is possible
	virtual bool CanInteract(AActor* Interactor) const { return true; }

	// Called when interaction begins
	virtual void OnInteract(AActor* Interactor) = 0;

	// Called when player looks at interactable
	virtual void OnBeginFocus(AActor* Interactor) {}

	// Called when player looks away
	virtual void OnEndFocus(AActor* Interactor) {}

	// Get the interaction prompt text
	virtual FString GetInteractionPrompt() const { return TEXT("Interact"); }

	// Get the type of interactable
	virtual EInteractableType GetInteractableType() const { return EInteractableType::Generic; }

	// Get any required item to interact (empty = no requirement)
	virtual FString GetRequiredItem() const { return FString(); }
};
