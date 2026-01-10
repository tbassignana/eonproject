// Copyright 2026 tbassignana. MIT License.

#include "TestItemSpawner.h"
#include "WorldItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ATestItemSpawner::ATestItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create a scene root so the actor can be moved in editor
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

#if WITH_EDITORONLY_DATA
	// Add editor billboard for visibility
	EditorSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorSprite"));
	if (EditorSprite)
	{
		EditorSprite->SetupAttachment(RootComponent);
		static ConstructorHelpers::FObjectFinder<UTexture2D> IconTexture(TEXT("/Engine/EditorResources/S_Note"));
		if (IconTexture.Succeeded())
		{
			EditorSprite->SetSprite(IconTexture.Object);
		}
		EditorSprite->SetRelativeScale3D(FVector(0.5f));
		EditorSprite->bIsScreenSizeScaled = true;
	}
#endif

	// Add default test items
	FTestItemDefinition HealthPotion;
	HealthPotion.ItemId = TEXT("health_potion");
	HealthPotion.DisplayName = TEXT("Health Potion");
	HealthPotion.Quantity = 1;
	HealthPotion.ItemColor = FLinearColor::Red;

	FTestItemDefinition ManaPotion;
	ManaPotion.ItemId = TEXT("mana_potion");
	ManaPotion.DisplayName = TEXT("Mana Potion");
	ManaPotion.Quantity = 1;
	ManaPotion.SpawnOffset = FVector(150.0f, 0, 0);
	ManaPotion.ItemColor = FLinearColor::Blue;

	FTestItemDefinition GoldCoin;
	GoldCoin.ItemId = TEXT("gold_coin");
	GoldCoin.DisplayName = TEXT("Gold Coin");
	GoldCoin.Quantity = 10;
	GoldCoin.SpawnOffset = FVector(300.0f, 0, 0);
	GoldCoin.ItemColor = FLinearColor(1.0f, 0.84f, 0.0f); // Gold

	FTestItemDefinition IronSword;
	IronSword.ItemId = TEXT("iron_sword");
	IronSword.DisplayName = TEXT("Iron Sword");
	IronSword.Quantity = 1;
	IronSword.SpawnOffset = FVector(0, 150.0f, 0);
	IronSword.ItemColor = FLinearColor(0.5f, 0.5f, 0.6f); // Steel gray

	FTestItemDefinition WoodenShield;
	WoodenShield.ItemId = TEXT("wooden_shield");
	WoodenShield.DisplayName = TEXT("Wooden Shield");
	WoodenShield.Quantity = 1;
	WoodenShield.SpawnOffset = FVector(150.0f, 150.0f, 0);
	WoodenShield.ItemColor = FLinearColor(0.6f, 0.4f, 0.2f); // Brown

	ItemsToSpawn.Add(HealthPotion);
	ItemsToSpawn.Add(ManaPotion);
	ItemsToSpawn.Add(GoldCoin);
	ItemsToSpawn.Add(IronSword);
	ItemsToSpawn.Add(WoodenShield);
}

void ATestItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnAllItems();
	}
}

void ATestItemSpawner::SpawnAllItems()
{
	FVector BaseLocation = GetActorLocation();

	for (const FTestItemDefinition& ItemDef : ItemsToSpawn)
	{
		FVector SpawnLocation = BaseLocation + ItemDef.SpawnOffset;
		SpawnLocation.Z += 50.0f; // Lift off ground

		AWorldItemPickup* Pickup = SpawnItem(ItemDef, SpawnLocation);
		if (Pickup)
		{
			SpawnedItems.Add(Pickup);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TestItemSpawner: Spawned %d test items"), SpawnedItems.Num());
}

AWorldItemPickup* ATestItemSpawner::SpawnItem(const FTestItemDefinition& ItemDef, FVector Location)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWorldItemPickup* Pickup = World->SpawnActor<AWorldItemPickup>(
		AWorldItemPickup::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Pickup)
	{
		Pickup->Initialize(0, ItemDef.ItemId, ItemDef.Quantity, ItemDef.DisplayName);

		// Set color via dynamic material
		if (UStaticMeshComponent* Mesh = Pickup->FindComponentByClass<UStaticMeshComponent>())
		{
			UMaterialInstanceDynamic* DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("Color"), ItemDef.ItemColor);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("TestItemSpawner: Spawned %s at %s"),
			*ItemDef.DisplayName, *Location.ToString());
	}

	return Pickup;
}
