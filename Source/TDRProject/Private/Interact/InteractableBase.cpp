// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableBase.h"
#include "..\..\Public\Interact\InteractableBase.h"

// Sets default values
AInteractableBase::AInteractableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableBase::OnInteract_Implementation(AActor* Caller)
{
	Destroy();
}

void AInteractableBase::StartFocus_Implementation()
{
	if(Debug)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("Interactable: Start Focus"));
}

void AInteractableBase::EndFocus_Implementation()
{
	if (Debug)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("Interactable: End Focus"));
}

