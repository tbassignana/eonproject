// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "EonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * AEonCharacter - Main player character for the Eon game
 *
 * Third-person character with:
 * - Movement and jumping
 * - Combat (melee attacks)
 * - Health system
 * - Network state replication via SpaceTimeDB
 */
UCLASS(config=Game)
class EON_API AEonCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEonCharacter();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AttackAction;

    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
    float CurrentHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
    float AttackRange = 150.0f;

    // Network sync timing
    UPROPERTY(EditAnywhere, Category = Network)
    float PositionSyncInterval = 0.05f; // 20 Hz

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAttack(const FInputActionValue& Value);
    void StopAttack(const FInputActionValue& Value);

    // Combat
    UFUNCTION(BlueprintCallable, Category = Combat)
    void PerformMeleeAttack();

    UFUNCTION(BlueprintCallable, Category = Combat)
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = Combat)
    void Heal(float HealAmount);

    // Network sync helpers
    void SendPositionUpdate();

private:
    float TimeSinceLastSync = 0.0f;
    FVector LastSyncedPosition;
    FRotator LastSyncedRotation;

public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
