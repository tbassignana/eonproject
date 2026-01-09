// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/InstanceBrowserWidget.h"
#include "UI/CreateInstanceDialog.h"
#include "UI/PlayerListWidget.h"
#include "EonHUD.generated.h"

/**
 * AEonHUD - Main HUD class that manages all game UI
 *
 * Handles showing/hiding different UI states:
 * - Main menu / Instance browser
 * - Create instance dialog
 * - In-game player list
 * - Health/inventory overlays
 */
UCLASS()
class EON_API AEonHUD : public AHUD
{
    GENERATED_BODY()

public:
    AEonHUD();

    // Widget classes (set in Blueprint or code)
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInstanceBrowserWidget> InstanceBrowserClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCreateInstanceDialog> CreateInstanceDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPlayerListWidget> PlayerListClass;

    // Show instance browser (main menu)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowInstanceBrowser();

    // Hide instance browser
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideInstanceBrowser();

    // Show create instance dialog
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowCreateInstanceDialog();

    // Hide create instance dialog
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideCreateInstanceDialog();

    // Show in-game player list
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowPlayerList();

    // Hide player list
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HidePlayerList();

    // Toggle player list visibility
    UFUNCTION(BlueprintCallable, Category = "UI")
    void TogglePlayerList();

    // Get widget instances
    UFUNCTION(BlueprintPure, Category = "UI")
    UInstanceBrowserWidget* GetInstanceBrowser() const { return InstanceBrowserWidget; }

    UFUNCTION(BlueprintPure, Category = "UI")
    UCreateInstanceDialog* GetCreateInstanceDialog() const { return CreateInstanceDialogWidget; }

    UFUNCTION(BlueprintPure, Category = "UI")
    UPlayerListWidget* GetPlayerList() const { return PlayerListWidget; }

    // Update player list data
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdatePlayerList(const TArray<FPlayerListEntry>& Players);

    // Update instance browser data
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateInstanceList(const TArray<FGameInstanceInfo>& Instances);

protected:
    virtual void BeginPlay() override;

    // Event handlers
    UFUNCTION()
    void OnJoinInstanceSelected(const FGameInstanceInfo& InstanceInfo);

    UFUNCTION()
    void OnCreateInstanceRequested();

    UFUNCTION()
    void OnRefreshInstancesRequested();

    UFUNCTION()
    void OnInstanceCreateConfirmed(const FString& InstanceName, int32 MaxPlayers);

    UFUNCTION()
    void OnInstanceCreateCancelled();

    UFUNCTION()
    void OnLeaveInstanceRequested();

    UFUNCTION()
    void OnKickPlayerRequested(const FString& PlayerIdentity);

private:
    // Widget instances
    UPROPERTY()
    UInstanceBrowserWidget* InstanceBrowserWidget = nullptr;

    UPROPERTY()
    UCreateInstanceDialog* CreateInstanceDialogWidget = nullptr;

    UPROPERTY()
    UPlayerListWidget* PlayerListWidget = nullptr;

    // Create widget if needed
    template<typename T>
    T* GetOrCreateWidget(T*& WidgetPtr, TSubclassOf<T> WidgetClass);
};
