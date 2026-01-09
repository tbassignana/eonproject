// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "SpaceTimeDBManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisconnected, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnError, const FString&, Error, bool, WasClean);

/**
 * USpaceTimeDBManager - Manages connection to SpaceTimeDB backend
 *
 * This subsystem handles:
 * - WebSocket connection to SpaceTimeDB
 * - Sending reducer calls (RPCs)
 * - Receiving subscription updates
 * - Parsing SpaceTimeDB protocol messages
 */
UCLASS()
class EON_API USpaceTimeDBManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Connection management
    UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
    void Connect(const FString& ServerUrl, const FString& DatabaseName);

    UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
    void Disconnect();

    UFUNCTION(BlueprintPure, Category = "SpaceTimeDB")
    bool IsConnected() const;

    UFUNCTION(BlueprintPure, Category = "SpaceTimeDB")
    FString GetIdentity() const { return Identity; }

    // Reducer calls
    UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
    void CallReducer(const FString& ReducerName, const TArray<FString>& Args);

    // Subscriptions
    UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
    void Subscribe(const FString& TableName);

    UFUNCTION(BlueprintCallable, Category = "SpaceTimeDB")
    void Unsubscribe(const FString& TableName);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB")
    FOnConnected OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB")
    FOnDisconnected OnDisconnected;

    UPROPERTY(BlueprintAssignable, Category = "SpaceTimeDB")
    FOnError OnError;

protected:
    // WebSocket callbacks
    void HandleWebSocketConnected();
    void HandleWebSocketConnectionError(const FString& Error);
    void HandleWebSocketClosed(int32 StatusCode, const FString& Reason, bool WasClean);
    void HandleWebSocketMessage(const FString& Message);

    // Message handlers
    void HandleIdentityToken(const TSharedPtr<FJsonObject>& Message);
    void HandleSubscriptionUpdate(const TSharedPtr<FJsonObject>& Message);
    void HandleTransactionUpdate(const TSharedPtr<FJsonObject>& Message);
    void HandleReducerResponse(const TSharedPtr<FJsonObject>& Message);

    // Protocol helpers
    void SendMessage(const TSharedPtr<FJsonObject>& Message);
    uint32 GenerateRequestId();

private:
    TSharedPtr<IWebSocket> WebSocket;
    FString Identity;
    FString Token;
    uint32 NextRequestId = 1;

    TMap<uint32, FString> PendingReducerCalls;
    TSet<FString> ActiveSubscriptions;

    bool bIsConnecting = false;
};
