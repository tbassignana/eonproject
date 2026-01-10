// Copyright 2026 tbassignana. MIT License.

#include "UI/CreateInstanceDialog.h"
#include "SpaceTimeDBManager.h"
#include "EonPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCreateInstanceDialog::NativeConstruct()
{
	Super::NativeConstruct();

	if (CreateButton)
	{
		CreateButton->OnClicked.AddDynamic(this, &UCreateInstanceDialog::HandleCreateClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCreateInstanceDialog::HandleCancelClicked);
	}

	if (InstanceNameInput)
	{
		InstanceNameInput->OnTextChanged.AddDynamic(this, &UCreateInstanceDialog::HandleNameChanged);
	}

	ResetToDefaults();
	SetVisibility(ESlateVisibility::Hidden);
}

void UCreateInstanceDialog::Show()
{
	ResetToDefaults();
	SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	if (InstanceNameInput)
	{
		InstanceNameInput->SetKeyboardFocus();
	}
}

void UCreateInstanceDialog::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UCreateInstanceDialog::ResetToDefaults()
{
	if (InstanceNameInput)
	{
		InstanceNameInput->SetText(FText::GetEmpty());
	}

	if (MaxPlayersSpinBox)
	{
		MaxPlayersSpinBox->SetMinValue(MinPlayers);
		MaxPlayersSpinBox->SetMaxValue(MaxPlayersLimit);
		MaxPlayersSpinBox->SetValue(DefaultMaxPlayers);
	}

	if (IsPublicCheckBox)
	{
		IsPublicCheckBox->SetIsChecked(true);
	}

	ClearError();
}

void UCreateInstanceDialog::HandleCreateClicked()
{
	if (!ValidateInput()) return;

	FString InstanceName = InstanceNameInput ? InstanceNameInput->GetText().ToString() : TEXT("New Instance");
	int32 MaxPlayers = MaxPlayersSpinBox ? static_cast<int32>(MaxPlayersSpinBox->GetValue()) : DefaultMaxPlayers;
	bool bIsPublic = IsPublicCheckBox ? IsPublicCheckBox->IsChecked() : true;

	// Create instance via SpaceTimeDB
	if (AEonPlayerController* PC = Cast<AEonPlayerController>(GetOwningPlayer()))
	{
		if (USpaceTimeDBManager* Manager = PC->GetSpaceTimeDBManager())
		{
			Manager->CreateInstance(InstanceName, MaxPlayers, bIsPublic);
			Hide();
		}
	}
}

void UCreateInstanceDialog::HandleCancelClicked()
{
	Hide();
}

void UCreateInstanceDialog::HandleNameChanged(const FText& Text)
{
	ClearError();
}

bool UCreateInstanceDialog::ValidateInput()
{
	if (!InstanceNameInput)
	{
		ShowError(TEXT("Internal error"));
		return false;
	}

	FString Name = InstanceNameInput->GetText().ToString();
	Name = Name.TrimStartAndEnd();

	if (Name.IsEmpty())
	{
		ShowError(TEXT("Instance name cannot be empty"));
		return false;
	}

	if (Name.Len() < 3)
	{
		ShowError(TEXT("Instance name must be at least 3 characters"));
		return false;
	}

	if (Name.Len() > 32)
	{
		ShowError(TEXT("Instance name cannot exceed 32 characters"));
		return false;
	}

	return true;
}

void UCreateInstanceDialog::ShowError(const FString& Message)
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(Message));
		ErrorText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCreateInstanceDialog::ClearError()
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Hidden);
	}
}
