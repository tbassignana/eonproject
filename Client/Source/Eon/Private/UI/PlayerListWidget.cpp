// Copyright 2026 Eon Project. All rights reserved.

#include "UI/PlayerListWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"

void UPlayerListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button callbacks
    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &UPlayerListWidget::OnLeaveButtonClicked);
    }

    UE_LOG(LogTemp, Log, TEXT("PlayerListWidget: Constructed"));
}

void UPlayerListWidget::SetPlayers(const TArray<FPlayerListEntry>& Players)
{
    CachedPlayers = Players;

    // Find local player
    for (const FPlayerListEntry& Player : Players)
    {
        if (Player.bIsLocalPlayer)
        {
            LocalPlayerIdentity = Player.Identity;
            break;
        }
    }

    RebuildPlayerList();

    UE_LOG(LogTemp, Log, TEXT("PlayerListWidget: Updated with %d players"), Players.Num());
}

void UPlayerListWidget::SetIsHost(bool bIsHost)
{
    bLocalPlayerIsHost = bIsHost;
    RebuildPlayerList(); // Rebuild to show/hide kick buttons
}

void UPlayerListWidget::UpdatePlayer(const FPlayerListEntry& Player)
{
    int32 Index = FindPlayerIndex(Player.Identity);
    if (Index >= 0)
    {
        CachedPlayers[Index] = Player;
    }
    else
    {
        CachedPlayers.Add(Player);
    }

    RebuildPlayerList();
}

void UPlayerListWidget::RemovePlayer(const FString& PlayerIdentity)
{
    int32 Index = FindPlayerIndex(PlayerIdentity);
    if (Index >= 0)
    {
        CachedPlayers.RemoveAt(Index);
        RebuildPlayerList();

        UE_LOG(LogTemp, Log, TEXT("PlayerListWidget: Removed player %s"), *PlayerIdentity);
    }
}

void UPlayerListWidget::SetInstanceInfo(const FString& InstanceName, int32 CurrentPlayers, int32 MaxPlayers)
{
    if (InstanceNameText)
    {
        InstanceNameText->SetText(FText::FromString(InstanceName));
    }

    if (PlayerCountText)
    {
        PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), CurrentPlayers, MaxPlayers)));
    }
}

void UPlayerListWidget::OnLeaveButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("PlayerListWidget: Leave requested"));
    OnLeaveRequested.Broadcast();
}

void UPlayerListWidget::RebuildPlayerList()
{
    if (!PlayerListBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerListWidget: PlayerListBox not bound"));
        return;
    }

    PlayerListBox->ClearChildren();

    // Sort players: host first, then local player, then alphabetical
    TArray<FPlayerListEntry> SortedPlayers = CachedPlayers;
    SortedPlayers.Sort([this](const FPlayerListEntry& A, const FPlayerListEntry& B)
    {
        // Host always first
        if (A.bIsHost != B.bIsHost) return A.bIsHost;
        // Local player second
        if (A.bIsLocalPlayer != B.bIsLocalPlayer) return A.bIsLocalPlayer;
        // Then alphabetical
        return A.DisplayName < B.DisplayName;
    });

    for (const FPlayerListEntry& Player : SortedPlayers)
    {
        UWidget* Row = CreatePlayerRow(Player);
        if (Row)
        {
            PlayerListBox->AddChildToVerticalBox(Row);
        }
    }
}

UWidget* UPlayerListWidget::CreatePlayerRow(const FPlayerListEntry& Player)
{
    UHorizontalBox* RowBox = NewObject<UHorizontalBox>(this);

    // Player name with optional indicators
    UTextBlock* NameText = NewObject<UTextBlock>(this);
    FString DisplayText = Player.DisplayName;
    if (Player.bIsHost)
    {
        DisplayText = FString::Printf(TEXT("[HOST] %s"), *Player.DisplayName);
    }
    if (Player.bIsLocalPlayer)
    {
        DisplayText += TEXT(" (You)");
    }
    NameText->SetText(FText::FromString(DisplayText));
    NameText->SetMinDesiredWidth(200.0f);

    // Color local player differently
    if (Player.bIsLocalPlayer)
    {
        NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.7f, 1.0f)));
    }
    else if (Player.bIsHost)
    {
        NameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.2f)));
    }

    RowBox->AddChildToHorizontalBox(NameText);

    // Health bar
    UProgressBar* HealthBar = NewObject<UProgressBar>(this);
    HealthBar->SetPercent(Player.Health / 100.0f);
    FProgressBarStyle HealthStyle;
    if (Player.Health > 60.0f)
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor::Green);
    }
    else if (Player.Health > 30.0f)
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor::Yellow);
    }
    else
    {
        HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
    }
    RowBox->AddChildToHorizontalBox(HealthBar);

    // Score
    UTextBlock* ScoreText = NewObject<UTextBlock>(this);
    ScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d pts"), Player.Score)));
    ScoreText->SetMinDesiredWidth(60.0f);
    RowBox->AddChildToHorizontalBox(ScoreText);

    // Ping (skip for local player)
    if (!Player.bIsLocalPlayer)
    {
        UTextBlock* PingText = NewObject<UTextBlock>(this);
        PingText->SetText(FText::FromString(FString::Printf(TEXT("%.0fms"), Player.Ping)));
        PingText->SetMinDesiredWidth(50.0f);

        // Color code ping
        if (Player.Ping < 50.0f)
        {
            PingText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        }
        else if (Player.Ping < 100.0f)
        {
            PingText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        }
        else
        {
            PingText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        }

        RowBox->AddChildToHorizontalBox(PingText);
    }

    // Kick button (only for host, not for self)
    if (bLocalPlayerIsHost && !Player.bIsLocalPlayer)
    {
        UButton* KickButton = NewObject<UButton>(this);

        UTextBlock* KickText = NewObject<UTextBlock>(this);
        KickText->SetText(FText::FromString(TEXT("Kick")));
        KickButton->AddChild(KickText);

        // Note: In practice, you'd need a custom approach to capture the player identity
        // For now, this demonstrates the structure
        RowBox->AddChildToHorizontalBox(KickButton);
    }

    return RowBox;
}

int32 UPlayerListWidget::FindPlayerIndex(const FString& Identity) const
{
    for (int32 i = 0; i < CachedPlayers.Num(); ++i)
    {
        if (CachedPlayers[i].Identity == Identity)
        {
            return i;
        }
    }
    return -1;
}
