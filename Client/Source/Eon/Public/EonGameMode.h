// Copyright 2026 tbassignana. MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EonGameMode.generated.h"

UCLASS()
class EON_API AEonGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEonGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	bool bAutoConnectToSpaceTimeDB = true;
};
