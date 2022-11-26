// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class FULLSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();
	void ThrowWeapon();

protected:
	virtual void Tick(float DeltaTime) override;

	void StopFalling();

private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bIsFalling;
};
