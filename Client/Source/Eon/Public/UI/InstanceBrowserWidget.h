// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InstanceBrowserWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;

USTRUCT(BlueprintType)
struct FInstanceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 InstanceId = 0;

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPublic = true;

	UPROPERTY(BlueprintReadOnly)
	FString OwnerName;
};

UCLASS()
class EON_API UInstanceRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Instance")
	void SetInstanceData(const FInstanceInfo& Info);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinClicked, int64, InstanceId);
	UPROPERTY(BlueprintAssignable)
	FOnJoinClicked OnJoinClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InstanceNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleJoinClicked();

private:
	int64 CachedInstanceId = 0;
};

UCLASS()
class EON_API UInstanceBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void RefreshInstanceList();

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void AddInstance(const FInstanceInfo& Info);

	UFUNCTION(BlueprintCallable, Category = "Instance")
	void ClearInstances();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* InstanceListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(EditDefaultsOnly, Category = "Instance")
	TSubclassOf<UInstanceRowWidget> InstanceRowClass;

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleJoinInstance(int64 InstanceId);

private:
	bool bIsShown = false;

	UPROPERTY()
	TArray<UInstanceRowWidget*> InstanceRows;
};
