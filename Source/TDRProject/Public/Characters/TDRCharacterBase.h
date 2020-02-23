// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../MovementComponent/TDRCharacterMovementComponent.h"
#include "TDRCharacterBase.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UAnimSequence;

UCLASS() // could add config=Game
class TDRPROJECT_API ATDRCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	/* 
	* TODO: figure out public/private/protected solution
	* why are we initializing twice?
	*/
	ATDRCharacterBase();
	ATDRCharacterBase(const FObjectInitializer& ObjectInitializer);

	UTDRCharacterMovementComponent* MyCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool Debug;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseLookupAtRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseDodgeMultiplier;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float TraceDistance;

	UFUNCTION(BlueprintNativeEvent)
	void TraceForward();

	/** Refactored to assign this abstraction to the game engine
	 * UFUNCTION(BluePrintCallable, Category = "Movement")
	 * FORCEINLINE class UTDRCharacterMovementComponent* GetMovementComponet() const { return MyCharacterMovementComponent; }
	 * **/

	UFUNCTION(BlueprintCallable)
	bool GetDash();

	// Server replicated variable
	UPROPERTY(Replicated)
	bool bDashing;

	FTimerHandle DashHandle;

	void StopDash();

	//used by movement component to know which wall is being touched
	bool bLeftArmTouchingWall;

private:

	AActor* FocusedActor;
	AActor* aWallTouched;

	bool bWallTouching;
	bool bRightArmTouchingWall;

	void Jump();
	void Dodge();
	void Dash(int movementType);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void InteractPressed();
	void TraceForward_Implementation();
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDash(int movementType);
	bool ServerDash_Validate(int movement);
	void ServerDash_Implementation(int movement);
	
	void AddControllerYawInput(float Val) override;
	void AddControllerPitchInput(float Val) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

};
