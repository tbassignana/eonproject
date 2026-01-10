// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestItemSpawner.generated.h"

class AWorldItemPickup;
class UBillboardComponent;

USTRUCT(BlueprintType)
struct FTestItemDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ItemColor = FLinearColor::White;
};

UCLASS()
class EON_API ATestItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATestItemSpawner();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "TestItems")
	void SpawnAllItems();

	UFUNCTION(BlueprintCallable, Category = "TestItems")
	AWorldItemPickup* SpawnItem(const FTestItemDefinition& ItemDef, FVector Location);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* EditorSprite;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestItems")
	TArray<FTestItemDefinition> ItemsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestItems")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestItems")
	float ItemSpacing = 150.0f;

private:
	UPROPERTY()
	TArray<AWorldItemPickup*> SpawnedItems;
};
