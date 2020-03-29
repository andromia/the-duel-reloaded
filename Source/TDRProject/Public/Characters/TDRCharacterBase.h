// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDRCharacterBase.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class DodgeDirection : uint8
{	
	Nothing     UMETA(DisplayName = "Nothing"),
	Forward 	UMETA(DisplayName = "Forward"),
	Left 		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right"),
	Backward	UMETA(DisplayName = "Backward")
};

UCLASS()
class TDRPROJECT_API ATDRCharacterBase : public ACharacter
{
	GENERATED_BODY()


private:

	void AddControllerYawInput(float Val) override;
	void AddControllerPitchInput(float Val) override;
	virtual void BeginPlay() override;
	DECLARE_DELEGATE_OneParam(DirectionDelagate, DodgeDirection);

protected:
	
	void MoveForward(float Value);	
	void MoveRight(float Value);		
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//movement methods
	void Dash(DodgeDirection Direction);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDash(DodgeDirection Direction);
	bool ServerDash_Validate(DodgeDirection Direction);
	void ServerDash_Implementation(DodgeDirection Direction);

	void StopDash();

	//Atack Methods
	void Atack();
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAtack();
	bool ServerAtack_Validate();
	void ServerAtack_Implementation();

	////Die methods
	void Die();
	UFUNCTION(NetMulticast, Reliable, WithValidation)	
	void MultiDie();
	bool MultiDie_Validate();
	void MultiDie_Implementation();	     
	void DestroyChar();

	//Weapons
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeaponBase> WeaponClass;
	class AWeaponBase* Weapon;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* MeshComp;

	class UPlayerStatComponent* PlayerStatComp;

	FTimerHandle DashHandle;

	FTimerHandle DestroyHandle;

	class ULineTrace* LineTraceComp;

public:
	UPROPERTY(EditAnywhere, Category = "Camera")
		float BaseTurnRate;
	UPROPERTY(EditAnywhere, Category = "Camera")
		float BaseLookupAtRate;

	UPROPERTY(Replicated)
	DodgeDirection DashingDirection;
	
	UPROPERTY(Replicated)
		bool bAtacking;

	UPROPERTY(Replicated)
		bool bBlocking;

public:
	ATDRCharacterBase();
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		DodgeDirection GetDash();

	UFUNCTION(BlueprintCallable)
		bool GetAtack();

	UFUNCTION(BlueprintPure)
		FString GetInformation();
	
	
};
