// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(0.7f),
	bIsFalling(false)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Keep the weapon upright
	if (GetItemState() == EItemState::EIS_Falling && bIsFalling) 
	{
		FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Get the mesh's forward and right direction
	// Note: Weapon Mesh is looking to the right so "Mesh Right is the forward direction" in the world direciton.
	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };

	// Direction in which we throw the weapon (rotated a little to the ground)
	// Rotate it around the X axis 
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	
	// Randomly rotate the direction around the world Z axis
	float RandomAngle{ FMath::RandRange(10.f, 30.f) };
	ImpulseDirection.RotateAngleAxis(RandomAngle, FVector(0.f, 0.f, 1.f));

	ImpulseDirection *= 7'000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);
	
	bIsFalling = true;
	GetWorldTimerManager().SetTimer(
		ThrowWeaponTimer,
		this, 
		&AWeapon::StopFalling,
		ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
}
