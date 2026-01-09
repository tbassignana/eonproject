// Copyright 2026 Eon Project. All rights reserved.

#include "EonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AEonCharacter::AEonCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Initialize health
    CurrentHealth = MaxHealth;

    // Network sync
    LastSyncedPosition = FVector::ZeroVector;
    LastSyncedRotation = FRotator::ZeroRotator;
}

void AEonCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Initialize sync state
    LastSyncedPosition = GetActorLocation();
    LastSyncedRotation = GetActorRotation();
}

void AEonCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Network position sync
    TimeSinceLastSync += DeltaTime;
    if (TimeSinceLastSync >= PositionSyncInterval)
    {
        const FVector CurrentPos = GetActorLocation();
        const FRotator CurrentRot = GetActorRotation();

        // Only send if position or rotation changed significantly
        const float PosDelta = FVector::DistSquared(CurrentPos, LastSyncedPosition);
        const float RotDelta = FMath::Abs(CurrentRot.Yaw - LastSyncedRotation.Yaw);

        if (PosDelta > 1.0f || RotDelta > 1.0f)
        {
            SendPositionUpdate();
            LastSyncedPosition = CurrentPos;
            LastSyncedRotation = CurrentRot;
        }

        TimeSinceLastSync = 0.0f;
    }
}

void AEonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEonCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEonCharacter::Look);

        // Attacking
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AEonCharacter::StartAttack);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AEonCharacter::StopAttack);
    }
}

void AEonCharacter::Move(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward and right vectors
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AEonCharacter::Look(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AEonCharacter::StartAttack(const FInputActionValue& Value)
{
    if (!bIsAttacking)
    {
        bIsAttacking = true;
        PerformMeleeAttack();
        // TODO: Send attack state to SpaceTimeDB
    }
}

void AEonCharacter::StopAttack(const FInputActionValue& Value)
{
    bIsAttacking = false;
    // TODO: Send attack state to SpaceTimeDB
}

void AEonCharacter::PerformMeleeAttack()
{
    // Simple sphere trace for melee attack
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * AttackRange;

    TArray<FHitResult> HitResults;
    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(50.0f);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        End,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        CollisionShape
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AEonCharacter* OtherCharacter = Cast<AEonCharacter>(Hit.GetActor()))
            {
                // Don't damage self
                if (OtherCharacter != this)
                {
                    // TODO: Send damage via SpaceTimeDB instead of direct call
                    // For now, apply damage locally for testing
                    OtherCharacter->ApplyDamage(AttackDamage);
                }
            }
        }
    }

    // Debug visualization
#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.5f);
#endif
}

void AEonCharacter::ApplyDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("Character took %.1f damage. Health: %.1f/%.1f"),
           DamageAmount, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        // Handle death
        UE_LOG(LogTemp, Warning, TEXT("Character died!"));
        // TODO: Respawn logic, notify SpaceTimeDB
    }
}

void AEonCharacter::Heal(float HealAmount)
{
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
    UE_LOG(LogTemp, Log, TEXT("Character healed %.1f. Health: %.1f/%.1f"),
           HealAmount, CurrentHealth, MaxHealth);
}

void AEonCharacter::SendPositionUpdate()
{
    const FVector Pos = GetActorLocation();
    const FRotator Rot = GetActorRotation();

    // TODO: Implement SpaceTimeDB network call
    // NetworkManager->CallReducer("update_player_transform",
    //     Pos.X, Pos.Y, Pos.Z,
    //     Rot.Pitch, Rot.Yaw, Rot.Roll);

    UE_LOG(LogTemp, Verbose, TEXT("Position sync: (%.1f, %.1f, %.1f) Rot: (%.1f, %.1f, %.1f)"),
           Pos.X, Pos.Y, Pos.Z, Rot.Pitch, Rot.Yaw, Rot.Roll);
}
