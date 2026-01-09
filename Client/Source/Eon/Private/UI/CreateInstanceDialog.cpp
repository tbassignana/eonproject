// Copyright 2026 Eon Project. All rights reserved.

#include "UI/CreateInstanceDialog.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Throbber.h"

void UCreateInstanceDialog::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button callbacks
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UCreateInstanceDialog::OnConfirmButtonClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UCreateInstanceDialog::OnCancelButtonClicked);
    }

    // Bind input callbacks
    if (NameInput)
    {
        NameInput->OnTextChanged.AddDynamic(this, &UCreateInstanceDialog::OnNameInputChanged);
    }

    if (PlayerCountSlider)
    {
        PlayerCountSlider->OnValueChanged.AddDynamic(this, &UCreateInstanceDialog::OnPlayerCountChanged);
        PlayerCountSlider->SetMinValue(static_cast<float>(MinPlayers));
        PlayerCountSlider->SetMaxValue(static_cast<float>(MaxPlayers));
        PlayerCountSlider->SetStepSize(1.0f);
    }

    ResetToDefaults();

    UE_LOG(LogTemp, Log, TEXT("CreateInstanceDialog: Constructed"));
}

void UCreateInstanceDialog::ResetToDefaults()
{
    CurrentName = TEXT("");
    CurrentMaxPlayers = DefaultMaxPlayers;

    if (NameInput)
    {
        NameInput->SetText(FText::GetEmpty());
    }

    if (PlayerCountSlider)
    {
        PlayerCountSlider->SetValue(static_cast<float>(DefaultMaxPlayers));
    }

    if (PlayerCountText)
    {
        PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d players"), DefaultMaxPlayers)));
    }

    if (ErrorText)
    {
        ErrorText->SetVisibility(ESlateVisibility::Collapsed);
    }

    SetLoading(false);
    UpdateConfirmButtonState();
}

bool UCreateInstanceDialog::IsInputValid() const
{
    // Name must not be empty
    if (CurrentName.IsEmpty())
    {
        return false;
    }

    // Name must not be too long
    if (CurrentName.Len() > 32)
    {
        return false;
    }

    // Player count must be in valid range
    if (CurrentMaxPlayers < MinPlayers || CurrentMaxPlayers > MaxPlayers)
    {
        return false;
    }

    return true;
}

FString UCreateInstanceDialog::GetValidationError() const
{
    if (CurrentName.IsEmpty())
    {
        return TEXT("Please enter a game name");
    }

    if (CurrentName.Len() > 32)
    {
        return TEXT("Name must be 32 characters or less");
    }

    if (CurrentMaxPlayers < MinPlayers)
    {
        return FString::Printf(TEXT("Minimum %d players required"), MinPlayers);
    }

    if (CurrentMaxPlayers > MaxPlayers)
    {
        return FString::Printf(TEXT("Maximum %d players allowed"), MaxPlayers);
    }

    return TEXT("");
}

void UCreateInstanceDialog::SetLoading(bool bLoading)
{
    bIsLoading = bLoading;

    if (LoadingThrobber)
    {
        LoadingThrobber->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (ConfirmButton)
    {
        ConfirmButton->SetIsEnabled(!bLoading && IsInputValid());
    }

    if (CancelButton)
    {
        CancelButton->SetIsEnabled(!bLoading);
    }

    if (NameInput)
    {
        NameInput->SetIsEnabled(!bLoading);
    }

    if (PlayerCountSlider)
    {
        PlayerCountSlider->SetIsEnabled(!bLoading);
    }
}

void UCreateInstanceDialog::ShowError(const FString& ErrorMessage)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(ErrorMessage));
        ErrorText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }

    SetLoading(false);
}

void UCreateInstanceDialog::OnConfirmButtonClicked()
{
    if (!IsInputValid())
    {
        ShowError(GetValidationError());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("CreateInstanceDialog: Creating instance '%s' with %d max players"),
           *CurrentName, CurrentMaxPlayers);

    SetLoading(true);
    OnConfirm.Broadcast(CurrentName, CurrentMaxPlayers);
}

void UCreateInstanceDialog::OnCancelButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("CreateInstanceDialog: Cancelled"));
    OnCancel.Broadcast();
}

void UCreateInstanceDialog::OnNameInputChanged(const FText& Text)
{
    CurrentName = Text.ToString().TrimStartAndEnd();

    // Hide error when user starts typing
    if (ErrorText)
    {
        ErrorText->SetVisibility(ESlateVisibility::Collapsed);
    }

    UpdateConfirmButtonState();
}

void UCreateInstanceDialog::OnPlayerCountChanged(float Value)
{
    CurrentMaxPlayers = FMath::RoundToInt(Value);

    if (PlayerCountText)
    {
        PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d players"), CurrentMaxPlayers)));
    }

    UpdateConfirmButtonState();
}

void UCreateInstanceDialog::UpdateConfirmButtonState()
{
    if (ConfirmButton)
    {
        ConfirmButton->SetIsEnabled(!bIsLoading && IsInputValid());
    }
}
