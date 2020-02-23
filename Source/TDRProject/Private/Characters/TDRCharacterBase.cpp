// Fill out your copyright notice in the Description page of Project Settings.


#include "TDRCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"
#include "InteractInterface.h"
#include "Animation/AnimSequence.h"
#include "TDRCharacterMovementComponent.h"
#include "Engine.h"
#include "UnrealNetwork.h"


// Sets default values
ATDRCharacterBase::ATDRCharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UTDRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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
	BaseDodgeMultiplier = 50.0f;
	TraceDistance = 2000.0f;
	bDashing = false;

	MyCharacterMovementComponent = Cast<UTDRCharacterMovementComponent>(GetMovementComponent());
}

void ATDRCharacterBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ATDRCharacterBase, bDashing);
}

void ATDRCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	USkeletalMeshComponent* Comp = GetMesh();
	Comp->OnComponentBeginOverlap.AddDynamic(this, &ATDRCharacterBase::OnOverlapBegin);
	Comp->OnComponentEndOverlap.AddDynamic(this, &ATDRCharacterBase::OnOverlapEnd);
}

void ATDRCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UTDRCharacterMovementComponent>(Super::GetMovementComponent());
}

void ATDRCharacterBase::MoveForward(float Value)
{
	if (MyCharacterMovementComponent->bWallWalking)
		return;
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
	if (MyCharacterMovementComponent->bWallWalking)
		return;
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

void ATDRCharacterBase::Dodge()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("Character: Dodging somewhere"));
	MyCharacterMovementComponent->Dodge();
}

void ATDRCharacterBase::Dash(int movement)
{
	if ((Controller != NULL))
	{
		if (Role < ROLE_Authority)
		{
			ServerDash(movement);
		}
	}
}

bool ATDRCharacterBase::GetDash()
{
	return bDashing;
}

void ATDRCharacterBase::StopDash()
{
	bDashing = false;
}

void ATDRCharacterBase::InteractPressed()
{
	TraceForward();
	if (FocusedActor)
	{
		IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
		if (Interface)
		{
			Interface->Execute_OnInteract(FocusedActor, this);
		}
	}
}

void ATDRCharacterBase::TraceForward_Implementation()
{
	FVector Loc;
	FRotator Rot;
	FHitResult Hit;

	GetController()->GetPlayerViewPoint(Loc, Rot);

	FVector Start = Loc;
	FVector End = Start + (Rot.Vector() * TraceDistance);

	FCollisionQueryParams TraceParams;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	if (Debug)
		DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 2.0f);


	/*
	* Casting is costly. This will sometimes import all of the class dependancies. Using blueprints for this
	* supposedly avoids casting, thus making it more performant. If applicable, use this for character-to-character
	* interaction, unless character casting is not as costly as more dynamic casing.
	*/
	if (bHit)
	{
		if (Debug)
			DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), FColor::Emerald, false, 2.0f);

		AActor* Interactable = Hit.GetActor();

		if (Interactable)
		{
			if (Interactable != FocusedActor)
			{
				if (FocusedActor)
				{
					IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
					if (Interface)
					{
						Interface->Execute_EndFocus(FocusedActor);
					}
				}
				IInteractInterface* Interface = Cast<IInteractInterface>(Interactable);
				if (Interface)
				{
					Interface->Execute_StartFocus(Interactable);
				}
				FocusedActor = Interactable;
			}
		}
		else
		{
			if (FocusedActor)
			{
				IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
				if (Interface)
				{
					Interface->Execute_EndFocus(FocusedActor);
				}
			}

			FocusedActor = nullptr;
		}
	}
}

void ATDRCharacterBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString TheString = OtherActor->GetName();
	if (OtherActor->GetName().Contains("Wall"))
	{
		float leftHAndDistance = GetMesh()->GetBoneLocation("hand_l").GetAbs().X - OtherActor->GetActorLocation().GetAbs().X;
		float righHAndDistance = GetMesh()->GetBoneLocation("hand_r").GetAbs().X - OtherActor->GetActorLocation().GetAbs().X;
		aWallTouched = OtherActor;

		if (abs(leftHAndDistance) > abs(righHAndDistance))
		{
			bRightArmTouchingWall = false;
			bLeftArmTouchingWall = true;

		}
		else
		{
			bRightArmTouchingWall = true;
			bLeftArmTouchingWall = false;
		}

		bWallTouching = true;
	}

	IInteractInterface* Interface = Cast<IInteractInterface>(OtherActor);
	if (Interface)
	{
		Interface->Execute_OnInteract(OtherActor, this);
	}
}

void ATDRCharacterBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->GetName().Contains("Wall"))
	{
		bWallTouching = false;
		aWallTouched = NULL;
	}

}

void ATDRCharacterBase::Tick(float DeltaTime)
{
	
}


void ATDRCharacterBase::Jump()
{
	if (bWallTouching && aWallTouched != NULL)
	{
		//angle to detect if we are going to walk up or walk sideways
		float angle = FMath::Acos(FVector::DotProduct(aWallTouched->GetActorRightVector(), RootComponent->GetForwardVector()));

		//Since we are about to be launched somewhere, setting friction to 0 so that we dont slowed down by anything, as the speed shold be constant
		GetCharacterMovement()->BrakingFrictionFactor = 0.f;

		//Character has to be rotateed on axis based  on which way it will wall walk
		if (angle > 2.7 && angle < 3.3)
		{
			MyCharacterMovementComponent->Walkup();
		}
		else
		{
			MyCharacterMovementComponent->SideWalk();			
		}
	}
	else
	{
		Super::Jump();
	}
}

void ATDRCharacterBase::AddControllerYawInput(float value)
{
	if (!MyCharacterMovementComponent->bWallWalking)
		Super::AddControllerYawInput(value);
}

void ATDRCharacterBase::AddControllerPitchInput(float value)
{
	if (!MyCharacterMovementComponent->bWallWalking)
		Super::AddControllerPitchInput(value);
}
// Called to bind functionality to input
void ATDRCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATDRCharacterBase::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("DodgeRight", IE_DoubleClick, this, &ATDRCharacterBase::Dodge);
	PlayerInputComponent->BindAction("DodgeLeft", IE_DoubleClick, this, &ATDRCharacterBase::Dodge);
	PlayerInputComponent->BindAction("DodgeForward", IE_DoubleClick, this, &ATDRCharacterBase::Dodge);
	PlayerInputComponent->BindAction("DodgeBackward", IE_DoubleClick, this, &ATDRCharacterBase::Dodge);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATDRCharacterBase::InteractPressed);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATDRCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDRCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &ATDRCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &ATDRCharacterBase::LookUpAtRate);
}

bool ATDRCharacterBase::ServerDash_Validate(int movement)
{
	
	return true;
}

void ATDRCharacterBase::ServerDash_Implementation(int movement)
{
	if (Role == ROLE_Authority)
	{
		//0 -> right
		//1 -> left
		//2 -> forward
		//3 -> backward
		
		bDashing = true;
		if (movement == 0)
		{
			LaunchCharacter(FVector(GetActorRightVector().X ,GetActorRightVector().Y, 0).GetSafeNormal() * 1000.f, true, true);
		}
		else if (movement == 1)
		{
			LaunchCharacter(FVector(-(GetActorRightVector().X), -(GetActorRightVector().Y), 0).GetSafeNormal() * 1000.f, true, true);
		}
		else if (movement == 2)
		{
			LaunchCharacter(FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0).GetSafeNormal() * 1000.f, true, true);
		}
		else
		{
			LaunchCharacter(FVector(-(GetActorForwardVector().X), -(GetActorForwardVector().Y), 0).GetSafeNormal() * 1000.f, true, true);
		}
		
		if (bDashing)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, "true");
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, "false");
		}
		GetWorld()->GetTimerManager().SetTimer(DashHandle, this, &ATDRCharacterBase::StopDash, 0.5f, false);
	}
}