// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerSyncComponent.generated.h"

/**
 * FPlayerSnapshot - A snapshot of player state at a point in time
 */
USTRUCT(BlueprintType)
struct FPlayerSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly)
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly)
    double Timestamp = 0.0;

    bool IsValid() const { return Timestamp > 0.0; }
};

/**
 * UPlayerSyncComponent - Handles network synchronization for player characters
 *
 * This component:
 * - Stores position/rotation snapshots from server
 * - Interpolates between snapshots for smooth remote player movement
 * - Handles client-side prediction for local player
 * - Reconciles local state with server authority
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EON_API UPlayerSyncComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerSyncComponent();

    // Sync configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
    float InterpolationDelay = 0.1f; // 100ms delay for smooth interpolation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
    float MaxExtrapolationTime = 0.25f; // Max time to extrapolate before snapping

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
    float SnapThreshold = 500.0f; // Distance threshold to snap instead of interpolate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
    int32 MaxStoredSnapshots = 32;

    // Identity
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sync")
    FString PlayerIdentity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sync")
    bool bIsLocalPlayer = false;

    // Current interpolated state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sync")
    FPlayerSnapshot InterpolatedState;

    // Called when a new snapshot arrives from the server
    UFUNCTION(BlueprintCallable, Category = "Sync")
    void ReceiveSnapshot(const FPlayerSnapshot& Snapshot);

    // Get the interpolated position for rendering
    UFUNCTION(BlueprintPure, Category = "Sync")
    FVector GetInterpolatedPosition() const;

    // Get the interpolated rotation for rendering
    UFUNCTION(BlueprintPure, Category = "Sync")
    FRotator GetInterpolatedRotation() const;

    // For local player: record input for prediction
    UFUNCTION(BlueprintCallable, Category = "Sync")
    void RecordLocalInput(const FVector& InputVelocity, const FRotator& InputRotation);

    // For local player: reconcile with server state
    UFUNCTION(BlueprintCallable, Category = "Sync")
    void ReconcileWithServer(const FPlayerSnapshot& ServerState);

    // Initialize as local or remote player
    UFUNCTION(BlueprintCallable, Category = "Sync")
    void SetupAsLocalPlayer(const FString& Identity);

    UFUNCTION(BlueprintCallable, Category = "Sync")
    void SetupAsRemotePlayer(const FString& Identity);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Snapshot buffer for interpolation
    TArray<FPlayerSnapshot> SnapshotBuffer;

    // Client prediction state
    TArray<FVector> PredictedInputs;
    int32 LastAcknowledgedInputIndex = 0;

    // Interpolation helpers
    void UpdateInterpolation(float DeltaTime);
    FPlayerSnapshot InterpolateSnapshots(const FPlayerSnapshot& From, const FPlayerSnapshot& To, float Alpha) const;
    int32 FindSnapshotIndexForTime(double TargetTime) const;

    // Server time estimation
    double EstimatedServerTime = 0.0;
    double LocalTimeOffset = 0.0;
};
