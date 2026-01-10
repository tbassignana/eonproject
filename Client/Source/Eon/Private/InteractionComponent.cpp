// Copyright 2026 tbassignana. MIT License.

#include "InteractionComponent.h"
#include "InteractableInterface.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastScan += DeltaTime;
	if (TimeSinceLastScan >= ScanInterval)
	{
		ScanForInteractables();
		TimeSinceLastScan = 0.0f;
	}
}

void UInteractionComponent::TryInteract()
{
	if (!CurrentInteractable) return;

	// Check if the actor implements the interactable interface
	if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		IInteractableInterface* Interactable = Cast<IInteractableInterface>(CurrentInteractable);
		if (Interactable && Interactable->CanInteract(GetOwner()))
		{
			Interactable->OnInteract(GetOwner());
			OnInteractionComplete.Broadcast(CurrentInteractable);
		}
	}
}

FString UInteractionComponent::GetInteractionPrompt() const
{
	if (!CurrentInteractable) return FString();

	if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		IInteractableInterface* Interactable = Cast<IInteractableInterface>(CurrentInteractable);
		if (Interactable)
		{
			return Interactable->GetInteractionPrompt();
		}
	}

	return TEXT("Interact");
}

void UInteractionComponent::ScanForInteractables()
{
	AActor* NewInteractable = FindBestInteractable();

	if (NewInteractable != CurrentInteractable)
	{
		if (CurrentInteractable && !NewInteractable)
		{
			// Lost interactable
			if (CurrentInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(CurrentInteractable))
				{
					Interactable->OnEndFocus(GetOwner());
				}
			}
			OnInteractableLost.Broadcast();
		}
		else if (NewInteractable)
		{
			// Found new interactable
			if (NewInteractable->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				if (IInteractableInterface* Interactable = Cast<IInteractableInterface>(NewInteractable))
				{
					Interactable->OnBeginFocus(GetOwner());
				}
			}
			OnInteractableFound.Broadcast(NewInteractable);
		}

		CurrentInteractable = NewInteractable;
	}
}

AActor* UInteractionComponent::FindBestInteractable() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	FVector OwnerLocation = Owner->GetActorLocation();
	FVector OwnerForward = Owner->GetActorForwardVector();

	// Sphere overlap for nearby interactables
	TArray<FOverlapResult> Overlaps;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(InteractionRange);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	World->OverlapMultiByChannel(
		Overlaps,
		OwnerLocation,
		FQuat::Identity,
		InteractionChannel,
		SphereShape,
		QueryParams
	);

	// Find the best candidate (closest in front of player)
	AActor* BestCandidate = nullptr;
	float BestScore = -1.0f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor) continue;

		// Must implement interactable interface
		if (!Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			continue;
		}

		IInteractableInterface* Interactable = Cast<IInteractableInterface>(Actor);
		if (!Interactable || !Interactable->CanInteract(Owner))
		{
			continue;
		}

		// Calculate score based on distance and facing direction
		FVector ToActor = Actor->GetActorLocation() - OwnerLocation;
		float Distance = ToActor.Size();
		ToActor.Normalize();

		float DotProduct = FVector::DotProduct(OwnerForward, ToActor);
		float Score = DotProduct * (1.0f - Distance / InteractionRange);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestCandidate = Actor;
		}
	}

	return BestCandidate;
}
