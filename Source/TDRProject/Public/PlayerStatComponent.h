// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TDRPROJECT_API UPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatComponent();
	void AddHealth(float Value);
	float GetHealth();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(Replicated)
		float Health;		
};
