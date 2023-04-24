// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Laser.generated.h"

UCLASS()
class LIVRE_API ALaser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaser();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	// Mesh Variables
	USceneComponent* SceneRoot;
	UStaticMeshComponent* Base;
	UArrowComponent* Arrow;

	// Collision Components
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* FrontCollision;
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* BackCollision;
	// Collision Overlaps
	bool isFrontCollided = false;
	bool isBackCollided = false;

	// functions
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

	UPROPERTY(EditAnywhere, Category = "Laser setup")
	float lifetime;

};
