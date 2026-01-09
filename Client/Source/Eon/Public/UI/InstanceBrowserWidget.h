// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InstanceBrowserWidget.generated.h"

/**
 * FGameInstanceInfo - Data about a game instance for display
 */
USTRUCT(BlueprintType)
struct FGameInstanceInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    uint64 InstanceId = 0;

    UPROPERTY(BlueprintReadOnly)
    FString Name;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    FString State; // "Lobby", "InProgress", "Ended"

    bool IsJoinable() const { return State == TEXT("Lobby") && CurrentPlayers < MaxPlayers; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceSelected, const FGameInstanceInfo&, InstanceInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateInstanceRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRefreshRequested);

/**
 * UInstanceBrowserWidget - Widget for browsing and joining game instances
 *
 * This widget displays available game instances from SpaceTimeDB and allows
 * players to join existing instances or create new ones.
 */
UCLASS()
class EON_API UInstanceBrowserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInstanceSelected OnJoinInstance;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCreateInstanceRequested OnCreateInstanceRequested;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRefreshRequested OnRefreshRequested;

    // Update the list of instances
    UFUNCTION(BlueprintCallable, Category = "Instance Browser")
    void SetInstances(const TArray<FGameInstanceInfo>& Instances);

    // Get currently selected instance
    UFUNCTION(BlueprintPure, Category = "Instance Browser")
    FGameInstanceInfo GetSelectedInstance() const { return SelectedInstance; }

    // Check if an instance is selected
    UFUNCTION(BlueprintPure, Category = "Instance Browser")
    bool HasSelectedInstance() const { return SelectedInstance.InstanceId != 0; }

    // Set loading state
    UFUNCTION(BlueprintCallable, Category = "Instance Browser")
    void SetLoading(bool bIsLoading);

    // Show error message
    UFUNCTION(BlueprintCallable, Category = "Instance Browser")
    void ShowError(const FString& ErrorMessage);

protected:
    virtual void NativeConstruct() override;

    // Called when an instance row is clicked
    UFUNCTION()
    void OnInstanceClicked(int32 InstanceIndex);

    // Called when join button is pressed
    UFUNCTION()
    void OnJoinButtonClicked();

    // Called when create button is pressed
    UFUNCTION()
    void OnCreateButtonClicked();

    // Called when refresh button is pressed
    UFUNCTION()
    void OnRefreshButtonClicked();

    // Rebuild the instance list UI
    void RebuildInstanceList();

    // UI Components (bound in C++ or created dynamically)
    UPROPERTY(meta = (BindWidgetOptional))
    class UVerticalBox* InstanceListBox;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* JoinButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* CreateButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* RefreshButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UThrobber* LoadingThrobber;

private:
    UPROPERTY()
    TArray<FGameInstanceInfo> CachedInstances;

    UPROPERTY()
    FGameInstanceInfo SelectedInstance;

    UPROPERTY()
    int32 SelectedIndex = -1;

    UPROPERTY()
    bool bIsLoading = false;
};
