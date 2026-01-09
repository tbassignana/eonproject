// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EonPlayerController.generated.h"

/**
 * AEonPlayerController - Player controller for the Eon game
 *
 * Handles:
 * - Input configuration
 * - UI management
 * - Network connection state
 */
UCLASS()
class EON_API AEonPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEonPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // Network connection state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Network)
    bool bIsConnected = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Network)
    FString PlayerIdentity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Network)
    int64 CurrentInstanceId = -1;

public:
    // Network state accessors
    UFUNCTION(BlueprintCallable, Category = Network)
    bool IsConnectedToServer() const { return bIsConnected; }

    UFUNCTION(BlueprintCallable, Category = Network)
    FString GetPlayerIdentity() const { return PlayerIdentity; }

    UFUNCTION(BlueprintCallable, Category = Network)
    int64 GetCurrentInstanceId() const { return CurrentInstanceId; }

    // Network actions
    UFUNCTION(BlueprintCallable, Category = Network)
    void ConnectToServer(const FString& ServerAddress);

    UFUNCTION(BlueprintCallable, Category = Network)
    void DisconnectFromServer();

    UFUNCTION(BlueprintCallable, Category = Network)
    void JoinInstance(int64 InstanceId);

    UFUNCTION(BlueprintCallable, Category = Network)
    void LeaveInstance();

    UFUNCTION(BlueprintCallable, Category = Network)
    void CreateInstance(const FString& InstanceName, int32 MaxPlayers);
};
