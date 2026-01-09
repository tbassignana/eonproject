// Copyright 2026 Eon Project. All rights reserved.

#include "EonGameMode.h"
#include "EonCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEonGameMode::AEonGameMode()
{
    // Set default pawn class to our character class
    DefaultPawnClass = AEonCharacter::StaticClass();
}
