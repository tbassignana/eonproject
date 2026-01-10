// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerSyncComponent.generated.h"

USTRUCT(BlueprintType)
struct FOtherPlayer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly)
	FString Username;

	UPROPERTY(BlueprintReadOnly)
	FVector Position;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
	float Health = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FOtherPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUpdated, const FOtherPlayer&, Player);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UPlayerSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerSyncComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PlayerSync")
	TArray<FOtherPlayer> GetOtherPlayers() const { return OtherPlayers; }

	UFUNCTION(BlueprintCallable, Category = "PlayerSync")
	FOtherPlayer GetPlayerById(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "PlayerSync")
	int32 GetPlayerCount() const { return OtherPlayers.Num(); }

	// Called when receiving player data from SpaceTimeDB
	UFUNCTION()
	void OnPlayerDataReceived(const FString& PlayerId, const FString& JsonData);

	UPROPERTY(BlueprintAssignable, Category = "PlayerSync")
	FOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "PlayerSync")
	FOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "PlayerSync")
	FOnPlayerUpdated OnPlayerUpdated;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "PlayerSync")
	float InterpolationSpeed = 10.0f;

private:
	UPROPERTY()
	TArray<FOtherPlayer> OtherPlayers;

	// Map of player representations in world
	UPROPERTY()
	TMap<FString, AActor*> PlayerActors;

	void SpawnPlayerRepresentation(const FOtherPlayer& Player);
	void UpdatePlayerRepresentation(const FOtherPlayer& Player);
	void RemovePlayerRepresentation(const FString& PlayerId);
};
