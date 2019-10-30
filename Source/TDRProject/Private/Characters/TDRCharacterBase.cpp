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
	SavePoint = this->GetActorLocation();
	x = SavePoint.X;
	y = SavePoint.Y;
	z = SavePoint.Z;
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
		WallTouching = true;
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
		WallTouching = false;
	}

}

void ATDRCharacterBase::Tick(float DeltaTime)
{
	TraceForward();
}

void ATDRCharacterBase::Jump()
{
	if (WallTouching)
	{
		WallWalking = true;
		FRotator NewRotation = FRotator(85, 0, 0);
		FQuat QuatRotation = FQuat(NewRotation);
		RootComponent->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
		GetMesh()->PlayAnimation(WallWalkingAnim, true);

		GetCharacterMovement()->BrakingFrictionFactor = 0.f;
		LaunchCharacter(FVector(0, 0, RootComponent->GetUpVector().Z).GetSafeNormal() * WalkUpDistance, true, true);

		GetWorldTimerManager().SetTimer(TimerHandle, this, &ATDRCharacterBase::TurnBack, 3.f, false);
	}
	else
	{
		Super::Jump();
	}
}

void ATDRCharacterBase::TurnBack()
{
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	FRotator NewRotation = FRotator(-85, 0, 0);
	FQuat QuatRotation = FQuat(NewRotation);
	RootComponent->AddLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	WallWalking = false;
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
