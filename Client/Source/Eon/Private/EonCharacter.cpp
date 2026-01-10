// Copyright 2026 tbassignana. MIT License.

#include "EonCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InventoryComponent.h"
#include "PlayerSyncComponent.h"
#include "InteractionComponent.h"

AEonCharacter::AEonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraBoomLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, CameraBoomHeightOffset);

	// Setup follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Character doesn't rotate with controller, camera does
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	// Create gameplay components
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	PlayerSyncComponent = CreateDefaultSubobject<UPlayerSyncComponent>(TEXT("PlayerSyncComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	Health = MaxHealth;
}

void AEonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Setup enhanced input
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	ConfigureMobileInput();
}

void AEonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEonCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEonCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AEonCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEonCharacter::StopJump);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AEonCharacter::Attack);
		}
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AEonCharacter::Interact);
		}
		if (InventoryAction)
		{
			EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &AEonCharacter::ToggleInventory);
		}
	}
}

void AEonCharacter::Move(const FInputActionValue& Value)
{
	if (IsDead()) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AEonCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

#if PLATFORM_IOS
	LookAxisVector *= TouchSensitivity;
#endif

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AEonCharacter::StartJump()
{
	if (!IsDead())
	{
		Jump();
	}
}

void AEonCharacter::StopJump()
{
	StopJumping();
}

void AEonCharacter::Attack()
{
	if (IsDead()) return;

	// Basic attack implementation
	// This would trigger combat animations and damage calculations
	UE_LOG(LogTemp, Log, TEXT("EonCharacter: Attack triggered"));
}

void AEonCharacter::Interact()
{
	if (IsDead()) return;

	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
}

void AEonCharacter::ToggleInventory()
{
	// This will be handled by the UI system
	UE_LOG(LogTemp, Log, TEXT("EonCharacter: Toggle inventory"));
}

void AEonCharacter::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		// Handle death
		UE_LOG(LogTemp, Warning, TEXT("EonCharacter: Player died"));
	}
}

void AEonCharacter::TakeDamage(float DamageAmount)
{
	SetHealth(Health - DamageAmount);
}

void AEonCharacter::Heal(float HealAmount)
{
	SetHealth(Health + HealAmount);
}

void AEonCharacter::SetupCamera()
{
	CameraBoom->TargetArmLength = CameraBoomLength;
	CameraBoom->SocketOffset.Z = CameraBoomHeightOffset;
}

void AEonCharacter::ConfigureMobileInput()
{
#if PLATFORM_IOS
	if (bEnableTouchInput)
	{
		// Enable touch interface for iOS
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			PC->bShowMouseCursor = false;
			PC->SetVirtualJoystickVisibility(true);
		}
	}
#endif
}
