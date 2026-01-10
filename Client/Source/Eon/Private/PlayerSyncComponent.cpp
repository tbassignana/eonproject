// Copyright 2026 tbassignana. MIT License.

#include "PlayerSyncComponent.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Engine/World.h"

UPlayerSyncComponent::UPlayerSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	// Subscribe to player updates from SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			// Bind to player data updates (will receive data once connected)
			Manager->OnPlayerDataReceived.AddDynamic(this, &UPlayerSyncComponent::OnPlayerDataReceived);
		}
	}
}

void UPlayerSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Interpolate other player positions for smooth movement
	for (const FOtherPlayer& Player : OtherPlayers)
	{
		if (AActor** ActorPtr = PlayerActors.Find(Player.PlayerId))
		{
			if (AActor* Actor = *ActorPtr)
			{
				FVector CurrentLocation = Actor->GetActorLocation();
				FRotator CurrentRotation = Actor->GetActorRotation();

				FVector NewLocation = FMath::VInterpTo(CurrentLocation, Player.Position, DeltaTime, InterpolationSpeed);
				FRotator NewRotation = FMath::RInterpTo(CurrentRotation, Player.Rotation, DeltaTime, InterpolationSpeed);

				Actor->SetActorLocation(NewLocation);
				Actor->SetActorRotation(NewRotation);
			}
		}
	}
}

FOtherPlayer UPlayerSyncComponent::GetPlayerById(const FString& PlayerId) const
{
	const FOtherPlayer* Found = OtherPlayers.FindByPredicate([&PlayerId](const FOtherPlayer& P) {
		return P.PlayerId == PlayerId;
	});

	return Found ? *Found : FOtherPlayer();
}

void UPlayerSyncComponent::OnPlayerDataReceived(const FString& PlayerId, const FString& JsonData)
{
	// Skip if this is our own player
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		// Compare identities - skip self
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	FOtherPlayer PlayerData;
	PlayerData.PlayerId = PlayerId;
	JsonObject->TryGetStringField(TEXT("username"), PlayerData.Username);

	double PosX, PosY, PosZ;
	JsonObject->TryGetNumberField(TEXT("position_x"), PosX);
	JsonObject->TryGetNumberField(TEXT("position_y"), PosY);
	JsonObject->TryGetNumberField(TEXT("position_z"), PosZ);
	PlayerData.Position = FVector(PosX, PosY, PosZ);

	double RotPitch, RotYaw, RotRoll;
	JsonObject->TryGetNumberField(TEXT("rotation_pitch"), RotPitch);
	JsonObject->TryGetNumberField(TEXT("rotation_yaw"), RotYaw);
	JsonObject->TryGetNumberField(TEXT("rotation_roll"), RotRoll);
	PlayerData.Rotation = FRotator(RotPitch, RotYaw, RotRoll);

	double Health;
	JsonObject->TryGetNumberField(TEXT("health"), Health);
	PlayerData.Health = static_cast<float>(Health);

	JsonObject->TryGetBoolField(TEXT("is_online"), PlayerData.bIsOnline);

	// Handle player leaving
	if (!PlayerData.bIsOnline)
	{
		OtherPlayers.RemoveAll([&PlayerId](const FOtherPlayer& P) { return P.PlayerId == PlayerId; });
		RemovePlayerRepresentation(PlayerId);
		OnPlayerLeft.Broadcast(PlayerId);
		return;
	}

	// Update or add player
	bool bFound = false;
	for (FOtherPlayer& Existing : OtherPlayers)
	{
		if (Existing.PlayerId == PlayerId)
		{
			Existing = PlayerData;
			bFound = true;
			OnPlayerUpdated.Broadcast(PlayerData);
			break;
		}
	}

	if (!bFound)
	{
		OtherPlayers.Add(PlayerData);
		SpawnPlayerRepresentation(PlayerData);
		OnPlayerJoined.Broadcast(PlayerData);
	}
}

void UPlayerSyncComponent::SpawnPlayerRepresentation(const FOtherPlayer& Player)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn a simple representation - in a real game this would be a full character
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// For now, log the spawn
	UE_LOG(LogTemp, Log, TEXT("PlayerSync: Spawning representation for player %s at %s"),
		*Player.Username, *Player.Position.ToString());

	// Store reference (actual spawning would happen here with a proper actor class)
	// PlayerActors.Add(Player.PlayerId, SpawnedActor);
}

void UPlayerSyncComponent::UpdatePlayerRepresentation(const FOtherPlayer& Player)
{
	// Position updates happen in Tick via interpolation
}

void UPlayerSyncComponent::RemovePlayerRepresentation(const FString& PlayerId)
{
	if (AActor** ActorPtr = PlayerActors.Find(PlayerId))
	{
		if (AActor* Actor = *ActorPtr)
		{
			Actor->Destroy();
		}
		PlayerActors.Remove(PlayerId);
	}
	UE_LOG(LogTemp, Log, TEXT("PlayerSync: Removed representation for player %s"), *PlayerId);
}
