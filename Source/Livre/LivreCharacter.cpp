// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreCharacter.h"
#include "LivreProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h" // for update component etc
#include "CoreFwd.h" // for some vectors
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SimModule/SimulationModuleBase.h"


//////////////////////////////////////////////////////////////////////////

/*
LivreCharacter: Luiz Guilherme Ferreira Costa

Age: 17
Height: full capsule component
Family: parents, grandmother and 6 older siblings 
Occupation: wall cleaner/agent of Espíritos de Liberdade
Hobbies: parkour, graffiti, dancing
Favourite music: hip-hop, technowave
Coding goal: make gameplay and player controller work in cohesion with the story
 */

ALivreCharacter::ALivreCharacter()
{
	// Autopossession 
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	
	//tUpdateWallRun = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline UpdaterWallRun"));
	//tUpdateWallRun->SetTimelineLength(5.0f);

	wallDetectionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Wall Detector"));
	wallDetectionCapsule->SetupAttachment(RootComponent);
	wallDetectionCapsule->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	wallDetectionCapsule->SetCapsuleHalfHeight(200.0f);
	wallDetectionCapsule->SetCapsuleRadius(10.0f);
	wallDetectionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ALivreCharacter::WallDetectionBeginOverlap);
	wallDetectionCapsule->OnComponentEndOverlap.AddDynamic(this, &ALivreCharacter::WallDetectionEndOverlap);
	wallDetectionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	wallDetectionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	// Connecting Collision Detection Functions
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ALivreCharacter::CapsuleTouched);	// might work?
}

void ALivreCharacter::BeginPlay()
{
	// call the base class  
	Super::BeginPlay();

	//add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Event Graph Functionality:
	EventJumpReset(maxJump);
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	
	//USkinnedMeshComponent::HideBoneByName(Neck, PBO_None); // might need to be a BP specific function

}

//////////////////////////////////////////////////////////////////////////// Input

 void ALivreCharacter::SetupPlayerInputComponent(class UInputComponent* playerInputComponent)
 {
 	// set up action bindings
 	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(playerInputComponent))
 	{
 		//Jumping
 		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomJump);
 		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomJumpEnded);	//  allows for jumps to reset

// 		//Moving
 		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ALivreCharacter::Move);

// 		//Looking
 		enhancedInputComponent->BindAction(lookAction, ETriggerEvent::Started, this, &ALivreCharacter::Look);

		//Sprinting
 		enhancedInputComponent->BindAction(sprintAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomSprintPressed);
 		enhancedInputComponent->BindAction(sprintAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomSprintReleased);	// stops the player from sprinting

 		//Sliding
 		enhancedInputComponent->BindAction(slideAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomSlidePressed);
 		enhancedInputComponent->BindAction(slideAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomSlideReleased);	// Stops the player from sliding and resets their values

 		//Wallrunning
 		enhancedInputComponent->BindAction(wallrunAction, ETriggerEvent::Started, this, &ALivreCharacter::BeginWallRun);
 		enhancedInputComponent->BindAction(wallrunAction, ETriggerEvent::Triggered, this, &ALivreCharacter::UpdateWallRun);
 		enhancedInputComponent->BindAction(wallrunAction, ETriggerEvent::Completed, this, &ALivreCharacter::CallEndWallRun);

 		playerInputComponent->BindAxis("Forward", this, &ALivreCharacter::MoveForward);
 		playerInputComponent->BindAxis("Right", this, &ALivreCharacter::MoveLateral);
 		playerInputComponent->BindAxis("MouseX", this, &ALivreCharacter::LookHorizontal);
 		playerInputComponent->BindAxis("MouseY", this, &ALivreCharacter::LookVertical);
 	}
 }

// Moving/looking functionality 
void ALivreCharacter::MoveForward(float value)
{
	AddMovementInput(RootComponent->GetForwardVector(), FMath::Clamp(value, -1.0f, 1.0f));
}

