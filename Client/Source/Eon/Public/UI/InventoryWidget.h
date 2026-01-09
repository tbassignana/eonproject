// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "InventoryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSlotClicked, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUseRequested, int64, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDropRequested, int64, ItemId, int32, Quantity);

/**
 * UInventoryWidget - Widget for displaying and managing player inventory
 *
 * Features:
 * - Grid display of inventory items
 * - Item tooltips with details
 * - Use/Drop item actions
 * - Drag and drop support (future)
 */
UCLASS()
class EON_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 GridColumns = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 GridRows = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float SlotSize = 64.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float SlotPadding = 4.0f;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemSlotClicked OnSlotClicked;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemUseRequested OnUseItem;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemDropRequested OnDropItem;

    // Update the inventory display
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetInventoryItems(const TArray<FInventoryItem>& Items);

    // Refresh from inventory component
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshFromComponent(UInventoryComponent* InventoryComp);

    // Get selected item
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetSelectedItem() const { return SelectedItem; }

    // Check if item is selected
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasSelectedItem() const { return SelectedItem.IsValid(); }

    // Select item at slot
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SelectSlot(int32 SlotIndex);

    // Clear selection
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearSelection();

    // Use selected item
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseSelectedItem();

    // Drop selected item
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropSelectedItem(int32 Quantity = 1);

    // Show/hide tooltip
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowTooltip(const FInventoryItem& Item, FVector2D ScreenPosition);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void HideTooltip();

protected:
    virtual void NativeConstruct() override;

    // Rebuild the inventory grid
    void RebuildInventoryGrid();

    // Create a slot widget
    class UWidget* CreateSlotWidget(int32 SlotIndex);

    // Update a single slot
    void UpdateSlot(int32 SlotIndex);

    // Handle slot click
    UFUNCTION()
    void OnSlotButtonClicked(int32 SlotIndex);

    // UI Components
    UPROPERTY(meta = (BindWidgetOptional))
    class UUniformGridPanel* InventoryGrid;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* TitleText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* CapacityText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* UseButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* DropButton;

    UPROPERTY(meta = (BindWidgetOptional))
    class UBorder* TooltipPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* TooltipNameText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* TooltipDescText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* TooltipStatsText;

private:
    UPROPERTY()
    TArray<FInventoryItem> CachedItems;

    UPROPERTY()
    FInventoryItem SelectedItem;

    UPROPERTY()
    int32 SelectedSlotIndex = -1;

    // Slot button references
    UPROPERTY()
    TArray<class UButton*> SlotButtons;

    // Item images in slots
    UPROPERTY()
    TArray<class UImage*> SlotImages;

    // Quantity text in slots
    UPROPERTY()
    TArray<class UTextBlock*> SlotQuantityTexts;

    // Update button states
    void UpdateActionButtons();

    // Get item at slot index
    FInventoryItem GetItemAtSlot(int32 SlotIndex) const;
};
