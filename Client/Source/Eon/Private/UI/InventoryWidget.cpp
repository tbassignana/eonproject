// Copyright 2026 Eon Project. All rights reserved.

#include "UI/InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/SizeBox.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind action buttons
    if (UseButton)
    {
        UseButton->OnClicked.AddDynamic(this, &UInventoryWidget::UseSelectedItem);
    }

    if (DropButton)
    {
        DropButton->OnClicked.AddDynamic(this, &UInventoryWidget::DropSelectedItem);
    }

    // Initialize tooltip as hidden
    HideTooltip();

    // Build initial grid
    RebuildInventoryGrid();

    UpdateActionButtons();

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Constructed with %dx%d grid"), GridColumns, GridRows);
}

void UInventoryWidget::SetInventoryItems(const TArray<FInventoryItem>& Items)
{
    CachedItems = Items;

    // Update all slots
    for (int32 i = 0; i < SlotButtons.Num(); ++i)
    {
        UpdateSlot(i);
    }

    // Update capacity display
    if (CapacityText)
    {
        int32 MaxSlots = GridColumns * GridRows;
        CapacityText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Items.Num(), MaxSlots)));
    }

    // Clear selection if item no longer exists
    if (SelectedSlotIndex >= 0)
    {
        FInventoryItem ItemAtSlot = GetItemAtSlot(SelectedSlotIndex);
        if (!ItemAtSlot.IsValid())
        {
            ClearSelection();
        }
    }

    UpdateActionButtons();

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Updated with %d items"), Items.Num());
}

void UInventoryWidget::RefreshFromComponent(UInventoryComponent* InventoryComp)
{
    if (InventoryComp)
    {
        SetInventoryItems(InventoryComp->GetAllItems());
    }
    else
    {
        SetInventoryItems(TArray<FInventoryItem>());
    }
}

void UInventoryWidget::SelectSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= SlotButtons.Num())
    {
        return;
    }

    // Clear previous selection highlight
    if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotButtons.Num())
    {
        // Remove highlight from old selection
        // (In practice, would need custom button style)
    }

    SelectedSlotIndex = SlotIndex;
    SelectedItem = GetItemAtSlot(SlotIndex);

    // Add highlight to new selection
    // (In practice, would need custom button style)

    UpdateActionButtons();
    OnSlotClicked.Broadcast(SlotIndex);

    if (SelectedItem.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Selected slot %d (%s)"),
               SlotIndex, *SelectedItem.ItemName);
    }
}

void UInventoryWidget::ClearSelection()
{
    if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotButtons.Num())
    {
        // Remove highlight
    }

    SelectedSlotIndex = -1;
    SelectedItem = FInventoryItem();

    UpdateActionButtons();
    HideTooltip();
}

void UInventoryWidget::UseSelectedItem()
{
    if (!HasSelectedItem())
    {
        return;
    }

    if (!SelectedItem.bIsConsumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryWidget: Item is not consumable"));
        return;
    }

    OnUseItem.Broadcast(SelectedItem.ItemId);

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Use item requested for %s"),
           *SelectedItem.ItemName);
}

void UInventoryWidget::DropSelectedItem(int32 Quantity)
{
    if (!HasSelectedItem())
    {
        return;
    }

    if (Quantity <= 0)
    {
        Quantity = 1;
    }

    OnDropItem.Broadcast(SelectedItem.ItemId, Quantity);

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Drop item requested for %s x%d"),
           *SelectedItem.ItemName, Quantity);
}

void UInventoryWidget::ShowTooltip(const FInventoryItem& Item, FVector2D ScreenPosition)
{
    if (!TooltipPanel)
    {
        return;
    }

    TooltipPanel->SetVisibility(ESlateVisibility::Visible);
    TooltipPanel->SetRenderTranslation(ScreenPosition);

    if (TooltipNameText)
    {
        TooltipNameText->SetText(FText::FromString(Item.ItemName));
    }

    if (TooltipDescText)
    {
        TooltipDescText->SetText(FText::FromString(Item.ItemDescription));
    }

    if (TooltipStatsText)
    {
        FString Stats;
        if (Item.Damage > 0)
        {
            Stats += FString::Printf(TEXT("Damage: %.0f\n"), Item.Damage);
        }
        if (Item.HealAmount > 0)
        {
            Stats += FString::Printf(TEXT("Heal: %.0f\n"), Item.HealAmount);
        }
        Stats += FString::Printf(TEXT("Quantity: %d"), Item.Quantity);

        TooltipStatsText->SetText(FText::FromString(Stats));
    }
}