void ALivreCharacter::MoveLateral(float value)
{
	AddMovementInput(RootComponent->GetRightVector(), FMath::Clamp(value, -1.0f, 1.0f));
}

void ALivreCharacter::LookHorizontal(float value)
{
	AddControllerYawInput(value * sensitivity);
}

void ALivreCharacter::LookVertical(float value)
{
	AddControllerPitchInput(-value * sensitivity);
}

// Custom jump for wall running
void ALivreCharacter::CustomJump()
{
	UE_LOG(LogTemp, Warning, TEXT("Calling CustomJump()"));
	
	if (JumpUsed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling Jump()"));
		Jump();

		if (isWallRunning)
		{
			EndWallRun(JumpOff);
		}
	}
	printf("CustomJump()");
}

// What happens after jump ends
void ALivreCharacter::CustomJumpEnded()
{
	if (jumpLeft == 0)
	{
		FTimerHandle resetJumps;
		GetWorld()->GetTimerManager().SetTimer(resetJumps, [&]()
		{
			EventJumpReset(maxJump);
		}, 0.25f, false);
	}
	StopJumping();
}
// Sprinting functionality
void ALivreCharacter::CustomSprintPressed()
{
	if (!isSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartSprint from CustomSprintPressed"));
	}
	isSprinting = true;
	StartSprint();
}

void ALivreCharacter::CustomSprintReleased()
{
	if (isSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("StopSprint from CustomSprintReleased"));
	}
	isSprinting = false;
	StopSprint();
}
//Sliding functionality
void ALivreCharacter::CustomSlidePressed()
{
	if (isSprinting && !GetCharacterMovement()->IsFalling())
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
		// GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	LaunchCharacter(GetActorForwardVector() * 2000.0f, true, false);
	}
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Pressed"));
}

void ALivreCharacter::CustomSlideReleased()
{

	UKismetSystemLibrary::PrintString(this, "Sliding Happened After Animation");
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	FTimerHandle CSP_InternalTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(CSP_InternalTimerHandle, [&]()
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	}, 0.5f, false);
	
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Released"));
}

