// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateInstanceDialog.generated.h"

class UEditableTextBox;
class UCheckBox;
class USpinBox;
class UButton;
class UTextBlock;

UCLASS()
class EON_API UCreateInstanceDialog : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void ResetToDefaults();

protected:
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InstanceNameInput;

	UPROPERTY(meta = (BindWidget))
	USpinBox* MaxPlayersSpinBox;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* IsPublicCheckBox;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ErrorText;

	UPROPERTY(EditDefaultsOnly, Category = "Instance")
	int32 DefaultMaxPlayers = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Instance")
	int32 MinPlayers = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Instance")
	int32 MaxPlayersLimit = 16;

	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleNameChanged(const FText& Text);

private:
	bool ValidateInput();
	void ShowError(const FString& Message);
	void ClearError();
};
