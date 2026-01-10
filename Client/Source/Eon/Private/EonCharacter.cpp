// Copyright 2026 tbassignana. MIT License.

#include "EonCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "InventoryComponent.h"
#include "PlayerSyncComponent.h"
#include "InteractionComponent.h"

AEonCharacter::AEonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Try to load UE5 Mannequin mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannequinMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	if (MannequinMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MannequinMesh.Object);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}
	else
	{
		// Fallback: Make capsule visible for debugging
		GetCapsuleComponent()->SetVisibility(true);
		GetCapsuleComponent()->SetHiddenInGame(false);
	}

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
}

void AEonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &AEonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AEonCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AEonCharacter::LookUp);

	// Action bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AEonCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AEonCharacter::StopJump);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AEonCharacter::Attack);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AEonCharacter::Interact);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AEonCharacter::ToggleInventory);
}

void AEonCharacter::MoveForward(float Value)
{
	if (IsDead() || Value == 0.0f) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AEonCharacter::MoveRight(float Value)
{
	if (IsDead() || Value == 0.0f) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

void AEonCharacter::Turn(float Value)
{
	if (Value != 0.0f)
	{
		AddControllerYawInput(Value);
	}
}

void AEonCharacter::LookUp(float Value)
{
	if (Value != 0.0f)
	{
		AddControllerPitchInput(Value);
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
	UE_LOG(LogTemp, Log, TEXT("EonCharacter: Toggle inventory"));
}

void AEonCharacter::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("EonCharacter: Player died"));
	}
}

void AEonCharacter::ApplyDamage(float DamageAmount)
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
