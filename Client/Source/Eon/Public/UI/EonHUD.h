// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EonHUD.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;
class UVerticalBox;

UCLASS()
class EON_API UEonHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(const FString& Prompt);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowInteractionPrompt(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowNotification(const FString& Message, float Duration = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPlayerCount(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInstanceName(const FString& Name);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractionPromptText;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* NotificationContainer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InstanceNameText;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> NotificationWidgetClass;

private:
	void UpdateFromCharacter();

	struct FNotification
	{
		FString Message;
		float RemainingTime;
	};
	TArray<FNotification> ActiveNotifications;
};
