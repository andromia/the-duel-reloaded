// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class TDRPROJECT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* MeshComp;

public:
	void Fire();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};
