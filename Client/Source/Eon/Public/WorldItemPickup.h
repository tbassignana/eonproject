// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AWorldItemPickup - Collectible item actor in the game world
 *
 * This actor:
 * - Represents items that can be picked up
 * - Syncs with SpaceTimeDB world_item table
 * - Handles interaction and collection
 */
UCLASS()
class EON_API AWorldItemPickup : public AActor
{
    GENERATED_BODY()

public:
    AWorldItemPickup();

    // SpaceTimeDB identifiers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    int64 WorldItemId = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    int64 ItemDefId = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    int32 Quantity = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    FString ItemName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    bool bIsCollected = false;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemMesh;

    // Interaction range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float InteractionRadius = 100.0f;

    // Visual settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float BobSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float BobHeight = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float RotationSpeed = 45.0f;

    // Initialize from SpaceTimeDB data
    UFUNCTION(BlueprintCallable, Category = "Item")
    void InitFromWorldItem(int64 InWorldItemId, int64 InItemDefId, int32 InQuantity, const FString& InItemName);

    // Called when player interacts
    UFUNCTION(BlueprintCallable, Category = "Item")
    void Collect(AActor* Collector);

    // Called when item is marked collected on server
    UFUNCTION(BlueprintCallable, Category = "Item")
    void MarkCollected();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Overlap event handlers
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                       bool bFromSweep, const FHitResult& SweepResult);

private:
    FVector OriginalLocation;
    float BobTime = 0.0f;
};
