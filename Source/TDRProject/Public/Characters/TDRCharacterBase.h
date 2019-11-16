// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDRCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UAnimSequence;

UCLASS()
class TDRPROJECT_API ATDRCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDRCharacterBase();

	ATDRCharacterBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BluePrintCallable, Category = "Movement")
		FORCEINLINE class UTDRCharacterMovementComponent* GetMovementComponet() const { return MyCharacterMovementComponent; }

	virtual void PostInitializeComponents() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* MeshComp;

	virtual void BeginPlay() override;

protected:

	FTimerHandle TimerHandle;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

	void InteractPressed();
	void StopCurrentAnimation();
	void Dash();

	UPROPERTY(EditAnywhere, Category = "Debug")
		bool Debug;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float BaseTurnRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float BaseLookupAtRate;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float BaseDodgeMultiplier;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimSequence* DodgeForwardAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimSequence* DodgeBackwardAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimSequence* DodgeRightAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimSequence* DodgeLeftAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
		UAnimSequence* WallWalkingAnim;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		float TraceDistance;

	UFUNCTION(BlueprintNativeEvent)
		void TraceForward();
	void TraceForward_Implementation();

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


public:

	enum MovementType { forward, backward, left, right };
	void Dodge(MovementType direction);
	virtual void Tick(float DeltaTime) override;
	void StopDashing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent * LeftHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent * RightHandMesh;

#pragma region Dash properties

	UPROPERTY(EditAnywhere, Category = "Dash")
		bool Dashing;

	UPROPERTY(EditAnywhere, Category = "Dash")
		float DashStop;

	UPROPERTY(EditAnywhere, Category = "Dash")
		FTimerHandle UnusedHandle;

	UPROPERTY(EditAnywhere, Category = "WalkUp")
		float WalkUpDistance;
#pragma endregion Dash properties

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	AActor* FocusedActor;
	void StartAnimationAndEndWithIddle(UAnimSequence*);
	DECLARE_DELEGATE_OneParam(DirectionDelagate, MovementType);
	void Jump();
	void TurnBack();

	bool bWallTouching;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallWalking)
	bool bWallWalking;
	bool bLeftArmTouchingWall;
	bool bRightArmTouchingWall;
	AActor* aWallTouched;
	float fRotationXForWallWalk;
	float fRotationZforWallWalk;
	int Counter;
	bool bWalkingSideWays;
	float zValue;	
	UTDRCharacterMovementComponent* MyCharacterMovementComponent;
	void AddControllerYawInput(float Val) override;
	void AddControllerPitchInput(float Val) override;
	
};
