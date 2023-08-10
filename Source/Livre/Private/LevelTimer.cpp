// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelTimer.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ALevelTimer::ALevelTimer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelTimer::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);

	if (TimerEndingSound)
	{
		finalSoundTimeTracker = TimerEndingSound->GetDuration();
	}
	
	GetWorld()->GetTimerManager().SetTimer(timeLimit, [&]()
	{
		if (time > 0)
		{
			time--;
			UE_LOG(LogTemp, Warning, TEXT("Time = %i"), time);

			if (TimerEndingSound && time == FMath::Floor(TimerEndingSound->GetDuration()))
			{ 
				UGameplayStatics::PlaySound2D(this, TimerEndingSound);
				SetActorTickEnabled(true);
			}
		}
		else if (time <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Timer Ended"));
			EndTimer();
			GetWorld()->GetTimerManager().ClearTimer(timeLimit);
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

	finalSoundTimeTracker-= DeltaTime;
	UE_LOG(LogTemp, Warning, TEXT("FSTT = %f"), finalSoundTimeTracker);

	if (finalSoundTimeTracker <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Timer Ended from sound"));
		EndTimer();
		GetWorld()->GetTimerManager().ClearTimer(timeLimit);
	}
}

