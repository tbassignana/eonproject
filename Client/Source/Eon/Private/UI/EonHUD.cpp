// Copyright 2026 tbassignana. MIT License.

#include "UI/EonHUD.h"
#include "EonCharacter.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UEonHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Initial state
	ShowInteractionPrompt(false);
}

void UEonHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateFromCharacter();

	// Update notifications
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		ActiveNotifications[i].RemainingTime -= InDeltaTime;
		if (ActiveNotifications[i].RemainingTime <= 0)
		{
			ActiveNotifications.RemoveAt(i);
		}
	}
}

void UEonHUD::SetHealth(float Current, float Max)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Max > 0 ? Current / Max : 0.0f);
	}

	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	}
}

void UEonHUD::SetInteractionPrompt(const FString& Prompt)
{
	if (InteractionPromptText)
	{
		InteractionPromptText->SetText(FText::FromString(Prompt));
	}
}

void UEonHUD::ShowInteractionPrompt(bool bShow)
{
	if (InteractionPromptText)
	{
		InteractionPromptText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UEonHUD::ShowNotification(const FString& Message, float Duration)
{
	FNotification Notification;
	Notification.Message = Message;
	Notification.RemainingTime = Duration;
	ActiveNotifications.Add(Notification);

	// In a full implementation, would spawn a notification widget
	UE_LOG(LogTemp, Log, TEXT("HUD Notification: %s"), *Message);
}

void UEonHUD::SetPlayerCount(int32 Count)
{
	if (PlayerCountText)
	{
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Players: %d"), Count)));
	}
}

void UEonHUD::SetInstanceName(const FString& Name)
{
	if (InstanceNameText)
	{
		InstanceNameText->SetText(FText::FromString(Name));
	}
}

void UEonHUD::UpdateFromCharacter()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AEonCharacter* Character = Cast<AEonCharacter>(PC->GetPawn());
	if (!Character) return;

	SetHealth(Character->GetHealth(), Character->GetMaxHealth());
}