// Vaulting functionality (in progress, not present in game atm)
// start vaulting
void ALivreCharacter::CustomVaultingPressed()
{
	FVector startPos = GetActorLocation() - FVector(0.0, 0.0, 44.0);
	FVector endPos = startPos + (GetActorForwardVector() * 70.0f); 
	FHitResult hitTracking;
	// look more in here
	bool firstLineTraceDidHit = LineTrace(startPos, endPos, EDrawDebugTrace::ForDuration, hitTracking);

	if (firstLineTraceDidHit)
	{
		wallLocation = hitTracking.Location;
		wallNormal = hitTracking.Normal;
		
		FVector endPos2 = (
			wallLocation +
			(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -10.0f)
			);
		FVector startPos2 = endPos2 - FVector(0.0f, 0.0f, 200.0f);
		FHitResult hitTracking2;
		
		bool secondLineTraceDidHit = LineTrace(startPos2, endPos2, EDrawDebugTrace::ForDuration, hitTracking2);

		if (secondLineTraceDidHit)
		{
			wallHeight = hitTracking2.Location;
				// sequence 0
				aboutToClimb = ((wallHeight - wallLocation).Z > 60.0f);

				// sequence 1
				FVector startPos3 = (
					wallLocation +
					(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -50.0f) +
					FVector(0.0f, 0.0f, 250.0f)
					);
			
				FVector endPos3 = startPos3 - FVector(0.0f, 0.0f, 300.0f);
				FHitResult hitTracking3;

			bool thirdLineTraceDidHit = LineTrace(startPos3, endPos3, EDrawDebugTrace::ForDuration, hitTracking3);

			if (thirdLineTraceDidHit)
			{
				wallHeight2 = hitTracking3.Location;
				wallIsThick = (wallHeight - wallHeight2).Z <= 30.0f;	//  negation was removed for simplicity
			}
			else
			{
				wallIsThick = false;
			}

			if (aboutToClimb)
			{
				// sequence 0
				FVector startPos4 = GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);		
				bool fourthLineTraceDidHit = LineTrace(startPos4, startPos4 + GetActorForwardVector() * 70.0f, EDrawDebugTrace::ForDuration, hitTracking3);

				if (fourthLineTraceDidHit) canClimb = false;
				
				// this last trace might be unnecessary, none of the outputs are used.
				bool FifthLineTraceDidHit = LineTrace(GetActorLocation(), GetActorLocation() + FVector(0.0f, 0.0f, 200.0f), EDrawDebugTrace::ForDuration, hitTracking3);

					// sequence 1
					if (canClimb)
					{
						GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						GetCharacterMovement()->SetMovementMode(MOVE_Flying);
						SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
						
						SetActorLocation(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f + GetActorLocation());
						SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 44.0f));
					
							// play anim montage here
							// duration = PlayAnimMontage();
							float duration = 4.0f;

							UKismetSystemLibrary::PrintString(this, "landing happened after animation");

							FTimerHandle CVP_ClimbTimerHandle;
							GetWorld()->GetTimerManager().SetTimer(CVP_ClimbTimerHandle, [&]()
							{
								GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
								GetCharacterMovement()->SetMovementMode(MOVE_Walking);
							}, duration, false);
					}
			}
			
			else
				
			{
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);

				FVector newActorLocation = (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f) + GetActorLocation();
				SetActorLocation(newActorLocation);

				float duration;
				if (wallIsThick)
					{
						// getting Up Animation goes here
						// duration = PlayAnimMontage();
						duration = 4.0f;
					}
				
				else
					
					{
						SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 20.0f));
						
						// duration = PlayAnimMontage();
						duration = 3.0f;
						UKismetSystemLibrary::PrintString(this, "vaulting happened after animation");
					}
				
						FTimerHandle cvpTimerHandle;
						GetWorld()->GetTimerManager().SetTimer(cvpTimerHandle, [&]()
						{
							GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
							GetCharacterMovement()->SetMovementMode(MOVE_Walking);
							SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
						}, duration, false);

				
			}
		}
	}
}
//End vaulting
void ALivreCharacter::CustomVaultingReleased()
{
	
}

// Custom collision profile
FCollisionQueryParams ALivreCharacter::GetIgnoreCharacterParams()
{
	FCollisionQueryParams parametres;

	TArray<AActor*> characterChildren;
	GetAllChildActors(characterChildren);
	parametres.AddIgnoredActors(characterChildren);
	parametres.AddIgnoredActor(this);

	return parametres;
}

// Setting running speed
void ALivreCharacter::StartSprint(float newSprintSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = newSprintSpeed;
}

// Changing sprinting speed to walking speed
void ALivreCharacter::StopSprint(float newWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = newWalkSpeed;
}

// Setting movement velocity
 void ALivreCharacter::SetHorizontalVelocity(float velocityX, float velocityY)
{
	float z = GetCharacterMovement()->Velocity.Z;
	GetCharacterMovement()->Velocity = FVector(velocityX, velocityY, z);
	printf("set horizontal velocity");
}

