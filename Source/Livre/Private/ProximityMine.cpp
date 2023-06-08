// Fill out your copyright notice in the Description page of Project Settings.


#include "ProximityMine.h"
#include "Engine.h"
#include "Components/SphereComponent.h"

// Sets default values
AProximityMine::AProximityMine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MineMesh =  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	RootComponent = MineMesh;
	MineSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("MineSphere"));
	MineSphereComponent->Setupattachment(MineMesh);
	MineSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProximityMine::OnActorBeginOverlap);
	
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

