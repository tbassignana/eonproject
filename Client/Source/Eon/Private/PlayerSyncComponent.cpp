// Copyright 2026 Eon Project. All rights reserved.

#include "PlayerSyncComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UPlayerSyncComponent::UPlayerSyncComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerSyncComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize server time estimation
    EstimatedServerTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
}

void UPlayerSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update estimated server time
    EstimatedServerTime += DeltaTime;

    // Update interpolation for remote players
    if (!bIsLocalPlayer)
    {
        UpdateInterpolation(DeltaTime);
    }
}

void UPlayerSyncComponent::SetupAsLocalPlayer(const FString& Identity)
{
    PlayerIdentity = Identity;
    bIsLocalPlayer = true;

    UE_LOG(LogTemp, Log, TEXT("PlayerSyncComponent: Setup as local player (Identity: %s)"), *Identity);
}

void UPlayerSyncComponent::SetupAsRemotePlayer(const FString& Identity)
{
    PlayerIdentity = Identity;
    bIsLocalPlayer = false;

    UE_LOG(LogTemp, Log, TEXT("PlayerSyncComponent: Setup as remote player (Identity: %s)"), *Identity);
}

void UPlayerSyncComponent::ReceiveSnapshot(const FPlayerSnapshot& Snapshot)
{
    // Add snapshot to buffer
    SnapshotBuffer.Add(Snapshot);

    // Limit buffer size
    while (SnapshotBuffer.Num() > MaxStoredSnapshots)
    {
        SnapshotBuffer.RemoveAt(0);
    }

    // Update server time estimate
    if (Snapshot.Timestamp > EstimatedServerTime)
    {
        // Smooth adjustment of time estimate
        double TimeDiff = Snapshot.Timestamp - EstimatedServerTime;
        EstimatedServerTime += TimeDiff * 0.1; // Gradual correction
    }

    UE_LOG(LogTemp, Verbose, TEXT("Received snapshot for %s at time %.3f"),
           *PlayerIdentity, Snapshot.Timestamp);
}

void UPlayerSyncComponent::UpdateInterpolation(float DeltaTime)
{
    if (SnapshotBuffer.Num() < 2)
    {
        // Not enough data to interpolate
        if (SnapshotBuffer.Num() == 1)
        {
            InterpolatedState = SnapshotBuffer[0];
        }
        return;
    }

    // Calculate target render time (slightly in the past for interpolation buffer)
    double RenderTime = EstimatedServerTime - InterpolationDelay;

    // Find the two snapshots to interpolate between
    int32 FromIndex = FindSnapshotIndexForTime(RenderTime);

    if (FromIndex < 0)
    {
        // All snapshots are in the future, use oldest
        InterpolatedState = SnapshotBuffer[0];
        return;
    }

    if (FromIndex >= SnapshotBuffer.Num() - 1)
    {
        // Need to extrapolate (no future snapshot)
        const FPlayerSnapshot& Latest = SnapshotBuffer.Last();
        double TimeSinceLatest = RenderTime - Latest.Timestamp;

        if (TimeSinceLatest > MaxExtrapolationTime)
        {
            // Too long since last update, just use latest state
            InterpolatedState = Latest;
        }
        else if (SnapshotBuffer.Num() >= 2)
        {
            // Extrapolate based on velocity
            const FPlayerSnapshot& Previous = SnapshotBuffer[SnapshotBuffer.Num() - 2];
            double DeltaT = Latest.Timestamp - Previous.Timestamp;

            if (DeltaT > 0.001)
            {
                FVector Velocity = (Latest.Position - Previous.Position) / DeltaT;
                InterpolatedState = Latest;
                InterpolatedState.Position = Latest.Position + Velocity * TimeSinceLatest;
            }
            else
            {
                InterpolatedState = Latest;
            }
        }
        else
        {
            InterpolatedState = Latest;
        }
        return;
    }

    // Normal interpolation between two snapshots
    const FPlayerSnapshot& From = SnapshotBuffer[FromIndex];
    const FPlayerSnapshot& To = SnapshotBuffer[FromIndex + 1];

    double DeltaT = To.Timestamp - From.Timestamp;
    if (DeltaT > 0.001)
    {
        float Alpha = FMath::Clamp((RenderTime - From.Timestamp) / DeltaT, 0.0, 1.0);
        InterpolatedState = InterpolateSnapshots(From, To, Alpha);
    }
    else
    {
        InterpolatedState = To;
    }

    // Check for large position jumps (teleport/respawn)
    AActor* Owner = GetOwner();
    if (Owner)
    {
        float Distance = FVector::Dist(Owner->GetActorLocation(), InterpolatedState.Position);
        if (Distance > SnapThreshold)
        {
            // Snap immediately instead of interpolating
            Owner->SetActorLocation(InterpolatedState.Position);
            Owner->SetActorRotation(InterpolatedState.Rotation);
            UE_LOG(LogTemp, Log, TEXT("Snapped player %s to position (distance: %.1f)"),
                   *PlayerIdentity, Distance);
        }
    }

    // Clean up old snapshots we no longer need
    while (SnapshotBuffer.Num() > 2 && SnapshotBuffer[0].Timestamp < RenderTime - 1.0)
    {
        SnapshotBuffer.RemoveAt(0);
    }
}

