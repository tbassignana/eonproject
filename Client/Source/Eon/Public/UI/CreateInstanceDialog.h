// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateInstanceDialog.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInstanceCreateConfirmed, const FString&, InstanceName, int32, MaxPlayers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstanceCreateCancelled);

/**
 * UCreateInstanceDialog - Dialog for creating a new game instance
 *
 * Allows players to specify:
 * - Instance name
 * - Max player count (2-8)
 */
UCLASS()
class EON_API UCreateInstanceDialog : public UUserWidget
{
    GENERATED_BODY()

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInstanceCreateConfirmed OnConfirm;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInstanceCreateCancelled OnCancel;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MinPlayers = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxPlayers = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 DefaultMaxPlayers = 4;

    // Reset the dialog to default state
    UFUNCTION(BlueprintCallable, Category = "Create Instance")
    void ResetToDefaults();

    // Validate current input
    UFUNCTION(BlueprintPure, Category = "Create Instance")
    bool IsInputValid() const;

    // Get validation error message
    UFUNCTION(BlueprintPure, Category = "Create Instance")
    FString GetValidationError() const;

    // Set loading state (while creating)
    UFUNCTION(BlueprintCallable, Category = "Create Instance")
    void SetLoading(bool bIsLoading);

    // Show error message
    UFUNCTION(BlueprintCallable, Category = "Create Instance")
    void ShowError(const FString& ErrorMessage);

protected:
    virtual void NativeConstruct() override;

    // Button callbacks
    UFUNCTION()
    void OnConfirmButtonClicked();

    UFUNCTION()
    void OnCancelButtonClicked();

    // Input change callbacks
    UFUNCTION()
    void OnNameInputChanged(const FText& Text);

    UFUNCTION()
    void OnPlayerCountChanged(float Value);

    // Update UI state
    void UpdateConfirmButtonState();

    // UI Components
    UPROPERTY(meta = (BindWidgetOptional))
    class UEditableTextBox* NameInput;

    UPROPERTY(meta = (BindWidgetOptional))
    class USlider* PlayerCountSlider;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* PlayerCountText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* ConfirmButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* CancelButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* ErrorText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UThrobber* LoadingThrobber;

private:
    UPROPERTY()
    FString CurrentName;

    UPROPERTY()
    int32 CurrentMaxPlayers = 4;

    UPROPERTY()
    bool bIsLoading = false;
};
