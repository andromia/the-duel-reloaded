// Fill out your copyright notice in the Description page of Project Settings.

#include "..\..\Public\MovementComponent\TDRCharacterMovementComponent.h"
#include "..\..\Public\Characters\TDRCharacterBase.h"

UTDRCharacterMovementComponent::UTDRCharacterMovementComponent(const class FObjectInitializer& ObjectInitialiazer) : Super(ObjectInitialiazer)
{
	DodgeStrength = 10000.f;
}

void UTDRCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	//SetMaxWalkSpeed
	if (bRequestMaxWalkSpeedChange)
	{
		bRequestMaxWalkSpeedChange = false;
		MaxWalkSpeed = MyNewMaxWalkSpeed;
	}

	//Dash

	if (bWantsToDodge)
	{
		MoveDirection.Normalize();
		FVector DodgeVelocity = MoveDirection * DodgeStrength;
		DodgeVelocity.Z = 0.0f;
		Launch(DodgeVelocity);
	}
}

void UTDRCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client side
{
	Super::UpdateFromCompressedFlags(Flags);
	bRequestMaxWalkSpeedChange = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToDodge = (Flags&FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UTDRCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);
	check(PawnOwner->Role < ROLE_Authority);

	if (!ClientPredictionData)
	{
		UTDRCharacterMovementComponent* MutableThis = const_cast<UTDRCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UTDRCharacterMovementComponent::FSavedMove_My::Clear()
{
	Super::Clear();

	bSavedRequestMaxWalkSpeedChange = false;
	bSavedWAntsToDodge = false;
	SavedMoveDirection = FVector::ZeroVector;
}

uint8 UTDRCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMaxWalkSpeedChange)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedWAntsToDodge)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

bool UTDRCharacterMovementComponent::FSavedMove_My::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bSavedRequestMaxWalkSpeedChange != ((FSavedMove_My*)&NewMove)->bSavedRequestMaxWalkSpeedChange)
	{
		return false;
	}

	if (bSavedWAntsToDodge != ((FSavedMove_My*)&NewMove)->bSavedWAntsToDodge)
	{
		return false;
	}

	if (SavedMoveDirection != ((FSavedMove_My*)&NewMove)->SavedMoveDirection)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UTDRCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	UTDRCharacterMovementComponent* CharacterMovement = Cast<UTDRCharacterMovementComponent>(Character->GetCharacterMovement());

	if (CharacterMovement)
	{
		bSavedRequestMaxWalkSpeedChange = CharacterMovement->bRequestMaxWalkSpeedChange;
		bSavedWAntsToDodge = CharacterMovement->bWantsToDodge;
		SavedMoveDirection = CharacterMovement->MoveDirection;
	}
}

void UTDRCharacterMovementComponent::FSavedMove_My::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	UTDRCharacterMovementComponent* CharacterMovement = Cast<UTDRCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->MoveDirection = SavedMoveDirection;
	}
}

UTDRCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr UTDRCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_My());
}

//Set max walk speed to trasfer the current walk speed to the server
bool UTDRCharacterMovementComponent::Server_SetMaxWalkSpeed_Validate(const float NewMaxWalkSpeed)
{
	if (NewMaxWalkSpeed < 0.f || NewMaxWalkSpeed > 2000.f)
		return false;
	else
		return true;
}

void UTDRCharacterMovementComponent::Server_SetMaxWalkSpeed_Implementation(const float NewMaxWalkSpeed)
{
	MyNewMaxWalkSpeed = NewMaxWalkSpeed;
}

void UTDRCharacterMovementComponent::SetMaxWalkSpeed(float NewMaxWalkSpeed)
{
	if (PawnOwner->IsLocallyControlled())
	{
		MyNewMaxWalkSpeed = NewMaxWalkSpeed;
		Server_SetMaxWalkSpeed(NewMaxWalkSpeed);
	}

	bRequestMaxWalkSpeedChange = true;
}

bool UTDRCharacterMovementComponent::Server_MoveDirection_Validate(const FVector& MoveDir)
{
	return true;
}

void UTDRCharacterMovementComponent::Server_MoveDirection_Implementation(const FVector& MoveDir)
{
	MoveDirection = MoveDir;
}

//Trigger dodge
void UTDRCharacterMovementComponent::Dodge(const FVector& MoveDir)
{
	if (PawnOwner->IsLocallyControlled())
	{		
		Server_MoveDirection(MoveDir);
	}	
	bWantsToDodge = true;
}

void UTDRCharacterMovementComponent::StopDodge()
{
	bWantsToDodge = false;
}