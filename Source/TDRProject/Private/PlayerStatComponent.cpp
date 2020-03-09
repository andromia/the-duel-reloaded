// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStatComponent.h"
#include "Net\UnrealNetwork.h"

// Sets default values for this component's properties
UPlayerStatComponent::UPlayerStatComponent()
{	
	Health = 100.0f;
}

void UPlayerStatComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone	
	DOREPLIFETIME(UPlayerStatComponent, Health);
}

// Called when the game starts
void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);
	// ...
	
}

//this method should only be called by the server... 
void UPlayerStatComponent::AddHealth(float Value)
{
	if(GetOwnerRole() == ROLE_Authority)
		Health += Value;
}

float UPlayerStatComponent::GetHealth()
{
	return Health;
}

