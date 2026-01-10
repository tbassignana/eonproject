// Copyright 2026 tbassignana. MIT License.

#include "UI/InventoryWidget.h"
#include "EonCharacter.h"
#include "InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// UInventorySlotWidget
// ============================================================================

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &UInventorySlotWidget::HandleButtonClicked);
	}

	Clear();
}

void UInventorySlotWidget::SetSlotData(const FInventorySlot& SlotData)
{
	CurrentSlot = SlotData;
	SlotIndex = SlotData.SlotIndex;
	EntryId = SlotData.EntryId;

	if (ItemIcon)
	{
		// Would load icon texture based on item
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}

	if (QuantityText)
	{
		if (SlotData.Quantity > 1)
		{
			QuantityText->SetText(FText::AsNumber(SlotData.Quantity));
			QuantityText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			QuantityText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UInventorySlotWidget::Clear()
{
	CurrentSlot = FInventorySlot();
	EntryId = 0;

	if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (QuantityText)
	{
		QuantityText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventorySlotWidget::HandleButtonClicked()
{
	OnSlotClicked.Broadcast(SlotIndex);
}

// ============================================================================
// UInventoryWidget
// ============================================================================

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleCloseClicked);
	}

	if (UseButton)
	{
		UseButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleUseClicked);
	}

	if (DropButton)
	{
		DropButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleDropClicked);
	}

	CreateSlotWidgets();

	// Find player's inventory component
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AEonCharacter* Character = Cast<AEonCharacter>(PC->GetPawn()))
		{
			CachedInventory = Character->InventoryComponent;
			if (CachedInventory)
			{
				CachedInventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);
			}
		}
	}

	ClearSelection();
	SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryWidget::CreateSlotWidgets()
{
	if (!InventoryGrid || !SlotWidgetClass) return;

	InventoryGrid->ClearChildren();
	SlotWidgets.Empty();

	int32 TotalSlots = GridColumns * GridRows;
	for (int32 i = 0; i < TotalSlots; ++i)
	{
		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
		if (SlotWidget)
		{
			int32 Row = i / GridColumns;
			int32 Col = i % GridColumns;

			InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);
			SlotWidget->OnSlotClicked.AddDynamic(this, &UInventoryWidget::HandleSlotClicked);
			SlotWidgets.Add(SlotWidget);
		}
	}
}

void UInventoryWidget::RefreshInventory()
{
	UpdateSlotWidgets();
}

void UInventoryWidget::UpdateSlotWidgets()
{
	if (!CachedInventory) return;

	// Clear all slots first
	for (UInventorySlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->Clear();
		}
	}

	// Fill with inventory data
	TArray<FInventorySlot> Items = CachedInventory->GetAllItems();
	for (const FInventorySlot& Item : Items)
	{
		if (Item.SlotIndex >= 0 && Item.SlotIndex < SlotWidgets.Num())
		{
			if (UInventorySlotWidget* SlotWidget = SlotWidgets[Item.SlotIndex])
			{
				SlotWidget->SetSlotData(Item);
			}
		}
	}
}

void UInventoryWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsShown = true;
	RefreshInventory();

	// Set input mode to UI
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UInventoryWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
	bIsShown = false;
	ClearSelection();

	// Restore game input
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UInventoryWidget::Toggle()
{
	if (bIsShown)
	{
		Hide();
	}
	else
	{
		Show();
	}
}

void UInventoryWidget::HandleCloseClicked()
{
	Hide();
}

void UInventoryWidget::HandleUseClicked()
{
	if (SelectedSlotIndex < 0 || !CachedInventory) return;

	FInventorySlot Slot = CachedInventory->GetItemAtSlot(SelectedSlotIndex);
	if (!Slot.IsEmpty())
	{
		CachedInventory->UseItem(Slot.EntryId);
	}
}

void UInventoryWidget::HandleDropClicked()
{
	if (SelectedSlotIndex < 0 || !CachedInventory) return;

	FInventorySlot Slot = CachedInventory->GetItemAtSlot(SelectedSlotIndex);
	if (!Slot.IsEmpty())
	{
		CachedInventory->RemoveItem(Slot.EntryId, 1);
		ClearSelection();
	}
}

void UInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
	SelectSlot(SlotIndex);
}

void UInventoryWidget::OnInventoryChanged()
{
	RefreshInventory();
}

void UInventoryWidget::SelectSlot(int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;

	if (!CachedInventory) return;

	FInventorySlot Slot = CachedInventory->GetItemAtSlot(SlotIndex);

	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromString(Slot.IsEmpty() ? TEXT("") : Slot.DisplayName));
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(FText::FromString(Slot.IsEmpty() ? TEXT("") : TEXT("Item description here")));
	}

	if (UseButton)
	{
		UseButton->SetIsEnabled(!Slot.IsEmpty() && Slot.ItemType == TEXT("consumable"));
	}

	if (DropButton)
	{
		DropButton->SetIsEnabled(!Slot.IsEmpty());
	}
}

void UInventoryWidget::ClearSelection()
{
	SelectedSlotIndex = -1;

	if (ItemNameText)
	{
		ItemNameText->SetText(FText::GetEmpty());
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(FText::GetEmpty());
	}

	if (UseButton)
	{
		UseButton->SetIsEnabled(false);
	}

	if (DropButton)
	{
		DropButton->SetIsEnabled(false);
	}
}
