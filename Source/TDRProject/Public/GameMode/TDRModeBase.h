// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDRModeBase.generated.h"

/**
 *
 */
UCLASS()
class TDRPROJECT_API ATDRModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	TArray<class ASpawnPoints*> SpawnPoints;


protected:
	UFUNCTION()
	void Spawn(AController* Controller);
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	ATDRModeBase();
	void Respawn(AController* Controller);
	virtual void BeginPlay() override;

};
