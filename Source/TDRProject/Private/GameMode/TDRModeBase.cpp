// Fill out your copyright notice in the Description page of Project Settings.

#include "TDRModeBase.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Actor.h"
#include "SpawnPoints.h"
#include "Engine/Engine.h"
#include "UnrealMathUtility.h"
#include "TimerManager.h"


ATDRModeBase::ATDRModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_CharacterBase"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}	
}

void ATDRModeBase::BeginPlay()
{
	Super::BeginPlay();

	UClass* SpawnPointer = ASpawnPoints::StaticClass();
	for (TActorIterator<ATargetPoint> Point(GetWorld(), SpawnPointer); Point; ++Point)
	{
		SpawnPoints.Add(Cast<ASpawnPoints>(*Point));
	}

	UE_LOG(LogTemp,Warning, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!! Points %d"), SpawnPoints.Num())
}

void ATDRModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (AController* Controller = Cast<AController>(NewPlayer))
	{
		Spawn(Controller);
	}
}



void ATDRModeBase::Respawn(AController* Controller)
{
	if (Controller && Role == ROLE_Authority)
	{
		FTimerDelegate TimerDele;
		TimerDele.BindUFunction(this, FName("Spawn"), Controller);
		FTimerHandle RespawnHandle;
		GetWorld()->GetTimerManager().SetTimer(RespawnHandle, TimerDele, 3.0f, false);
	}
}

void ATDRModeBase::Spawn(AController* Controller)
{
	if (Role == ROLE_Authority)
	{
		int32 slot = FMath::RandRange(0, SpawnPoints.Num() - 1);
		if (SpawnPoints[slot])
		{
			FVector Location = SpawnPoints[slot]->GetActorLocation();
			FRotator Rotator = SpawnPoints[slot]->GetActorRotation();
			if (APawn* pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotator))
			{
				Controller->Possess(pawn);
			}
		}
	}
}

