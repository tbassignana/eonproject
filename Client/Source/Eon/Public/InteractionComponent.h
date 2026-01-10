// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractableInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionComplete, AActor*, Interactable);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool HasInteractableInRange() const { return CurrentInteractable != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FString GetInteractionPrompt() const;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableFound OnInteractableFound;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableLost OnInteractableLost;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionComplete OnInteractionComplete;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float ScanInterval = 0.1f; // How often to scan for interactables

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_GameTraceChannel1;

private:
	UPROPERTY()
	AActor* CurrentInteractable = nullptr;

	float TimeSinceLastScan = 0.0f;

	void ScanForInteractables();
	AActor* FindBestInteractable() const;
};
