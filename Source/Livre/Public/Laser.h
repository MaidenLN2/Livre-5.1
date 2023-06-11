// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Laser.generated.h"

class ALivreCharacter;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Mesh Variables
	UPROPERTY(EditInstanceOnly)
	UStaticMeshComponent* Mesh;

	// functions
	UFUNCTION()
	void BeginLaserOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);

	UPROPERTY(EditAnywhere, Category = "Laser setup")
	float onlineTime = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Laser setup")
	float offlineTime = 2.0f;
	float trackingTime = 0.0f;

	// bools
	bool isActive = false;

	static ALivreCharacter* luizRef;

};
