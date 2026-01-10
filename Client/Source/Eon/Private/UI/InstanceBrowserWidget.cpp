// Copyright 2026 tbassignana. MIT License.

#include "UI/InstanceBrowserWidget.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// UInstanceRowWidget
// ============================================================================

void UInstanceRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UInstanceRowWidget::HandleJoinClicked);
	}
}

void UInstanceRowWidget::SetInstanceData(const FInstanceInfo& Info)
{
	CachedInstanceId = Info.InstanceId;

	if (InstanceNameText)
	{
		InstanceNameText->SetText(FText::FromString(Info.Name));
	}

	if (PlayerCountText)
	{
		FString CountStr = FString::Printf(TEXT("%d / %d"), Info.CurrentPlayers, Info.MaxPlayers);
		PlayerCountText->SetText(FText::FromString(CountStr));
	}

	// Disable join if full
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(Info.CurrentPlayers < Info.MaxPlayers);
	}
}

void UInstanceRowWidget::HandleJoinClicked()
{
	OnJoinClicked.Broadcast(CachedInstanceId);
}

// ============================================================================
// UInstanceBrowserWidget
// ============================================================================

void UInstanceBrowserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::HandleRefreshClicked);
	}

	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::HandleCreateClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::HandleCloseClicked);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UInstanceBrowserWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsShown = true;
	RefreshInstanceList();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UInstanceBrowserWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
	bIsShown = false;

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UInstanceBrowserWidget::RefreshInstanceList()
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Loading...")));
	}

	ClearInstances();

	// Request instance list from SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(GetOwningPlayer()))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->RequestInstanceList();
		}
	}
}

void UInstanceBrowserWidget::AddInstance(const FInstanceInfo& Info)
{
	if (!InstanceListScrollBox || !InstanceRowClass) return;

	UInstanceRowWidget* RowWidget = CreateWidget<UInstanceRowWidget>(this, InstanceRowClass);
	if (RowWidget)
	{
		RowWidget->SetInstanceData(Info);
		RowWidget->OnJoinClicked.AddDynamic(this, &UInstanceBrowserWidget::HandleJoinInstance);
		InstanceListScrollBox->AddChild(RowWidget);
		InstanceRows.Add(RowWidget);
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("%d instances found"), InstanceRows.Num())));
	}
}

void UInstanceBrowserWidget::ClearInstances()
{
	if (InstanceListScrollBox)
	{
		InstanceListScrollBox->ClearChildren();
	}
	InstanceRows.Empty();
}

void UInstanceBrowserWidget::HandleRefreshClicked()
{
	RefreshInstanceList();
}

void UInstanceBrowserWidget::HandleCreateClicked()
{
	// Show create instance dialog
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(GetOwningPlayer()))
	{
		PC->ShowCreateInstanceDialog();
	}
}

void UInstanceBrowserWidget::HandleCloseClicked()
{
	Hide();
}

void UInstanceBrowserWidget::HandleJoinInstance(int64 InstanceId)
{
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(GetOwningPlayer()))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->JoinInstance(InstanceId);
			Hide();
		}
	}
}
