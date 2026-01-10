// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "WorldItemPickup.generated.h"

UCLASS()
class EON_API AWorldItemPickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AWorldItemPickup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// IInteractableInterface
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual void OnInteract(AActor* Interactor) override;
	virtual void OnBeginFocus(AActor* Interactor) override;
	virtual void OnEndFocus(AActor* Interactor) override;
	virtual FString GetInteractionPrompt() const override;
	virtual EInteractableType GetInteractableType() const override { return EInteractableType::Pickup; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Initialize(int64 InWorldItemId, const FString& InItemId, int32 InQuantity);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	FString GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	int32 GetQuantity() const { return Quantity; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bBobUpAndDown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bRotate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float BobSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float BobHeight = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float RotationSpeed = 90.0f;

private:
	int64 WorldItemId = 0; // SpaceTimeDB world_item_id
	FVector InitialLocation;
	float BobTime = 0.0f;
	bool bIsCollected = false;

	void UpdateAnimation(float DeltaTime);
	void Collect(AActor* Interactor);
};
