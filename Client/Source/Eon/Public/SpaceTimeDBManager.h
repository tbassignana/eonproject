// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "IWebSocket.h"
#include "SpaceTimeDBManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisconnected, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDataReceived, const FString&, PlayerId, const FString&, JsonData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceListReceived, const TArray<FString>&, Instances);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const FString&, JsonData);

USTRUCT(BlueprintType)
struct FSpaceTimeDBConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Host = TEXT("wss://maincloud.spacetimedb.com");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ModuleName = TEXT("eon");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReconnectDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReconnectAttempts = 5;
};

UCLASS()
class EON_API USpaceTimeDBManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
	void Connect(const FSpaceTimeDBConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
	bool IsConnected() const;

	// Player Management
	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Player")
	void RegisterPlayer(const FString& Username);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Player")
	void UpdatePlayerPosition(FVector Position, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Player")
	void SetPlayerOnline(bool bOnline);

	// Instance Management
	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Instance")
	void CreateInstance(const FString& Name, int32 MaxPlayers, bool bIsPublic);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Instance")
	void JoinInstance(int64 InstanceId);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Instance")
	void LeaveInstance();

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Instance")
	void RequestInstanceList();

	// Inventory Management
	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Inventory")
	void AddItemToInventory(const FString& ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Inventory")
	void RemoveItemFromInventory(int64 EntryId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Inventory")
	void UseConsumable(int64 EntryId);

	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Inventory")
	void CollectWorldItem(int64 WorldItemId);

	// Interactables
	UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB|Interactable")
	void ToggleInteractable(const FString& InteractableId);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB|Events")
	FOnConnected OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB|Events")
	FOnDisconnected OnDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB|Events")
	FOnPlayerDataReceived OnPlayerDataReceived;

	UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB|Events")
	FOnInstanceListReceived OnInstanceListReceived;

	UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB|Events")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	void CallReducer(const FString& ReducerName, const TArray<FString>& Args);
	void Subscribe(const FString& Query);
	void HandleMessage(const FString& Message);
	void AttemptReconnect();

private:
	TSharedPtr<IWebSocket> WebSocket;
	FSpaceTimeDBConfig CurrentConfig;
	FString Identity;
	bool bIsConnected = false;
	int32 ReconnectAttempts = 0;
	FTimerHandle ReconnectTimerHandle;
};
