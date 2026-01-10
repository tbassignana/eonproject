// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableChest.generated.h"

USTRUCT(BlueprintType)
struct FChestLoot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropChance = 1.0f; // 1.0 = 100%
};

UCLASS()
class EON_API AInteractableChest : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractableChest();

	virtual void BeginPlay() override;

	// IInteractableInterface
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual void OnInteract(AActor* Interactor) override;
	virtual void OnBeginFocus(AActor* Interactor) override;
	virtual void OnEndFocus(AActor* Interactor) override;
	virtual FString GetInteractionPrompt() const override;
	virtual EInteractableType GetInteractableType() const override { return EInteractableType::Chest; }
	virtual FString GetRequiredItem() const override { return RequiredKeyItem; }

	UFUNCTION(BlueprintCallable, Category = "Chest")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintCallable, Category = "Chest")
	void SetLocked(bool bLocked, const FString& KeyItem = TEXT(""));

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	TArray<FChestLoot> LootTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	FString RequiredKeyItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	bool bDestroyKeyOnUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	FString ChestId; // For SpaceTimeDB persistence

private:
	bool bIsOpen = false;
	bool bHasBeenLooted = false;

	void OpenChest(AActor* Interactor);
	void SpawnLoot(AActor* Interactor);
	void SyncStateToServer();
};
