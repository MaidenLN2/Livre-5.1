// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreGameMode.h"
#include "LivreCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALivreGameMode::ALivreGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
