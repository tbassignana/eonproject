// Copyright 2026 Eon Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EonGameMode.generated.h"

/**
 * AEonGameMode - Base game mode for the Eon game
 *
 * Handles:
 * - Default pawn class (AEonCharacter)
 * - Player controller class
 * - HUD class
 * - Game rules
 */
UCLASS(minimalapi)
class AEonGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEonGameMode();
};
