// Copyright 2026 Eon Project. All rights reserved.

#include "EonPlayerController.h"

AEonPlayerController::AEonPlayerController()
{
    // Enable mouse cursor by default for UI
    bShowMouseCursor = false;
}

void AEonPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // TODO: Auto-connect to SpaceTimeDB on begin play
    // ConnectToServer(DefaultServerAddress);
}

void AEonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Additional input bindings can be added here
}

void AEonPlayerController::ConnectToServer(const FString& ServerAddress)
{
    UE_LOG(LogTemp, Log, TEXT("Connecting to SpaceTimeDB server: %s"), *ServerAddress);

    // TODO: Implement SpaceTimeDB WebSocket connection
    // 1. Create WebSocket connection
    // 2. Send initial handshake
    // 3. Subscribe to relevant tables (player, game_instance, etc.)
    // 4. Set bIsConnected = true on success

    bIsConnected = false; // Will be set to true when connection succeeds
}

void AEonPlayerController::DisconnectFromServer()
{
    UE_LOG(LogTemp, Log, TEXT("Disconnecting from SpaceTimeDB server"));

    // TODO: Implement disconnect
    // 1. Call leave_instance reducer if in an instance
    // 2. Close WebSocket connection
    // 3. Clear subscriptions

    bIsConnected = false;
    PlayerIdentity = TEXT("");
    CurrentInstanceId = -1;
}

void AEonPlayerController::JoinInstance(int64 InstanceId)
{
    if (!bIsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot join instance - not connected to server"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Joining instance %lld"), InstanceId);

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("join_instance", InstanceId);

    CurrentInstanceId = InstanceId;
}

void AEonPlayerController::LeaveInstance()
{
    if (!bIsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot leave instance - not connected to server"));
        return;
    }

    if (CurrentInstanceId < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not currently in an instance"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Leaving instance %lld"), CurrentInstanceId);

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("leave_instance");

    CurrentInstanceId = -1;
}

void AEonPlayerController::CreateInstance(const FString& InstanceName, int32 MaxPlayers)
{
    if (!bIsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create instance - not connected to server"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Creating instance '%s' with max %d players"), *InstanceName, MaxPlayers);

    // TODO: Call SpaceTimeDB reducer
    // NetworkManager->CallReducer("create_instance", InstanceName, MaxPlayers);
    // The new instance ID will come via subscription update
}
