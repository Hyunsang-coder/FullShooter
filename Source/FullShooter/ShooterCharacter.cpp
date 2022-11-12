// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	HipFireTurnRate(45.f),
	HipFireLookUpRate(45.f),
	AimingTurnRate(22.f),
	AimingLookUpRate(22.f),
	bIsAiming(false),
	DefaultFOV(90.f),
	ZoomedFOV(35.f),
	CrosshairVelocityFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairShootingFactor(0.f),
	bIsShooting(false),
	CrosshairShootingDuration(0.05f)

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// CameraBoom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Controller only affects the camera, not the character
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Configure Character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character move in the direction of Input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera) 
	{
		FollowCamera->SetFieldOfView(DefaultFOV);
		CurrentFOV = DefaultFOV;
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	UpdateTurnLookupRate();
	CalculateCrosshairSpread(DeltaTime);

}

void AShooterCharacter::UpdateTurnLookupRate() 
{
	if (bIsAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipFireTurnRate;
		BaseLookUpRate = HipFireLookUpRate;
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CorsshairSpreadMultiplier;
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime) 
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity { GetVelocity()};
	Velocity.Z = 0.f;

	// WalkSpeedRange 범위의 Velocity.size의 값을 VelocityMultiplierRange로 변환 
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size());

	//  InAirFactor
	if (GetCharacterMovement()->IsFalling()) 
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else 
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 15.f);
	}

	// AimFactor
	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.5f, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, ZoomInterpSpeed);
	}

	// ShiootingFactor
	if (bIsShooting) 
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else 
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CorsshairSpreadMultiplier = 
		0.5f +
		CrosshairVelocityFactor + 
		CrosshairInAirFactor + 
		CrosshairAimFactor + 
		CrosshairShootingFactor;


	FString VelocityFactorLog = FString::Printf(TEXT("CrosshairSpreadMultiplier: %f"), CorsshairSpreadMultiplier);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Yellow, VelocityFactorLog);
	}
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime) 
{
	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	FollowCamera->SetFieldOfView(CurrentFOV);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Released, this, &AShooterCharacter::AimButtonReleased);

}




void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Controller에서 로테이션이랑 Yaw 가져오기
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Controller에서 로테이션이랑 Yaw 가져오기
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, Value);
	}
}


void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * SensitivityMultiplier); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * SensitivityMultiplier); // deg/sec * sec/frame
}


void AShooterCharacter::FireWeapon()
{
	// 1. 사운드 플레이
	if (FireSound) 
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	//2 소켓 트랜스폼 가져오기 
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		//3. 총구 이펙트 플레이 
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

				
		//BeamEndPoint 가져 와 -> 타격 효과(impact particle) 구현
		FVector BeamEndPoint;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEndPoint);

		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEndPoint);
			}
		}
		// Beam 효과 
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransform);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
	}

	// GetAnimInstance and PlayMontage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage) 
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	StartShootingSpread();

	/*FString WeaponFireLog = FString::Printf(TEXT("Fired the weapon"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Red, WeaponFireLog);
	}*/
}


bool AShooterCharacter::GetBeamEndLocation(const FVector & MuzzleSocketLocation, FVector & OutBeamLocation)
{
	//GEngine으로 ViewportSize 가져오기
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ViewportSize 조정 후 2D스크린 좌표를 3D 월드 좌표로 변환
	FVector2D CrosshairLocation{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };
	CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	// 1.화면 중심 기준 LineTracing
	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * WeaponRange;

		OutBeamLocation = End;

		GetWorld()->LineTraceSingleByChannel(
			ScreenTraceHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;
		}


		// 2. Barrel기준 LineTracing 
		FHitResult BarrelTraceHit;
		const FVector BarrelStart = MuzzleSocketLocation;
		const FVector BarrelEnd = OutBeamLocation;

		GetWorld()->LineTraceSingleByChannel(
			BarrelTraceHit,
			BarrelStart,
			BarrelEnd,
			ECollisionChannel::ECC_Visibility);

		if (BarrelTraceHit.bBlockingHit)
		{
			OutBeamLocation = BarrelTraceHit.Location;
		}
		return true;
	}
	return false;
}

void AShooterCharacter::AimButtonPressed()
{
	bIsAiming = true;
}

void AShooterCharacter::AimButtonReleased()
{
	bIsAiming = false;
}

void AShooterCharacter::StartShootingSpread() 
{
	bIsShooting = true;
	GetWorldTimerManager().SetTimer(
		ShootingTimer, 
		this, 
		&AShooterCharacter::FinishShootingSpread,
		CrosshairShootingDuration);
}

void AShooterCharacter::FinishShootingSpread()
{
	bIsShooting = false;
}
