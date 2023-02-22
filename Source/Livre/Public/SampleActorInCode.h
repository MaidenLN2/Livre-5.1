// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "SampleActorInCode.generated.h"

UCLASS()
class LIVRE_API ASampleActorInCode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASampleActorInCode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Player reference
	AActor* PlayerRef;

	// Mesh Variables
	USceneComponent* SceneRoot;
	UStaticMeshComponent* Base;
	// UStaticMeshComponent* Post;
	// UStaticMeshComponent* Handles;
	UArrowComponent* Arrow;

	// Collision Components
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* FrontCollision;
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* BackCollision;
	// Collision Overlaps
	bool isFrontCollided = false;
	bool isBackCollided = false;

	// FUNCTIONS
	// Front
	UFUNCTION()
	void BeginFrontOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);
	UFUNCTION()
	void EndFrontOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// Back
	UFUNCTION()
	void BeginBackOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);
	UFUNCTION()
	void EndBackOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// Initialised
	bool bIsInitialised = false;
};
