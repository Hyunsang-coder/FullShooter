// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr) 
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter) 
	{
		//Get the character's lateral velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// Is the character in the air? 
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();


		// Is the character accelerating?
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else bIsAccelerating = false;
		

		// 에임과 움직임 사이의 Offset 값 얻기
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		if (bIsAccelerating) 
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		// 화면에 수치 표시해 debugging
		FString OffsetYawMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementOffsetYaw);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetYawMessage);
		}
		
		bIsAiming = ShooterCharacter->GetIsAiming();
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}