// What is happening during actual wall run (math)
void ALivreCharacter::UpdateWallRun()
{
    if (wallToRunOn && isWallRunning)
    {
    	FVector2D cardinalForward;
    	FVector2D positiveX(1.0f, 0.0f);
    	FVector2D negativeX(-1.0f, 0.0f);
    	FVector2D positiveY(0.0f, 1.0f);
    	FVector2D negativeY(0.0f, -1.0f);
    	FVector2D actorForward(GetActorForwardVector());
    	float dotProduct = 999999;

    	if (float newProduct = FVector2D::DotProduct(actorForward, positiveX); newProduct < dotProduct)
    	{
    		dotProduct = newProduct;
    		cardinalForward = positiveX;
    	}
    	if (float newProduct = FVector2D::DotProduct(actorForward, negativeX); newProduct < dotProduct)
    	{
    		dotProduct = newProduct;
    		cardinalForward = negativeX;
    	}
    	if (float newProduct = FVector2D::DotProduct(actorForward, positiveY); newProduct < dotProduct)
    	{
    		dotProduct = newProduct;
    		cardinalForward = positiveY;
    	}
    	if (float newProduct = FVector2D::DotProduct(actorForward, negativeY); newProduct < dotProduct)
    	{
    		cardinalForward = negativeY;
    	}    
  	
    	FVector speedByWallRunDirection = FVector(-cardinalForward, GetActorForwardVector().Z) * GetCharacterMovement()->GetMaxSpeed();
    	GetCharacterMovement()->Velocity = speedByWallRunDirection;
    }
	printf("update wall run");

  	
}

void ALivreCharacter::ClampHorizontalVelocity()
{
    if (GetCharacterMovement()->IsFalling())
    {
        float length = GetHorizontalVelocity().Length();
        float lengthBySpeed = length / GetCharacterMovement()->GetMaxSpeed();
        
        if (lengthBySpeed > 1)
        {
            FVector2d clampedHorizontalVelocity = GetHorizontalVelocity() / lengthBySpeed;
            SetHorizontalVelocity(clampedHorizontalVelocity.X, clampedHorizontalVelocity.Y);
        }
    }
}

std::tuple<FVector, int> ALivreCharacter::FindRunDirectionAndSide(FVector inputWallNormal)
{
	enum WallRunSide sideLocal;
    FVector2d vectorFirstNormal(inputWallNormal);
    FVector2d vectorRight(GetActorRightVector());

    float dotProd = FVector2d::DotProduct(vectorFirstNormal, vectorRight);
    float zFlip = 1;
	
    if  (dotProd > 0)
    {
        sideLocal = Right;
    }
	
    else
    	
    {
        sideLocal = Left;
        zFlip *= -1;
    }

    FVector crossProduct = FVector::CrossProduct(inputWallNormal, FVector(0, 0, zFlip));
	
    return std::tuple(crossProduct, sideLocal);
}

bool ALivreCharacter::IsSurfaceWallRan(FVector surfaceVector)
{
    if (surfaceVector.Z < -0.05f)
    {
        return false;
    }
    
    FVector dotComp = FVector(surfaceVector.X, surfaceVector.Y, 0.0f);
    dotComp.Normalize();

    float dotResult = FVector::DotProduct(dotComp, surfaceVector);
    float arcCosDotResult = UKismetMathLibrary::DegAcos(dotResult);    // KismetMathLibrary 
    
    float walkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
    return (arcCosDotResult < walkableFloorAngle);
}

FVector ALivreCharacter::LaunchVelocity()
{
    FVector launchDirection;
    // sequence 0
    if (isWallRunning)
    {
        FVector crossComp = FVector(0, 0, wallRunSide == Left ? 1 : -1 );

        launchDirection = FVector::CrossProduct(wallRunDirection, crossComp);
    }
    else
    {
        if (GetCharacterMovement()->IsFalling())
        {
            launchDirection = (GetActorRightVector() * axisRight) + (GetActorForwardVector() * axisForward);
        }
    }
	printf("launch velocity");
    // sequence 1
    return (launchDirection + FVector(0, 0, 1)) * GetCharacterMovement()->JumpZVelocity;
}

bool ALivreCharacter::AreKeysRequired()
{
	const FVector2D currentVelocity(GetCharacterMovement()->Velocity);
	const FVector2D actorForwardVector2D(GetActorForwardVector());
	//const FVector2D actorRightVector2D(GetActorRightVector());

	const float DotForward = FVector2D::DotProduct(currentVelocity, actorForwardVector2D);

	UE_LOG(LogTemp, Warning, TEXT("DotForward = %f"), DotForward);

	if (DotForward > 90)
	{
		return true;
	}

	return false;
}

