// Fill out your copyright notice in the Description page of Project Settings.


#include "TDRCharacterBase.h"
#include "TDRModeBase.h"
#include "PlayerStatComponent.h"
#include "WeaponBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"
#include "InteractInterface.h"
#include "Animation/AnimSequence.h"
#include "Engine.h"
#include "LineTrace.h"
#include "UnrealNetwork.h"


// Sets default values
ATDRCharacterBase::ATDRCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = 1;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	BaseTurnRate = 45.0f;
	BaseLookupAtRate = 45.0f;		
	PlayerStatComp = CreateDefaultSubobject<UPlayerStatComponent>("PlayertStatComponent");
	LineTraceComp = CreateDefaultSubobject<ULineTrace>("LineTraceComponent");
}

DodgeDirection ATDRCharacterBase::GetDash()
{
	return DashingDirection;
}

bool ATDRCharacterBase::GetAtack()
{
	if (bAtacking) {
		UE_LOG(LogTemp, Warning, TEXT("true"));
	}		
	else
		UE_LOG(LogTemp, Warning, TEXT("false"));
	return bAtacking;
}

void ATDRCharacterBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ATDRCharacterBase, DashingDirection);
	DOREPLIFETIME(ATDRCharacterBase, bAtacking);
	DOREPLIFETIME(ATDRCharacterBase, bBlocking);
}

void ATDRCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	if (WeaponClass)
	{
		FTransform WeaponTransform;
		WeaponTransform.SetLocation(FVector::ZeroVector);
		WeaponTransform.SetRotation(FQuat(FRotator::ZeroRotator));
		Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, WeaponTransform, SpawnParams);		
		if (Weapon)
		{
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_R"));
		}
	}			
}

void ATDRCharacterBase::MoveForward(float Value)
{
	if ((Controller) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATDRCharacterBase::MoveRight(float Value)
{

	if ((Controller) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATDRCharacterBase::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATDRCharacterBase::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookupAtRate * GetWorld()->GetDeltaSeconds());
}

void ATDRCharacterBase::AddControllerYawInput(float value)
{
	
		Super::AddControllerYawInput(value);
}

void ATDRCharacterBase::AddControllerPitchInput(float value)
{
	
		Super::AddControllerPitchInput(value);
}
// Called to bind functionality to input
void ATDRCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Atack", IE_Pressed, this, &ATDRCharacterBase::Atack);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATDRCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDRCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &ATDRCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &ATDRCharacterBase::LookUpAtRate);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeRight", IE_DoubleClick, this, &ATDRCharacterBase::Dash, DodgeDirection::Right);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeLeft", IE_DoubleClick, this, &ATDRCharacterBase::Dash, DodgeDirection::Left);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeForward", IE_DoubleClick, this, &ATDRCharacterBase::Dash, DodgeDirection::Forward);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeBackward", IE_DoubleClick, this, &ATDRCharacterBase::Dash, DodgeDirection::Backward);
	PlayerInputComponent->BindAction("Test", IE_Released, this, &ACharacter::Jump);
}

void ATDRCharacterBase::Dash(DodgeDirection Direction)
{	
	if ((Controller != NULL))
	{		
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerDash(Direction);
		}
	}
}

bool ATDRCharacterBase::ServerDash_Validate(DodgeDirection Direction)
{
	return true;
}

void ATDRCharacterBase::ServerDash_Implementation(DodgeDirection Direction)
{
	if (GetLocalRole() == ROLE_Authority)
	{				
		//PlayerStatComp->AddHealth(20);
		DashingDirection = Direction;
		if (Direction == DodgeDirection::Right)
		{			
			LaunchCharacter(FVector(GetActorRightVector().X, GetActorRightVector().Y, 0).GetSafeNormal() * 1000.f, true, true);
		}
		else if (Direction == DodgeDirection::Left)
		{			
			LaunchCharacter(FVector(-(GetActorRightVector().X), -(GetActorRightVector().Y), 0).GetSafeNormal() * 1000.f, true, true);
		}
		else if (Direction == DodgeDirection::Forward)
		{			
			LaunchCharacter(FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0).GetSafeNormal() * 1000.f, true, true);
		}
		else
		{
			LaunchCharacter(FVector(-(GetActorForwardVector().X), -(GetActorForwardVector().Y), 0).GetSafeNormal() * 1000.f, true, true);
		}				

		GetWorld()->GetTimerManager().SetTimer(DashHandle, this, &ATDRCharacterBase::StopDash, 0.5f, false);
	}
}

void ATDRCharacterBase::StopDash()
{
	DashingDirection = DodgeDirection::Nothing;
	bAtacking = false;;
}

void ATDRCharacterBase::Atack()
{	
	bAtacking = true;
	
	FVector Start = GetMesh()->GetBoneLocation(FName("Head"));
	FVector End = Start + CameraComp->GetForwardVector() * 1500.0f;

	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);
	
	if (Role < ROLE_Authority)
	{
		ServerAtack();
	}
	else
	{
		if (AActor* Actor = HitResult.GetActor())
		{
			FString name = Actor->GetName();

			if (ATDRCharacterBase* Character = Cast<ATDRCharacterBase>(Actor))
			{


				float testDamage = -20;
				Character->TakeDamage(testDamage, FDamageEvent(), GetController(), this);
				if (Character->PlayerStatComp->GetHealth() == 0.f)
				{
					Character->Die();
				}

				if (bAtacking) {
					UE_LOG(LogTemp, Warning, TEXT("true"));
				}
				else
					UE_LOG(LogTemp, Warning, TEXT("false"));
			}
		}
	}
	FTimerHandle test;
	GetWorld()->GetTimerManager().SetTimer(test, this, &ATDRCharacterBase::StopDash, 0.5f, false);
	//bAtacking = false;
	
}

bool ATDRCharacterBase::ServerAtack_Validate()
{
	return true;
}

void ATDRCharacterBase::ServerAtack_Implementation()
{
	Atack();
}

void ATDRCharacterBase::Die()
{	
	if (Role == ROLE_Authority)
	{
		MultiDie();
		AGameModeBase* GM = GetWorld()->GetAuthGameMode();
		if (ATDRModeBase* GameMode = Cast<ATDRModeBase>(GM))
		{
			GameMode->Respawn(GetController());
		}
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &ATDRCharacterBase::DestroyChar, 10.5f, false);		
	}
}
//
bool ATDRCharacterBase::MultiDie_Validate()
{
	return true;
}

void ATDRCharacterBase::MultiDie_Implementation()
{
	GetCapsuleComponent()->DestroyComponent();
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetAllBodiesSimulatePhysics(true);
}

void ATDRCharacterBase::DestroyChar()
{
	Destroy();
}



FString ATDRCharacterBase::GetInformation()
{
	FString retString = "Health : " + FString::SanitizeFloat(PlayerStatComp->GetHealth());
	return retString;
}

float ATDRCharacterBase::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (Role == ROLE_Authority)
	{		
		PlayerStatComp->AddHealth(ActualDamage);	
	}
	return ActualDamage;;	
}