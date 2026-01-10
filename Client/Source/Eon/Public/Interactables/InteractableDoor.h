// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "InteractableDoor.generated.h"

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Closed,
	Opening,
	Open,
	Closing
};

UCLASS()
class EON_API AInteractableDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractableDoor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// IInteractableInterface
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual void OnInteract(AActor* Interactor) override;
	virtual void OnBeginFocus(AActor* Interactor) override;
	virtual void OnEndFocus(AActor* Interactor) override;
	virtual FString GetInteractionPrompt() const override;
	virtual EInteractableType GetInteractableType() const override { return EInteractableType::Door; }
	virtual FString GetRequiredItem() const override { return RequiredKeyItem; }

	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsOpen() const { return DoorState == EDoorState::Open; }

	UFUNCTION(BlueprintCallable, Category = "Door")
	void SetLocked(bool bLocked, const FString& KeyItem = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Open();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Close();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Toggle();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FString RequiredKeyItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bDestroyKeyOnUse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bAutoClose = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bAutoClose"))
	float AutoCloseDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FString DoorId; // For SpaceTimeDB persistence

private:
	EDoorState DoorState = EDoorState::Closed;
	float CurrentAngle = 0.0f;
	float TargetAngle = 0.0f;
	float AutoCloseTimer = 0.0f;
	FRotator InitialRotation;

	void UpdateDoorRotation(float DeltaTime);
	void SyncStateToServer();
};
