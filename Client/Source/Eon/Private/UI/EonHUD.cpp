// Copyright 2026 Eon Project. All rights reserved.

#include "UI/EonHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AEonHUD::AEonHUD()
{
}

void AEonHUD::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("EonHUD: BeginPlay"));

    // Show instance browser at start
    ShowInstanceBrowser();
}

template<typename T>
T* AEonHUD::GetOrCreateWidget(T*& WidgetPtr, TSubclassOf<T> WidgetClass)
{
    if (!WidgetPtr && WidgetClass)
    {
        APlayerController* PC = GetOwningPlayerController();
        if (PC)
        {
            WidgetPtr = CreateWidget<T>(PC, WidgetClass);
        }
    }
    return WidgetPtr;
}

void AEonHUD::ShowInstanceBrowser()
{
    // Hide other screens
    HideCreateInstanceDialog();
    HidePlayerList();

    UInstanceBrowserWidget* Browser = GetOrCreateWidget(InstanceBrowserWidget, InstanceBrowserClass);
    if (Browser)
    {
        if (!Browser->IsInViewport())
        {
            Browser->AddToViewport(10);
        }
        Browser->SetVisibility(ESlateVisibility::Visible);

        // Bind events if not already bound
        if (!Browser->OnJoinInstance.IsBound())
        {
            Browser->OnJoinInstance.AddDynamic(this, &AEonHUD::OnJoinInstanceSelected);
            Browser->OnCreateInstanceRequested.AddDynamic(this, &AEonHUD::OnCreateInstanceRequested);
            Browser->OnRefreshRequested.AddDynamic(this, &AEonHUD::OnRefreshInstancesRequested);
        }

        // Set input mode to UI only
        APlayerController* PC = GetOwningPlayerController();
        if (PC)
        {
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(Browser->TakeWidget());
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }

        UE_LOG(LogTemp, Log, TEXT("EonHUD: Showing instance browser"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EonHUD: Could not create InstanceBrowserWidget. Is InstanceBrowserClass set?"));
    }
}

void AEonHUD::HideInstanceBrowser()
{
    if (InstanceBrowserWidget && InstanceBrowserWidget->IsInViewport())
    {
        InstanceBrowserWidget->SetVisibility(ESlateVisibility::Collapsed);
        UE_LOG(LogTemp, Log, TEXT("EonHUD: Hiding instance browser"));
    }
}

void AEonHUD::ShowCreateInstanceDialog()
{
    UCreateInstanceDialog* Dialog = GetOrCreateWidget(CreateInstanceDialogWidget, CreateInstanceDialogClass);
    if (Dialog)
    {
        if (!Dialog->IsInViewport())
        {
            Dialog->AddToViewport(20); // Above browser
        }
        Dialog->SetVisibility(ESlateVisibility::Visible);
        Dialog->ResetToDefaults();

        // Bind events if not already bound
        if (!Dialog->OnConfirm.IsBound())
        {
            Dialog->OnConfirm.AddDynamic(this, &AEonHUD::OnInstanceCreateConfirmed);
            Dialog->OnCancel.AddDynamic(this, &AEonHUD::OnInstanceCreateCancelled);
        }

        UE_LOG(LogTemp, Log, TEXT("EonHUD: Showing create instance dialog"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EonHUD: Could not create CreateInstanceDialog. Is CreateInstanceDialogClass set?"));
    }
}

void AEonHUD::HideCreateInstanceDialog()
{
    if (CreateInstanceDialogWidget && CreateInstanceDialogWidget->IsInViewport())
    {
        CreateInstanceDialogWidget->SetVisibility(ESlateVisibility::Collapsed);
        UE_LOG(LogTemp, Log, TEXT("EonHUD: Hiding create instance dialog"));
    }
}

void AEonHUD::ShowPlayerList()
{
    // Hide menu screens
    HideInstanceBrowser();
    HideCreateInstanceDialog();

    UPlayerListWidget* List = GetOrCreateWidget(PlayerListWidget, PlayerListClass);
    if (List)
    {
        if (!List->IsInViewport())
        {
            List->AddToViewport(5);
        }
        List->SetVisibility(ESlateVisibility::Visible);

        // Bind events if not already bound
        if (!List->OnLeaveRequested.IsBound())
        {
            List->OnLeaveRequested.AddDynamic(this, &AEonHUD::OnLeaveInstanceRequested);
            List->OnKickPlayer.AddDynamic(this, &AEonHUD::OnKickPlayerRequested);
        }

        // Set input mode to game and UI
        APlayerController* PC = GetOwningPlayerController();
        if (PC)
        {
            FInputModeGameAndUI InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }

        UE_LOG(LogTemp, Log, TEXT("EonHUD: Showing player list"));
    }
}

void AEonHUD::HidePlayerList()
{
    if (PlayerListWidget && PlayerListWidget->IsInViewport())
    {
        PlayerListWidget->SetVisibility(ESlateVisibility::Collapsed);

        // Set input mode to game only
        APlayerController* PC = GetOwningPlayerController();
        if (PC)
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }

        UE_LOG(LogTemp, Log, TEXT("EonHUD: Hiding player list"));
    }
}

void AEonHUD::TogglePlayerList()
{
    if (PlayerListWidget && PlayerListWidget->IsVisible())
    {
        HidePlayerList();
    }
    else
    {
        ShowPlayerList();
    }
}

void AEonHUD::UpdatePlayerList(const TArray<FPlayerListEntry>& Players)
{
    if (PlayerListWidget)
    {
        PlayerListWidget->SetPlayers(Players);
    }
}

void AEonHUD::UpdateInstanceList(const TArray<FGameInstanceInfo>& Instances)
{
    if (InstanceBrowserWidget)
    {
        InstanceBrowserWidget->SetInstances(Instances);
    }
}

void AEonHUD::OnJoinInstanceSelected(const FGameInstanceInfo& InstanceInfo)
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Join instance requested: %llu (%s)"),
           InstanceInfo.InstanceId, *InstanceInfo.Name);

    // TODO: Call SpaceTimeDBManager to join the instance
    // For now, transition to in-game UI
    HideInstanceBrowser();
    ShowPlayerList();
}

void AEonHUD::OnCreateInstanceRequested()
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Create instance requested"));
    ShowCreateInstanceDialog();
}

void AEonHUD::OnRefreshInstancesRequested()
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Refresh instances requested"));

    // TODO: Call SpaceTimeDBManager to refresh instance list
    if (InstanceBrowserWidget)
    {
        InstanceBrowserWidget->SetLoading(true);
    }
}

void AEonHUD::OnInstanceCreateConfirmed(const FString& InstanceName, int32 MaxPlayers)
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Creating instance '%s' with max %d players"),
           *InstanceName, MaxPlayers);

    // TODO: Call SpaceTimeDBManager to create the instance
    HideCreateInstanceDialog();

    // For now, transition to in-game UI
    ShowPlayerList();
}

void AEonHUD::OnInstanceCreateCancelled()
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Instance creation cancelled"));
    HideCreateInstanceDialog();
}

void AEonHUD::OnLeaveInstanceRequested()
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Leave instance requested"));

    // TODO: Call SpaceTimeDBManager to leave the instance
    HidePlayerList();
    ShowInstanceBrowser();
}

void AEonHUD::OnKickPlayerRequested(const FString& PlayerIdentity)
{
    UE_LOG(LogTemp, Log, TEXT("EonHUD: Kick player requested: %s"), *PlayerIdentity);

    // TODO: Call SpaceTimeDBManager to kick the player (if host)
}
