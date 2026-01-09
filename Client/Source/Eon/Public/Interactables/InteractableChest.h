// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableChest.generated.h"

/**
 * FChestItem - An item contained in a chest
 */
USTRUCT(BlueprintType)
struct FChestItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint64 ItemDefId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;
};

/**
 * AInteractableChest - A container that gives items to the player
 *
 * Features:
 * - Contains configurable items
 * - Opens with animation on interaction
 * - Items transfer to player inventory
 * - Can be single-use or respawning
 */
UCLASS()
class EON_API AInteractableChest : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AInteractableChest();

    // Chest configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    FText ChestName = FText::FromString(TEXT("Chest"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    TArray<FChestItem> Contents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    bool bSingleUse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    float RespawnTime = 60.0f; // If not single-use

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    float OpenAnimDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
    float OpenAngle = 90.0f;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
    bool bIsOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest")
    bool bIsEmpty = false;

    // Manual control
    UFUNCTION(BlueprintCallable, Category = "Chest")
    void Open();

    UFUNCTION(BlueprintCallable, Category = "Chest")
    void Close();

    UFUNCTION(BlueprintCallable, Category = "Chest")
    void Refill(const TArray<FChestItem>& NewContents);

    // IInteractableInterface implementation
    virtual FInteractionInfo GetInteractionInfo_Implementation() const override;
    virtual bool CanInteract_Implementation(AEonCharacter* Interactor) const override;
    virtual void OnInteract_Implementation(AEonCharacter* Interactor) override;
    virtual void OnInteractionStart_Implementation(AEonCharacter* Interactor) override;
    virtual void OnInteractionCancelled_Implementation(AEonCharacter* Interactor) override;
    virtual void OnInteractionComplete_Implementation(AEonCharacter* Interactor) override;
    virtual void OnFocusBegin_Implementation(AEonCharacter* Interactor) override;
    virtual void OnFocusEnd_Implementation(AEonCharacter* Interactor) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* ChestRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ChestBase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ChestLid;

private:
    void GiveItemsToPlayer(AEonCharacter* Player);
    void StartRespawnTimer();

    float CurrentLidAngle = 0.0f;
    float TargetLidAngle = 0.0f;
    FRotator InitialLidRotation;
    FTimerHandle RespawnTimerHandle;
};
