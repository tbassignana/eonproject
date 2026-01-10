// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class UButton;
class UTextBlock;
class UImage;

UCLASS()
class EON_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const FInventorySlot& SlotData);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Clear();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetSlotIndex() const { return SlotIndex; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable)
	FOnSlotClicked OnSlotClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* SlotButton;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuantityText;

	UFUNCTION()
	void HandleButtonClicked();

	virtual void NativeConstruct() override;

private:
	int32 SlotIndex = -1;
	int64 EntryId = 0;
	FInventorySlot CurrentSlot;
};

UCLASS()
class EON_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Hide();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsShown() const { return bIsShown; }

protected:
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* InventoryGrid;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UButton* UseButton;

	UPROPERTY(meta = (BindWidget))
	UButton* DropButton;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 GridColumns = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 GridRows = 4;

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleUseClicked();

	UFUNCTION()
	void HandleDropClicked();

	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex);

	UFUNCTION()
	void OnInventoryChanged();

private:
	bool bIsShown = false;
	int32 SelectedSlotIndex = -1;

	UPROPERTY()
	TArray<UInventorySlotWidget*> SlotWidgets;

	UPROPERTY()
	UInventoryComponent* CachedInventory;

	void CreateSlotWidgets();
	void UpdateSlotWidgets();
	void SelectSlot(int32 SlotIndex);
	void ClearSelection();
};
