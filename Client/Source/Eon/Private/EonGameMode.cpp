// Copyright 2026 tbassignana. MIT License.

#include "EonGameMode.h"
#include "EonCharacter.h"
#include "EonPlayerController.h"

AEonGameMode::AEonGameMode()
{
	DefaultPawnClass = AEonCharacter::StaticClass();
	PlayerControllerClass = AEonPlayerController::StaticClass();
}

void AEonGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogTemp, Log, TEXT("EonGameMode: InitGame - Map: %s"), *MapName);
}

void AEonGameMode::StartPlay()
{
	Super::StartPlay();
	UE_LOG(LogTemp, Log, TEXT("EonGameMode: StartPlay"));
}
