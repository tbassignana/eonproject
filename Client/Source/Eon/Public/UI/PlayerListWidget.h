// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerListWidget.generated.h"

/**
 * FPlayerListEntry - Data about a player for display
 */
USTRUCT(BlueprintType)
struct FPlayerListEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Identity;

    UPROPERTY(BlueprintReadOnly)
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly)
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLocalPlayer = false;

    UPROPERTY(BlueprintReadOnly)
    bool bIsHost = false;

    UPROPERTY(BlueprintReadOnly)
    int32 Score = 0;

    UPROPERTY(BlueprintReadOnly)
    float Ping = 0.0f; // In milliseconds
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerKickRequested, const FString&, PlayerIdentity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeaveRequested);

/**
 * UPlayerListWidget - Widget for displaying players in the current game instance
 *
 * Shows all connected players with their status information.
 * Allows host to kick players.
 */
UCLASS()
class EON_API UPlayerListWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerKickRequested OnKickPlayer;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLeaveRequested OnLeaveRequested;

    // Update the player list
    UFUNCTION(BlueprintCallable, Category = "Player List")
    void SetPlayers(const TArray<FPlayerListEntry>& Players);

    // Set whether local player is host (can kick others)
    UFUNCTION(BlueprintCallable, Category = "Player List")
    void SetIsHost(bool bIsHost);

    // Update a single player's data
    UFUNCTION(BlueprintCallable, Category = "Player List")
    void UpdatePlayer(const FPlayerListEntry& Player);

    // Remove a player from the list
    UFUNCTION(BlueprintCallable, Category = "Player List")
    void RemovePlayer(const FString& PlayerIdentity);

    // Get player count
    UFUNCTION(BlueprintPure, Category = "Player List")
    int32 GetPlayerCount() const { return CachedPlayers.Num(); }

    // Set instance info header
    UFUNCTION(BlueprintCallable, Category = "Player List")
    void SetInstanceInfo(const FString& InstanceName, int32 CurrentPlayers, int32 MaxPlayers);

protected:
    virtual void NativeConstruct() override;

    // Button callbacks
    UFUNCTION()
    void OnLeaveButtonClicked();

    // Rebuild the player list UI
    void RebuildPlayerList();

    // Create a row widget for a player
    class UWidget* CreatePlayerRow(const FPlayerListEntry& Player);

    // UI Components
    UPROPERTY(meta = (BindWidgetOptional))
    class UVerticalBox* PlayerListBox;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* InstanceNameText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* PlayerCountText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* LeaveButton;

private:
    UPROPERTY()
    TArray<FPlayerListEntry> CachedPlayers;

    UPROPERTY()
    bool bLocalPlayerIsHost = false;

    UPROPERTY()
    FString LocalPlayerIdentity;

    // Find player index by identity
    int32 FindPlayerIndex(const FString& Identity) const;
};