FPlayerSnapshot UPlayerSyncComponent::InterpolateSnapshots(const FPlayerSnapshot& From, const FPlayerSnapshot& To, float Alpha) const
{
    FPlayerSnapshot Result;

    // Linear interpolation for position
    Result.Position = FMath::Lerp(From.Position, To.Position, Alpha);

    // Spherical interpolation for rotation (shortest path)
    Result.Rotation = FQuat::Slerp(From.Rotation.Quaternion(), To.Rotation.Quaternion(), Alpha).Rotator();

    // Take latest health value
    Result.Health = Alpha > 0.5f ? To.Health : From.Health;

    // Take latest attack state
    Result.bIsAttacking = Alpha > 0.5f ? To.bIsAttacking : From.bIsAttacking;

    // Interpolated timestamp
    Result.Timestamp = FMath::Lerp(From.Timestamp, To.Timestamp, Alpha);

    return Result;
}

int32 UPlayerSyncComponent::FindSnapshotIndexForTime(double TargetTime) const
{
    // Find the latest snapshot that is before or at the target time
    for (int32 i = SnapshotBuffer.Num() - 1; i >= 0; --i)
    {
        if (SnapshotBuffer[i].Timestamp <= TargetTime)
        {
            return i;
        }
    }
    return -1;
}

FVector UPlayerSyncComponent::GetInterpolatedPosition() const
{
    return InterpolatedState.Position;
}

FRotator UPlayerSyncComponent::GetInterpolatedRotation() const
{
    return InterpolatedState.Rotation;
}

void UPlayerSyncComponent::RecordLocalInput(const FVector& InputVelocity, const FRotator& InputRotation)
{
    // For client-side prediction
    // Store the input so we can replay it if server state differs
    PredictedInputs.Add(InputVelocity);

    // Limit stored inputs
    while (PredictedInputs.Num() > 64)
    {
        PredictedInputs.RemoveAt(0);
        LastAcknowledgedInputIndex = FMath::Max(0, LastAcknowledgedInputIndex - 1);
    }
}

void UPlayerSyncComponent::ReconcileWithServer(const FPlayerSnapshot& ServerState)
{
    if (!bIsLocalPlayer)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Compare server state with predicted state
    FVector CurrentPosition = Owner->GetActorLocation();
    float PositionError = FVector::Dist(CurrentPosition, ServerState.Position);

    const float CorrectionThreshold = 10.0f; // Unreal units

    if (PositionError > CorrectionThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("Server reconciliation: Position error %.2f, correcting"),
               PositionError);

        // Option 1: Snap to server position (simple but jarring)
        // Owner->SetActorLocation(ServerState.Position);

        // Option 2: Smooth correction (better UX)
        FVector CorrectedPosition = FMath::Lerp(CurrentPosition, ServerState.Position, 0.3f);
        Owner->SetActorLocation(CorrectedPosition);

        // Clear predicted inputs since we've reconciled
        PredictedInputs.Empty();
    }

    // Update health from server
    // TODO: Update character health from InterpolatedState.Health
}
