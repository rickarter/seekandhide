// Copyright Epic Games, Inc. All Rights Reserved.

#include "seekandhideGameMode.h"
#include "seekandhideCharacter.h"
#include "UObject/ConstructorHelpers.h"

AseekandhideGameMode::AseekandhideGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
