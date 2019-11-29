// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TDRCharacterMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class TDRPROJECT_API UTDRCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

		class FSavedMove_My : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		//Dodge
		FVector SavedMoveDirection;
		uint8 bSavedWAntsToDodge : 1;
		uint8 bSavedWalkUp : 1;
		uint8 bSavedWalkSideWays : 1;
	};

	class FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);
		typedef FNetworkPredictionData_Client_Character Super;
		virtual FSavedMovePtr AllocateNewMove() override;
	};

private:
	virtual void UpdateFromCompressedFlags(uint8 flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity);

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_MoveDirection(const FVector& MoveDir);
	
	void EndSideWalk();
	

	FVector MoveDirection;

	uint8 bWantsToDodge : 1;

	uint8 bWalkup : 1;

	uint8 bWalkSideWays : 1;
	FQuat PosToReturn;
	uint8 bEndSideWalk;
	
	float ZLocation;
		
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION(Reliable, Server, WithValidation)
		void Server_SetLocation();
	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_WallWalking(bool wallWalking);

	
public:
	//Dodge
	UPROPERTY(EditAnywhere, Category = "Dodge")
		float DodgeStrength;
	UPROPERTY(EditAnywhere, Category = "WallWalking")
		float WallWalkingStrength;
	//variables that will be used for the animation	
	uint8 bWalkingUp : 1;
	uint8 bWalkingSideWays : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerMusicSkill)
	bool bWalkingInWall;

	//methods called by the main actor
	void Walkup();
	void SideWalk();
	void Dodge();
};
