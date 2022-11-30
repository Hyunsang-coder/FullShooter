// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent> (TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(ItemMesh);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(ItemMesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	// Hide the pickup widget
	if (PickupWidget)
	{	
		PickupWidget->SetVisibility(false);
	}
	// Set Active StarsArray based on Rarity
	SetActiveStars();

	//Set up overlap for AreaSphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnBeginOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnEndOverlap);

	SetItemProperties(ItemState);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveItem(DeltaTime);
}

void AItem::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Red, TEXT("Overlapped!"));
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character) 
	{
		Character->IncrementOverlappedItemCount(1);
	}
}

void AItem::OnEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//GEngine->AddOnScreenDebugMessage(4, 3.f, FColor::Red, TEXT("Overlap ended!"));
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		Character->IncrementOverlappedItemCount(-1);
	}
	PickupWidget->SetVisibility(false);
}

void AItem::SetActiveStars()
{
	// Not going to use 0 element 
	for (int32 i = 0; i < 6; i++) 
	{
		ActiveStarts.Add(false);
	}

	switch (ItemRarity) 
	{
		case EItemRarity::EIR_Damaged:
			ActiveStarts[1] = true;
			break;
		case EItemRarity::EIR_Common:
			ActiveStarts[1] = true;
			ActiveStarts[2] = true;
			break;
		case EItemRarity::EIR_Uncommon:
			ActiveStarts[1] = true;
			ActiveStarts[2] = true;
			ActiveStarts[3] = true;
			break;
		case EItemRarity::EIR_Rare:
			ActiveStarts[1] = true;
			ActiveStarts[2] = true;
			ActiveStarts[3] = true;
			ActiveStarts[4] = true;
			break;

		case EItemRarity::EIR_Legendary:
			ActiveStarts[1] = true;
			ActiveStarts[2] = true;
			ActiveStarts[3] = true;
			ActiveStarts[4] = true;
			ActiveStarts[5] = true;
			break;
	}

}

void AItem::SetItemProperties(EItemState State)
{
	switch (State) 
	{
		case(EItemState::EIS_PickUp):
		//Set Mesh Properties
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set AreaSphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//Set CollisionBox properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionResponseToChannel(
				ECollisionChannel::ECC_Visibility, 
				ECollisionResponse::ECR_Block);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;

		case(EItemState::EIS_Equipped):
			PickupWidget->SetVisibility(false);
			//Set mesh properties 
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//Set AreaSphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//Set CollisionBox properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case(EItemState::EIS_Falling):
			//Set mesh properties 
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(
				ECollisionChannel::ECC_WorldStatic,
				ECollisionResponse::ECR_Block);
			//Set AreaSphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//Set CollisionBox properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		case(EItemState::EIS_EquipInterp):
			PickupWidget->SetVisibility(false);
			//Set mesh properties 
			ItemMesh->SetVisibility(true);
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//Set AreaSphere properties
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//Set CollisionBox properties
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

	}
}



void AItem::DisplayWidget() 
{
	PickupWidget->SetVisibility(true);
}


void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartInterp(AShooterCharacter* _Player)
{
	Player = _Player;
	InterpStartLocation = GetActorLocation();
	bIsInterp = true;
	SetItemState(EItemState::EIS_EquipInterp);

	GetWorldTimerManager().SetTimer(
		InterpZTimer,
		this,
		&AItem::FinishInterp,
		InterpZTime);

	//Get Delta Yaw
	const FRotator CameraRotation = Player->GetCamera()->GetComponentRotation();
	const FRotator ItemRotation = GetActorRotation();
	InitialDeltaYaw = ItemRotation.Yaw - CameraRotation.Yaw;

	FString DeltaYawLog = FString::Printf(TEXT("Initial Delta Yaw: %f"), InitialDeltaYaw);
	GEngine->AddOnScreenDebugMessage(2, 10.f, FColor::White, DeltaYawLog);
}

void AItem::FinishInterp()
{
	bIsInterp = false;
	if (Player) 
	{
		Player->GetPickUpItem(this);
	}
}

void AItem::MoveItem(float DeltaTime) 
{
	if (!bIsInterp) return;

	if (Player && InterpZCurve) 
	{
		// Get Elapsed Time by passing in InterpZTimer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(InterpZTimer);

		// Get Z Curve value accoding to Elapsed Time 
		const float CurveValue = InterpZCurve->GetFloatValue(ElapsedTime);

		// Get Initial Item Location & 
		FVector ItemLocation = InterpStartLocation;
		InterpTargetLocation = Player->GetCameraInterpLocation();

		//Vector from Item to InterpTargetLocation, only get Z.
		const FVector ItemToCamera = { 0, 0, (InterpTargetLocation - ItemLocation).Z };
		const float DeltaZ = ItemToCamera.Size(); // In fact, not necessary here.


		// Interp x, y values;
		InterpX_Value = FMath::FInterpTo(InterpStartLocation.X, InterpTargetLocation.X, DeltaTime, 30.f);
		InterpY_Value = FMath::FInterpTo(InterpStartLocation.Y, InterpTargetLocation.Y, DeltaTime, 30.f);


		// Apply the value to the item locaiton
		ItemLocation.X = InterpX_Value;
		ItemLocation.Y = InterpY_Value;

		// Multiply curveValue (based on elapsed time) by DeltaZ
		ItemLocation.Z += CurveValue * DeltaZ;
		// Change the item's location 
		SetActorLocation(ItemLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// Change the item's rotation
		const FRotator CameraRotation = Player->GetCamera()->GetComponentRotation();
		const float ItemYaw = CameraRotation.Yaw + InitialDeltaYaw;
		SetActorRotation(FRotator(0.f, ItemYaw, 0.f), ETeleportType::TeleportPhysics);
	}
	
}
