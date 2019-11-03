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
#include "Engine.h"

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

	LeftHandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left"));
	LeftHandMesh->SetupAttachment(RootComponent);
	RightHandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right"));
	RightHandMesh->SetupAttachment(RootComponent);



	BaseTurnRate = 45.0f;
	BaseLookupAtRate = 45.0f;
	BaseDodgeMultiplier = 50.0f;
	TraceDistance = 2000.0f;

	CanDash = true;
	DashDistance = 1500.0f;
	DashCoolDown = 0.0000001f;
	DashStop = 0.5f;

	WalkUpDistance = 1000.f;
}

void ATDRCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	USkeletalMeshComponent* Comp = GetMesh();
	Comp->OnComponentBeginOverlap.AddDynamic(this, &ATDRCharacterBase::OnOverlapBegin);
	Comp->OnComponentEndOverlap.AddDynamic(this, &ATDRCharacterBase::OnOverlapEnd);
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

void ATDRCharacterBase::Dodge(MovementType direction)
{
	if (Debug)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("Character: Dodging somewhere"));

	LaunchCharacterForDash(direction);

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

void ATDRCharacterBase::StopCurrentAnimation()
{
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
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
	TraceForward();

	//When walking sideways, we want to keep the actor on the z axis location
	if (bWalkingSideWays && bWallWalking)
	{
		FVector location = GetActorLocation();
		location.Z = zValue;
		SetActorLocation(location);
	}
}

void ATDRCharacterBase::Jump()
{
	if (bWallTouching && aWallTouched != NULL)
	{
		//angle to detect if we are going to walk up or walk sideways
		float angle = FMath::Acos(FVector::DotProduct(aWallTouched->GetActorRightVector(), RootComponent->GetForwardVector()));

		//Since we are about to be launched somewhere, setting friction to 0 so that we dont slowed down by anything, as the speed shold be constant
		GetCharacterMovement()->BrakingFrictionFactor = 0.f;
		bWallWalking = true;


		//Character has to be rotateed on axis based  on which way it will wall walk
		if (angle > 2.7 && angle < 3.3)
		{
			fRotationXForWallWalk = 85;
			LaunchCharacter(FVector(0, 0, CameraComp->GetUpVector().Z).GetSafeNormal() * WalkUpDistance, true, true);
		}
		else
		{
			bWalkingSideWays = true;

			//When we walk sideways, we want to be able to move up the wall on z the z axis first istead of touching the wall, this takes care of that as well as the code in the tick method
			FVector location = GetActorLocation();
			location.Z += 300;
			zValue = location.Z;
			SetActorLocation(location);

			if (bLeftArmTouchingWall)
			{
				fRotationZforWallWalk = -85;
				LaunchCharacter(FVector(0, -(CameraComp->GetRightVector().Y), 0).GetSafeNormal() * WalkUpDistance, true, true);
			}
			else
			{
				fRotationZforWallWalk = 85;
				LaunchCharacter(FVector(0, (CameraComp->GetRightVector().Y), 0).GetSafeNormal() * WalkUpDistance, true, true);
			}

		}
		FQuat QuatRotation = FQuat(FRotator(fRotationXForWallWalk, 0, fRotationZforWallWalk));
		RootComponent->AddLocalRotation(QuatRotation, true, 0, ETeleportType::None);
		GetMesh()->PlayAnimation(WallWalkingAnim, true);
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATDRCharacterBase::TurnBack, 2.f, false);
	}
	else
	{
		Super::Jump();
	}
}

void ATDRCharacterBase::TurnBack()
{
	bWallWalking = false;
	bWalkingSideWays = false;

	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	FQuat QuatRotation = FQuat(FRotator(-fRotationXForWallWalk, 0, -fRotationZforWallWalk));
	RootComponent->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	fRotationXForWallWalk = 0;
	fRotationZforWallWalk = 0;
}

// Called to bind functionality to input
void ATDRCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATDRCharacterBase::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeRight", IE_DoubleClick, this, &ATDRCharacterBase::Dodge, right);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeLeft", IE_DoubleClick, this, &ATDRCharacterBase::Dodge, left);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeForward", IE_DoubleClick, this, &ATDRCharacterBase::Dodge, forward);
	PlayerInputComponent->BindAction<DirectionDelagate>("DodgeBackward", IE_DoubleClick, this, &ATDRCharacterBase::Dodge, backward);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATDRCharacterBase::InteractPressed);


	PlayerInputComponent->BindAxis("MoveForward", this, &ATDRCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDRCharacterBase::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &ATDRCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &ATDRCharacterBase::LookUpAtRate);
}

#pragma region Helper methods

void ATDRCharacterBase::StartAnimationAndEndWithIddle(UAnimSequence* StartAnimation)
{
	if (StartAnimation)
	{
		bool bLoop = false;
		GetMesh()->PlayAnimation(StartAnimation, bLoop);
		float AnimationLength = StartAnimation->SequenceLength / StartAnimation->RateScale;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATDRCharacterBase::StopCurrentAnimation, AnimationLength, false);
	}
}

void ATDRCharacterBase::LaunchCharacterForDash(MovementType type)
{
	if (CanDash)
	{
		GetCharacterMovement()->BrakingFrictionFactor = 0.f;
		switch (type)
		{
		case left:
			StartAnimationAndEndWithIddle(DodgeLeftAnim);
			LaunchCharacter(FVector(-(CameraComp->GetRightVector().X), -(CameraComp->GetRightVector().Y), 0).GetSafeNormal() * DashDistance, true, true);
			break;
		case right:
			StartAnimationAndEndWithIddle(DodgeRightAnim);
			LaunchCharacter(FVector(CameraComp->GetRightVector().X, CameraComp->GetRightVector().Y, 0).GetSafeNormal() * DashDistance, true, true);
			break;
		case forward:
			StartAnimationAndEndWithIddle(DodgeForwardAnim);
			LaunchCharacter(FVector(CameraComp->GetForwardVector().X, CameraComp->GetForwardVector().Y, 0).GetSafeNormal() * DashDistance, true, true);
			break;
		case backward:
			StartAnimationAndEndWithIddle(DodgeBackwardAnim);
			LaunchCharacter(FVector(-(CameraComp->GetForwardVector().X), -(CameraComp->GetForwardVector().Y), 0).GetSafeNormal() * DashDistance, true, true);
			break;
		}

		CanDash = false;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ATDRCharacterBase::StopDashing, DashStop, false);
	}
}

void ATDRCharacterBase::StopDashing()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ATDRCharacterBase::ResetDash, DashCoolDown, false);
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
}

void ATDRCharacterBase::ResetDash()
{
	CanDash = true;
}
#pragma endregion methods
