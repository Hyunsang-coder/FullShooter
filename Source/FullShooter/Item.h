// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary")
};

UCLASS()
class FULLSHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Set Active Stars based on rarity
	void SetActiveStars();
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void DisplayWidget();

	UFUNCTION()
	void ResetWidgetTimer();

	// In order to set collision to ignoreall when it's equipped by Character
	void SetCollisionToIgnoreAll();

private:
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;


	FTimerHandle WidgetTimer;
	UPROPERTY(EditAnywhere,BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	float WidgetDisplayTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	FString ItemName = "Default";

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity = EItemRarity::EIR_Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStarts;

public:
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; };
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; };
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; };
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

	
};
