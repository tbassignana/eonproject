// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableDoor.generated.h"

/**
 * AInteractableDoor - A door that can be opened/closed by player interaction
 *
 * Features:
 * - Opens/closes on interaction
 * - Configurable open angle and speed
 * - Optional locked state requiring key item
 * - Sound effects (placeholders)
 */
UCLASS()
class EON_API AInteractableDoor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AInteractableDoor();

    // Door configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bStartsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    uint64 RequiredKeyItemId = 0; // Item definition ID required to unlock

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    FText DoorName = FText::FromString(TEXT("Door"));

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    bool bIsOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    float CurrentAngle = 0.0f;

    // Manual control
    UFUNCTION(BlueprintCallable, Category = "Door")
    void Open();

    UFUNCTION(BlueprintCallable, Category = "Door")
    void Close();

    UFUNCTION(BlueprintCallable, Category = "Door")
    void Toggle();

    UFUNCTION(BlueprintCallable, Category = "Door")
    void Unlock();

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
    USceneComponent* DoorRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DoorPanel;

private:
    float TargetAngle = 0.0f;
    FRotator InitialRotation;
};
