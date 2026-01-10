// Copyright 2026 tbassignana. MIT License.

#include "UI/PlayerListWidget.h"
#include "EonCharacter.h"
#include "PlayerSyncComponent.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// UPlayerListEntryWidget
// ============================================================================

void UPlayerListEntryWidget::SetPlayerData(const FOtherPlayer& PlayerData)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PlayerData.Username));
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(PlayerData.Health / 100.0f);
	}

	if (StatusText)
	{
		FString Status = PlayerData.bIsOnline ? TEXT("Online") : TEXT("Offline");
		StatusText->SetText(FText::FromString(Status));
	}
}

// ============================================================================
// UPlayerListWidget
// ============================================================================

void UPlayerListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Find PlayerSyncComponent
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AEonCharacter* Character = Cast<AEonCharacter>(PC->GetPawn()))
		{
			CachedPlayerSync = Character->PlayerSyncComponent;
		}
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerListWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsShown)
	{
		TimeSinceRefresh += InDeltaTime;
		if (TimeSinceRefresh >= RefreshInterval)
		{
			RefreshPlayerList();
			TimeSinceRefresh = 0.0f;
		}
	}
}

void UPlayerListWidget::RefreshPlayerList()
{
	if (!CachedPlayerSync) return;

	TArray<FOtherPlayer> Players = CachedPlayerSync->GetOtherPlayers();
	UpdatePlayerEntries(Players);

	if (PlayerCountText)
	{
		// +1 for local player
		int32 TotalPlayers = Players.Num() + 1;
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Players: %d"), TotalPlayers)));
	}
}

void UPlayerListWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsShown = true;
	RefreshPlayerList();
}

void UPlayerListWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
	bIsShown = false;
}

void UPlayerListWidget::Toggle()
{
	if (bIsShown)
	{
		Hide();
	}
	else
	{
		Show();
	}
}

void UPlayerListWidget::UpdatePlayerEntries(const TArray<FOtherPlayer>& Players)
{
	if (!PlayerListBox || !PlayerEntryClass) return;

	// Remove extra widgets
	while (EntryWidgets.Num() > Players.Num())
	{
		UPlayerListEntryWidget* Widget = EntryWidgets.Pop();
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}

	// Add missing widgets
	while (EntryWidgets.Num() < Players.Num())
	{
		UPlayerListEntryWidget* Widget = CreateWidget<UPlayerListEntryWidget>(this, PlayerEntryClass);
		if (Widget)
		{
			PlayerListBox->AddChild(Widget);
			EntryWidgets.Add(Widget);
		}
	}

	// Update data
	for (int32 i = 0; i < Players.Num(); ++i)
	{
		if (EntryWidgets.IsValidIndex(i) && EntryWidgets[i])
		{
			EntryWidgets[i]->SetPlayerData(Players[i]);
		}
	}
}
