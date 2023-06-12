// Fill out your copyright notice in the Description page of Project Settings.


#include "ProximityMine.h"
#include "Engine.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Livre/LivreCharacter.h"

// Sets default values
AProximityMine::AProximityMine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MineMesh =  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	RootComponent = MineMesh;
	MineSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("MineSphere"));
	MineSphereComponent->SetupAttachment(MineMesh);
	MineSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProximityMine::OnBeginOverlap);
	MineSphereComponent->OnComponentEndOverlap.AddDynamic(this, &AProximityMine::OnEndOverlap);
	
}

// Called when the game starts or when spawned
void AProximityMine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProximityMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProximityMine::Explode()
{
	if (ALivreCharacter* player = Cast<ALivreCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		player->DealDamageToPlayer(damage);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), MineSound, this->GetActorLocation());
		Destroy();
	}
}

void AProximityMine::OnBeginOverlap(UPrimitiveComponent* overlappedComp, AActor* otherActor,
	UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& sweepResult)
{
	GetWorld()->GetTimerManager().SetTimer(mineTimer, this, &AProximityMine::Explode, mineDelay, false);
}

void AProximityMine::OnEndOverlap(UPrimitiveComponent* overlappedComp, AActor* otherActor,
	UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	GetWorld()->GetTimerManager().ClearTimer(mineTimer);
}

