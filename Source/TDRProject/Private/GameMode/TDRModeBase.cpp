// Fill out your copyright notice in the Description page of Project Settings.

#include "TDRModeBase.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"
#include "UObject/ConstructorHelpers.h"

ATDRModeBase::ATDRModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_CharacterBase"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}