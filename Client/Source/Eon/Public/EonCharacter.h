// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInventoryComponent;
class UPlayerSyncComponent;
class UInteractionComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class EON_API AEonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEonCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Character")
	bool IsDead() const { return Health <= 0.0f; }

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerSyncComponent* PlayerSyncComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractionComponent* InteractionComponent;

protected:
	virtual void BeginPlay() override;

	// Input Actions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void Attack();
	void Interact();
	void ToggleInventory();

	// Enhanced Input
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* InventoryAction;

	// Character Stats
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float Health = 100.0f;

	// Camera Settings
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraBoomLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraBoomHeightOffset = 60.0f;

	// Mobile touch support
	UPROPERTY(EditDefaultsOnly, Category = "Mobile")
	bool bEnableTouchInput = true;

	UPROPERTY(EditDefaultsOnly, Category = "Mobile")
	float TouchSensitivity = 1.0f;

private:
	void SetupCamera();
	void ConfigureMobileInput();
};
