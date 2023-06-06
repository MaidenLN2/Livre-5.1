// Fill out your copyright notice in the Description page of Project Settings.


#include "Laser.h"
#include "Livre/LivreCharacter.h"
#include "Kismet/GameplayStatics.h"

ALivreCharacter* ALaser::luizRef;

// Sets default values
ALaser::ALaser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->OnComponentBeginOverlap.AddDynamic(this, &ALaser::BeginLaserOverlap);
}

// Called when the game starts or when spawned
void ALaser::BeginPlay()
{
	Super::BeginPlay();

	if (!luizRef)
	{
		luizRef = Cast<ALivreCharacter>(UGameplayStatics::GetActorOfClass(this, ALivreCharacter::StaticClass()));
	}
}

void ALaser::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (luizRef)
	{
		luizRef = nullptr;
	}
}

// Called every frame
void ALaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	trackingTime += DeltaTime; // keeps track how long lasers were working 

	if (trackingTime > 0 && trackingTime < onlineTime && !isActive)
	{
		isActive = true;

		// make visible/collideable
		Mesh->SetVisibility(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else if (trackingTime >= onlineTime && trackingTime <= (onlineTime + offlineTime) && isActive)
	{
		isActive = false;

		// make invisible/uncollideable
		Mesh->SetVisibility(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// clamping
	if (trackingTime > (onlineTime + offlineTime))
	{
		trackingTime -= onlineTime + offlineTime;
	}
}

ALivreCharacter* ALaser::GetPlayerRef()
{
	return (luizRef ? luizRef : nullptr);
}

void ALaser::BeginLaserOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == luizRef)
	{
		UGameplayStatics::OpenLevel(this, FName("LaserDeath"));
	}
}
