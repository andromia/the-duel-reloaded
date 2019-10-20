// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDRCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

UCLASS()
class TDRPROJECT_API ATDRCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDRCharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UStaticMeshComponent* MeshComp;
	
	virtual void BeginPlay() override;

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void InteractPressed();

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
	void TraceForward_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void DodgeRight();
	void DodgeRight_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void DodgeLeft();
	void DodgeLeft_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void DodgeForward();
	void DodgeForward_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void DodgeBackward();
	void DodgeBackward_Implementation();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

public:	

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	AActor* FocusedActor;
};
