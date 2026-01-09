// Copyright 2026 Eon Project. All rights reserved.

#include "UI/InstanceBrowserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Throbber.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"

void UInstanceBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button callbacks
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::OnJoinButtonClicked);
    }

    if (CreateButton)
    {
        CreateButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::OnCreateButtonClicked);
    }

    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::OnRefreshButtonClicked);
    }

    // Initial state
    SetLoading(false);

    UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Constructed"));
}

void UInstanceBrowserWidget::SetInstances(const TArray<FGameInstanceInfo>& Instances)
{
    CachedInstances = Instances;
    SelectedIndex = -1;
    SelectedInstance = FGameInstanceInfo();

    RebuildInstanceList();

    // Update status text
    if (StatusText)
    {
        if (Instances.Num() == 0)
        {
            StatusText->SetText(FText::FromString(TEXT("No games available. Create one!")));
        }
        else
        {
            StatusText->SetText(FText::FromString(FString::Printf(TEXT("%d game(s) found"), Instances.Num())));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Updated with %d instances"), Instances.Num());
}

void UInstanceBrowserWidget::SetLoading(bool bLoading)
{
    bIsLoading = bLoading;

    if (LoadingThrobber)
    {
        LoadingThrobber->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (JoinButton)
    {
        JoinButton->SetIsEnabled(!bLoading && HasSelectedInstance() && SelectedInstance.IsJoinable());
    }

    if (CreateButton)
    {
        CreateButton->SetIsEnabled(!bLoading);
    }

    if (RefreshButton)
    {
        RefreshButton->SetIsEnabled(!bLoading);
    }
}

void UInstanceBrowserWidget::ShowError(const FString& ErrorMessage)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(FString::Printf(TEXT("Error: %s"), *ErrorMessage)));
        StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
    }

    SetLoading(false);
}

void UInstanceBrowserWidget::OnInstanceClicked(int32 InstanceIndex)
{
    if (InstanceIndex >= 0 && InstanceIndex < CachedInstances.Num())
    {
        SelectedIndex = InstanceIndex;
        SelectedInstance = CachedInstances[InstanceIndex];

        // Update button state
        if (JoinButton)
        {
            JoinButton->SetIsEnabled(SelectedInstance.IsJoinable());
        }

        // Rebuild to show selection highlight
        RebuildInstanceList();

        UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Selected instance %llu (%s)"),
               SelectedInstance.InstanceId, *SelectedInstance.Name);
    }
}

void UInstanceBrowserWidget::OnJoinButtonClicked()
{
    if (HasSelectedInstance() && SelectedInstance.IsJoinable())
    {
        UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Join requested for instance %llu"),
               SelectedInstance.InstanceId);
        OnJoinInstance.Broadcast(SelectedInstance);
    }
}

void UInstanceBrowserWidget::OnCreateButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Create instance requested"));
    OnCreateInstanceRequested.Broadcast();
}

void UInstanceBrowserWidget::OnRefreshButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("InstanceBrowserWidget: Refresh requested"));
    SetLoading(true);
    OnRefreshRequested.Broadcast();
}

void UInstanceBrowserWidget::RebuildInstanceList()
{
    if (!InstanceListBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("InstanceBrowserWidget: InstanceListBox not bound"));
        return;
    }

    // Clear existing entries
    InstanceListBox->ClearChildren();

    // Create entry for each instance
    for (int32 i = 0; i < CachedInstances.Num(); ++i)
    {
        const FGameInstanceInfo& Info = CachedInstances[i];

        // Create a horizontal box for the row
        UHorizontalBox* RowBox = NewObject<UHorizontalBox>(this);

        // Instance name
        UTextBlock* NameText = NewObject<UTextBlock>(this);
        NameText->SetText(FText::FromString(Info.Name));
        NameText->SetMinDesiredWidth(200.0f);
        RowBox->AddChildToHorizontalBox(NameText);

        // Player count
        UTextBlock* PlayersText = NewObject<UTextBlock>(this);
        PlayersText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Info.CurrentPlayers, Info.MaxPlayers)));
        PlayersText->SetMinDesiredWidth(60.0f);
        RowBox->AddChildToHorizontalBox(PlayersText);

        // State
        UTextBlock* StateText = NewObject<UTextBlock>(this);
        StateText->SetText(FText::FromString(Info.State));
        StateText->SetMinDesiredWidth(80.0f);

        // Color-code state
        if (Info.State == TEXT("Lobby"))
        {
            StateText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        }
        else if (Info.State == TEXT("InProgress"))
        {
            StateText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        }
        else
        {
            StateText->SetColorAndOpacity(FSlateColor(FLinearColor::Gray));
        }
        RowBox->AddChildToHorizontalBox(StateText);

        // Wrap in a button for click handling
        UButton* RowButton = NewObject<UButton>(this);
        RowButton->AddChild(RowBox);

        // Highlight if selected
        if (i == SelectedIndex)
        {
            FButtonStyle SelectedStyle;
            SelectedStyle.Normal.TintColor = FSlateColor(FLinearColor(0.2f, 0.4f, 0.8f, 1.0f));
            RowButton->SetStyle(SelectedStyle);
        }

        // Bind click event - we use a lambda workaround via index capture
        const int32 CapturedIndex = i;
        RowButton->OnClicked.AddDynamic(this, &UInstanceBrowserWidget::OnJoinButtonClicked);
        // Note: We need a different approach for per-row callbacks

        InstanceListBox->AddChildToVerticalBox(RowButton);
    }
}