void UInventoryWidget::HideTooltip()
{
    if (TooltipPanel)
    {
        TooltipPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventoryWidget::RebuildInventoryGrid()
{
    if (!InventoryGrid)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryWidget: InventoryGrid not bound"));
        return;
    }

    InventoryGrid->ClearChildren();
    SlotButtons.Empty();
    SlotImages.Empty();
    SlotQuantityTexts.Empty();

    int32 TotalSlots = GridColumns * GridRows;

    for (int32 i = 0; i < TotalSlots; ++i)
    {
        UWidget* SlotWidget = CreateSlotWidget(i);
        if (SlotWidget)
        {
            int32 Row = i / GridColumns;
            int32 Col = i % GridColumns;
            InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Built grid with %d slots"), TotalSlots);
}

UWidget* UInventoryWidget::CreateSlotWidget(int32 SlotIndex)
{
    // Create a size box to enforce slot dimensions
    USizeBox* SizeBox = NewObject<USizeBox>(this);
    SizeBox->SetWidthOverride(SlotSize);
    SizeBox->SetHeightOverride(SlotSize);

    // Create button for the slot
    UButton* SlotButton = NewObject<UButton>(this);

    // Create vertical box to hold icon and quantity
    UVerticalBox* SlotContent = NewObject<UVerticalBox>(this);

    // Create item icon image
    UImage* ItemImage = NewObject<UImage>(this);
    ItemImage->SetVisibility(ESlateVisibility::Collapsed); // Hidden until item assigned
    SlotContent->AddChildToVerticalBox(ItemImage);
    SlotImages.Add(ItemImage);

    // Create quantity text
    UTextBlock* QuantityText = NewObject<UTextBlock>(this);
    QuantityText->SetText(FText::GetEmpty());
    QuantityText->SetVisibility(ESlateVisibility::Collapsed);
    SlotContent->AddChildToVerticalBox(QuantityText);
    SlotQuantityTexts.Add(QuantityText);

    SlotButton->AddChild(SlotContent);

    // Store button reference
    SlotButtons.Add(SlotButton);

    // Bind click (need workaround for capturing slot index)
    // For now, use a simpler approach - will need custom widget in production

    SizeBox->AddChild(SlotButton);

    return SizeBox;
}

void UInventoryWidget::UpdateSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= SlotButtons.Num())
    {
        return;
    }

    FInventoryItem Item = GetItemAtSlot(SlotIndex);

    // Update image
    if (SlotIndex < SlotImages.Num())
    {
        UImage* Image = SlotImages[SlotIndex];
        if (Image)
        {
            if (Item.IsValid())
            {
                // In production, would load item icon texture here
                Image->SetVisibility(ESlateVisibility::Visible);
                Image->SetColorAndOpacity(FLinearColor::White);
            }
            else
            {
                Image->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    // Update quantity text
    if (SlotIndex < SlotQuantityTexts.Num())
    {
        UTextBlock* QuantityText = SlotQuantityTexts[SlotIndex];
        if (QuantityText)
        {
            if (Item.IsValid() && Item.Quantity > 1)
            {
                QuantityText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Item.Quantity)));
                QuantityText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                QuantityText->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}

void UInventoryWidget::OnSlotButtonClicked(int32 SlotIndex)
{
    SelectSlot(SlotIndex);
}

void UInventoryWidget::UpdateActionButtons()
{
    if (UseButton)
    {
        UseButton->SetIsEnabled(HasSelectedItem() && SelectedItem.bIsConsumable);
    }

    if (DropButton)
    {
        DropButton->SetIsEnabled(HasSelectedItem());
    }
}

FInventoryItem UInventoryWidget::GetItemAtSlot(int32 SlotIndex) const
{
    for (const FInventoryItem& Item : CachedItems)
    {
        if (Item.SlotIndex == SlotIndex)
        {
            return Item;
        }
    }
    return FInventoryItem();
}
