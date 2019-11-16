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
		uint8 bSavedWalkUp: 1;
		uint8 bReturnToNormal : 1;
	};

	class FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);
		typedef FNetworkPredictionData_Client_Character Super;
		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:		
	virtual void UpdateFromCompressedFlags(uint8 flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity);


	//Dodge
	UPROPERTY(EditAnywhere, Category = "Dodge")
		float DodgeStrength;

	UFUNCTION(Unreliable, Server, WithValidation)
		void Server_MoveDirection(const FVector& MoveDir);

	UFUNCTION(BlueprintCallable, Category = "Dodge")
		void Dodge(const FVector& MoveDir);

	void Walkup();
			

	FVector MoveDirection;
	uint8 bWantsToDodge : 1;
	uint8 bWalkup : 1;
	uint8 bReturnToNormal : 1;
	uint8 Turn : 1;
	void ReturnToNormal();
};