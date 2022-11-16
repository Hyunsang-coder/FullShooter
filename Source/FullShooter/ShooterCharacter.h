// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class FULLSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	// Value 1 = 100% 원하는 속도
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamEndLocation);

	void AimButtonPressed();
	void AimButtonReleased();

	void CameraInterpZoom(float DeltaTime);
	void UpdateTurnLookupRate();
	void CalculateCrosshairSpread(float DeltaTime);
	
	void StartShootingSpread();

	UFUNCTION()
	void FinishShootingSpread();

	// Auto Fire-related functions 
	void FireButtonPressed();
	void FireButtonReleased();
	void AutoFire();
	UFUNCTION()
	void AutoFireReset();

	bool TraceUnderCrossHair(FHitResult& ItemHitResult, FVector& OutEndLocation);
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Cemera related 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed = 20.f;

	float DefaultFOV;
	float ZoomedFOV;
	float CurrentFOV;


	// Turn rates in deg/sec. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float HipFireTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float HipFireLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float AimingTurnRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float AimingLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "0.0", UIMax = "3.0"))
	float SensitivityMultiplier = 1.f;


	// Crosshair multiplier + relevant factors 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CorsshairSpreadMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Crosshair, meta = (AllowPrivateAccess = "true"))
	bool bIsShooting;

	FTimerHandle ShootingTimer;

	// Automatic fire variables
	bool bShouldFire = true;
	bool bFireButtonPressed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float AutomaticFireRate = 0.1f;

	FTimerHandle FireHandle;


	// Weapon assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	float WeaponRange = 5000.f;

	
	


public: 
	// 값을 바꾸지 않는 함수는 모두 const로 만들어 주자
	// 보통 단순하고 크기가 작은 함수에 FORCEINLINE 사용 (왔다갔다를 줄여줘서 성능 향상) 
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE UCameraComponent* GetCamera() const { return FollowCamera; }
	
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;
};
