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
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"

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

	SetDefaultWeapon();
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	UpdateTurnLookupRate();
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();

}


void AShooterCharacter::TraceForItems() 
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemHitResult;
		FVector EndLocation;
		TraceUnderCrossHair(ItemHitResult, EndLocation);
		
		if (ItemHitResult.bBlockingHit) 
		{
			AItem* HitItem = Cast<AItem>(ItemHitResult.GetActor());
			if (HitItem) 
			{
				// I decided to use a timer instead of Stephen's logic
				HitItem->DisplayWidget();
			}
		}

		//if (ItemHitResult.bBlockingHit)
		//{
		//	// HitResul
		//	AItem* HitItem = Cast<AItem>(ItemHitResult.GetActor());
		//	// Casting
		//	if (HitItem && HitItem->GetPickupWidget())
		//	{
		//		HitItem->GetPickupWidget()->SetVisibility(true);
		//		
		//	}
		//	// Hit an AItem last frame
		//	if (TraceHitItemLastFrame) 
		//	{
		//		if (HitItem != TraceHitItemLastFrame) 
		//		{
		//			TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		//		}
		//	}
		//	TraceHitItemLastFrame = HitItem;
		//}
		//else if (TraceHitItemLastFrame) 
		//{
		//	TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		//}
	}
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

	// Convert WalkSpeedRange to the range of VelocityMultiplierRange. Input is Velocity.Size(); 
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
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Released, this, &AShooterCharacter::AimButtonReleased);

}




void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
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
	if (FireSound) 
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

				
		//BeamEndPoint ���� �� -> Ÿ�� ȿ��(impact particle) ����
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
		// Beam ȿ�� 
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


bool AShooterCharacter::GetBeamEndLocation(const FVector & MuzzleSocketLocation, FVector & OutBeamEndLocation)
{
	FHitResult CorsshairHitResult;
	if(TraceUnderCrossHair(CorsshairHitResult, OutBeamEndLocation))
	{
		OutBeamEndLocation = CorsshairHitResult.Location;
	}
	else 
	{
		// EndLocation will be the end of the beam, which is set in TraceUnderCrossHair
		return false;
	}


	// Barrel���� 2nd LineTracing - ���⼭ OutBeamLocation ���� ������!
	FHitResult BarrelTraceHit;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	FVector StartToEnd{ OutBeamEndLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd *1.25;
	FVector TestBeamEnd = OutBeamEndLocation * 1.25;


	GetWorld()->LineTraceSingleByChannel(
		BarrelTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);

	/* For debugging
	DrawDebugLine(GetWorld(), WeaponTraceStart, WeaponTraceEnd, FColor::Red, true);
	
	FString WeaponTraceEndLog = FString::Printf(TEXT("WeaponTraceEndLocation: %s"), *WeaponTraceEnd.ToString());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, WeaponTraceEndLog);
	}
	*/

	if (BarrelTraceHit.bBlockingHit)
	{
		OutBeamEndLocation = BarrelTraceHit.Location;
	}
	return true;
	
}


bool AShooterCharacter::TraceUnderCrossHair(FHitResult& ItemHitResult, FVector& OutEndLocation)
{
	// Get Viewport Size 
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Divide ViewportSize by 2 to get the center of the screen
	FVector2D CrosshairLocation{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };
	CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get 3D positions of crosshair from the 2D position
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	// LineTrace from the 3D position of the crosshair
	if (bScreenToWorld)
	{
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * WeaponRange;
		OutEndLocation = End;

		GetWorld()->LineTraceSingleByChannel(
			ItemHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);

		/* For debugging
		if (bIsShooting) 
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
		}
		*/

		if (ItemHitResult.bBlockingHit && ItemHitResult.GetActor())
		{
			return true;
		}
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

void AShooterCharacter::FireButtonPressed()
{
	//doesn't mean it's true while holding down the button.
	// It is set to true once you hit the button.
	bFireButtonPressed = true;
	AutoFire();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::AutoFire()
{
	if(bShouldFire) 
	{
		FireWeapon();
		bShouldFire = false;
		//bShouldFire = false;
		GetWorldTimerManager().SetTimer(
			FireHandle,
			this,
			&AShooterCharacter::AutoFireReset,
			AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed) 
	{
		AutoFire();
	}
}

void AShooterCharacter::IncrementOverlappedItemCount(int32 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else 
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}


void AShooterCharacter::SetDefaultWeapon() 
{
	if (DefaultWeaponClass)
	{
		// Spawn a weapon
		AWeapon* DefaultWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

		//Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(DefaultWeapon, GetMesh());
		}
		EquippedWeapon = DefaultWeapon;

		// Set Equipped Weapon's collision to ignore all
		EquippedWeapon->SetCollisionToIgnoreAll();
	}
}
