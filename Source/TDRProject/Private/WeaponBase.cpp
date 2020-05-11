// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshCompoent");
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponBase::Fire()
{

}

