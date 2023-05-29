// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTimer.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelTimer::ALevelTimer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ALevelTimer::SafeLevelReload()
{
	currentLevel->GetTimerManager().ClearAllTimersForObject(this);

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TSoftObjectPtr<UWorld>(currentLevel), false);
}

// Called when the game starts or when spawned
void ALevelTimer::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(timeLimit, [&]()
	{
		if (time > 0)
		{
			time--;
			UE_LOG(LogTemp, Warning, TEXT("Time = %i"), time);
		}
		else if (time <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Timer Ended"));
			UGameplayStatics::OpenLevel(this, FName("Time"));
		}
	}, 1.0, true);
}

void ALevelTimer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(timeLimit);
}

// Called every frame
void ALevelTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