FVector2d ALivreCharacter::GetHorizontalVelocity()
{
	printf("get horizontal velocity");
    return FVector2d(GetCharacterMovement()->GetLastUpdateVelocity());
}

bool ALivreCharacter::JumpUsed()
{
	if (isWallRunning)	return true;

	if (jumpLeft > 0)
	{
		jumpLeft--;
		return true;
	}
	return false;
}

void ALivreCharacter::EventJumpReset(int jumps)
{
	jumpLeft = UKismetMathLibrary::Clamp(jumps, 0, maxJump);
}
// damage system
void ALivreCharacter::EventAnyDamage(float damage)
{
	health -= damage;

	if (health <= 0.0f)
	{
		isDead = true;
		UGameplayStatics::OpenLevel(this, FName("Main_Menu"));
	}
}

void ALivreCharacter::EventOnLanded()
{
	EventJumpReset(maxJump);

	GetCharacterMovement()->GravityScale = initialGravity;

	UGameplayStatics::PlayWorldCameraShake(this, TSubclassOf<UCameraShakeBase>(), GetActorLocation(), 0.0f, 100.0f, 1.0f);
}

bool ALivreCharacter::LineTrace(FVector startPos, FVector endPos, EDrawDebugTrace::Type durationType,
	FHitResult& hitResult)
{
	return UKismetSystemLibrary::LineTraceSingleForObjects(
				this,                                     
				startPos,       
				endPos,         
				TArray<TEnumAsByte<EObjectTypeQuery>>(),  
				true,                                      
				TArray<AActor*>(),                         
				durationType,   
				hitResult,       
				true                                       
				);
}

void ALivreCharacter::CapsuleTouched(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& sweepResult)
{
	if (isWallRunning) return;

	if (IsSurfaceWallRan(sweepResult.ImpactNormal) && GetCharacterMovement()->IsFalling())
	{
		auto [OutDir, OutSide] = FindRunDirectionAndSide(sweepResult.ImpactNormal);
		wallRunDirection = OutDir;

		switch (OutSide)
		{
		case Left:
			wallRunSide = Left;
			break;
		case Right:
			wallRunSide = Right;
			break;
		default: break;
		}

		if (AreKeysRequired())
		{
			BeginWallRun();
		}
		else
		{
			EndWallRun(FallOff);
		}
	}
}

