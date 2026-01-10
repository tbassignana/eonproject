// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerSyncComponent.h"
#include "PlayerListWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UProgressBar;

UCLASS()
class EON_API UPlayerListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerData(const FOtherPlayer& PlayerData);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;
};

UCLASS()
class EON_API UPlayerListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void RefreshPlayerList();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Toggle();

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PlayerListBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<UPlayerListEntryWidget> PlayerEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float RefreshInterval = 1.0f;

private:
	bool bIsShown = false;
	float TimeSinceRefresh = 0.0f;

	UPROPERTY()
	UPlayerSyncComponent* CachedPlayerSync;

	UPROPERTY()
	TArray<UPlayerListEntryWidget*> EntryWidgets;

	void UpdatePlayerEntries(const TArray<FOtherPlayer>& Players);
};
