// Copyright 2026 tbassignana. MIT License.

#include "InteractionComponent.h"
#include "InteractableInterface.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"

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

	// Use sphere trace to find nearby actors
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(Owner);

	TArray<FHitResult> HitResults;
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		World,
		OwnerLocation,
		OwnerLocation + FVector(0, 0, 1), // Minimal trace distance
		InteractionRange,
		UEngineTypes::ConvertToTraceType(InteractionChannel),
		false,
		IgnoredActors,
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	if (!bHit) return nullptr;

	AActor* BestCandidate = nullptr;
	float BestScore = -1.0f;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* Actor = Hit.GetActor();
		if (!Actor) continue;

		if (!Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			continue;
		}

		IInteractableInterface* Interactable = Cast<IInteractableInterface>(Actor);
		if (!Interactable || !Interactable->CanInteract(Owner))
		{
			continue;
		}

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