void ALivreCharacter::BeginWallRun()
{
	
	// FTimerHandle TestHandle;        // Can be declared inside the function and not worried about again
	//
	// GetWorld()->GetTimerManager().SetTimer(TestHandle, [&]()
	// {
	// 	if (!isUpdatingWallRun)
	// 	{
	// 		isUpdatingWallRun = true;
	// 		GetWorld()->GetTimerManager().SetTimer(TestHandle, [&]()
	// 		{
	// 			EndWallRun(FallOff);
	// 		}, -1.0f, false, timeDelay);
	// 	}
	// 	else
	// 	{
	// 		UpdateWallRun();
	// 	}
	// }, 0.01f, true);

	/// LERA IMPLEMENTATION
	// // sequence 0
	// if (!isWallRunning)	// This check makes sure the function isn't called multiple times on activation. Once it activates once, it prevents it from starting again.
	// {
	// 	// Added check in here to see if the player is next to a wall before deciding to activate the wallrun.
	// 	// Doesn't check for sides atm.
	// 	FHitResult HitResultCapture;
	// 	
	// 	FVector rightTraceEnd = GetActorLocation() + (GetActorRightVector() * 250);
	// 	bool rightLineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
	// 		this,
	// 		GetActorLocation(),
	// 		rightTraceEnd,
	// 		UEngineTypes::ConvertToTraceType(ECC_Visibility),
	// 		false,
	// 		TArray<AActor*>(),
	// 		EDrawDebugTrace::None,
	// 		HitResultCapture,
	// 		true
	// 		);
	//
	// 	FVector leftTraceEnd = GetActorLocation() + (GetActorRightVector() * -250);
	// 	bool leftLineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
	// 		this,
	// 		GetActorLocation(),
	// 		leftTraceEnd,
	// 		UEngineTypes::ConvertToTraceType(ECC_Visibility),
	// 		false,
	// 		TArray<AActor*>(),
	// 		EDrawDebugTrace::None,
	// 		HitResultCapture,
	// 		true
	// 		);
	//
	// 	// If we did detect a line trace, perform the function and let's wallrun!
	// 	if (rightLineTraceDidHit || leftLineTraceDidHit)
	// 	{
	// 		if (rightLineTraceDidHit)
	// 		{
	// 			WallRunSide = Right;
	// 		}
	// 		else
	// 		{
	// 			WallRunSide = Left;
	// 		}
	// 		
	// 		FTimerHandle BWR_Delayhandle;
	// 		GetWorld()->GetTimerManager().SetTimer(BWR_Delayhandle, [&]()
	// 		{
	// 			// call End Wall Run
	// 			EndWallRun(FallOff);
	// 		}, 2, false);
	//
	// 		// sequence 1 where player runs on walls
	// 		GetCharacterMovement()->AirControl = 1.0f;
	// 		GetCharacterMovement()->GravityScale = 0.0f;
	//
	// 		isWallRunning = true;
	// 		//  place for camera tilt begin
	//
	// 		// Timeline the UpdateWallRun function for 5 seconds
	// 	}
	// }

	/// capsule implementation of wall running
	if (!isWallRunning && wallToRunOn)
	{
		FTimerHandle bwrDelayhandle;
		GetWorld()->GetTimerManager().SetTimer(bwrDelayhandle, [&]()
		{
			// call End Wall Run
			EndWallRun(FallOff);
		}, 2, false);

		// sequence 1 where player runs on walls
		GetCharacterMovement()->AirControl = 1.0f;
		GetCharacterMovement()->GravityScale = 0.0f;

		isWallRunning = true;
	}
}


void ALivreCharacter::CallEndWallRun()
{
	if (isWallRunning)
	{
		EndWallRun(FallOff);
	}
}

void ALivreCharacter::EndWallRun(WallRunEnd why)
{
	if (isWallRunning) // we only want to stop the wallrunning if it's already started
	{
		switch (why)
		{
		case FallOff:
			EventJumpReset(1);
			break;
		case JumpOff:
			EventJumpReset(maxJump);
			break;
		default: ;
		}

		GetCharacterMovement()->AirControl = 0.05f;
		GetCharacterMovement()->GravityScale = 1.0f; // why not working?
		isWallRunning = false;
		isUpdatingWallRun = false;
	}

	//  place for camera tilt end

	
}

void ALivreCharacter::WallDetectionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* otherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!wallToRunOn && otherComp->GetClass() == UStaticMeshComponent::StaticClass())
	{
		wallToRunOn = Cast<UStaticMeshComponent>(otherComp);
	}
}

void ALivreCharacter::WallDetectionEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* otherComp,
	int32 OtherBodyIndex)
{
	if (wallToRunOn && otherComp == wallToRunOn)
	{
		wallToRunOn = nullptr;
		EndWallRun(FallOff);
	}
}

void ALivreCharacter::Move(const FInputActionValue& value)
{
	// input is a Vector2D
	FVector2D movementVector = value.Get<FVector2D>();
	inputStorage = movementVector;

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), movementVector.Y);
		AddMovementInput(GetActorRightVector(), movementVector.X);
	}
}

void ALivreCharacter::Look(const FInputActionValue& value)
{
	// input is a Vector2D
	FVector2D lookAxisVector = value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(lookAxisVector.X);
		AddControllerPitchInput(lookAxisVector.Y);
	}
}