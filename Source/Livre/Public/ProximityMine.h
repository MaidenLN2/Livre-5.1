// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "ProximityMine.generated.h"

UCLASS()
class LIVRE_API AProximityMine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProximityMine();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float damage = 1.0f;
	UPROPERTY(EditAnywhere, Category = Mesh)
	UStaticMeshComponent* MineMesh;
	UPROPERTY(EditAnywhere, Category = MineSphere)
	USphereComponent* MineSphereComponent;
	UPROPERTY(EditAnywhere, Category = SoundCue)
	USoundCue* MineSound;
	UFUNCTION()
	void Explode();
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& sweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex);

	FTimerHandle mineTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Delay)
	float mineDelay =  2.0f;	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
