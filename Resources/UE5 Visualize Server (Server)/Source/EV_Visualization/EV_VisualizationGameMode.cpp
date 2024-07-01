// Copyright Epic Games, Inc. All Rights Reserved.

#include "EV_VisualizationGameMode.h"
#include "EV_VisualizationCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEV_VisualizationGameMode::AEV_VisualizationGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
