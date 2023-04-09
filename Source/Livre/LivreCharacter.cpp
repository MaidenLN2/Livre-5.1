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
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ALivreCharacter::CapsuleTouched);

	// Setting default speed
	GetCharacterMovement()->MaxWalkSpeed = 10000.0f;

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
	GetCharacterMovement()->GravityScale = initialGravity;
	GetCharacterMovement()->bNotifyApex = true;
	// Event Graph Functionality:
	EventJumpReset(maxJump);
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	
	//USkinnedMeshComponent::HideBoneByName(Neck, PBO_None); // might need to be a BP specific function
	
	//timer functionality
	currentLevel = GetWorld();
	
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
			SafeLevelReload();
		}
	}, 1.0, true);
}

void ALivreCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(timeLimit);
	GetWorld()->GetTimerManager().ClearTimer(delayHandle);
	GetWorld()->GetTimerManager().ClearTimer(resetJumps);
	GetWorld()->GetTimerManager().ClearTimer(internalTimerHandle);
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

		//Walking
 		enhancedInputComponent->BindAction(walkAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomWalkPressed);
 		enhancedInputComponent->BindAction(walkAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomWalkReleased);	// stops the player from walking

 		//Sliding
 		enhancedInputComponent->BindAction(slideAction, ETriggerEvent::Started, this, &ALivreCharacter::CustomSlidePressed);
 		//enhancedInputComponent->BindAction(slideAction, ETriggerEvent::Completed, this, &ALivreCharacter::CustomSlideReleased);	// Stops the player from sliding and resets their values

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
		GetWorld()->GetTimerManager().SetTimer(resetJumps, [&]()
		{
			EventJumpReset(maxJump);
		}, 0.25f, false);
	}
	StopJumping();
}
// Sprinting functionality
void ALivreCharacter::CustomWalkPressed()
{
	if (!isWalking)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartSprint from CustomSprintPressed"));
	}
	isWalking = true;
	StartWalk();
}

void ALivreCharacter::CustomWalkReleased()
{
	if (isWalking)
	{
		UE_LOG(LogTemp, Warning, TEXT("StopSprint from CustomSprintReleased"));
	}
	isWalking = false;
	StopWalk();
}
//Sliding functionality
void ALivreCharacter::CustomSlidePressed()
{
	if (!GetCharacterMovement()->IsFalling() && !wasSlidingLongTime)
	{
     	GetWorld()->GetTimerManager().SetTimer(internalTimerHandle, [&]()
     	{
     		CustomSlideReleased();
     	}, slideTime, false);

		wasSlidingLongTime = true;
		
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
		LaunchCharacter(GetActorForwardVector() * slideForce, true, false);		
		//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Pressed"));
}

void ALivreCharacter::CustomSlideReleased()
{

	UKismetSystemLibrary::PrintString(this, "Sliding Happened After Animation");
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (wasSlidingLongTime)
	{
		wasSlidingLongTime = false;
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	}
	UE_LOG(LogTemp, Warning, TEXT("Custom Slide Released"));
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

// Setting walking speed
void ALivreCharacter::StartWalk(float newWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = newWalkSpeed;
}

// Changing walkin speed to running speed
void ALivreCharacter::StopWalk(float newNormalSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = newNormalSpeed;
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
  	
    	FVector speedByWallRunDirection = FVector(-cardinalForward, GetFirstPersonCameraComponent()->GetForwardVector().Z) * wallRunSpeed;
    	GetCharacterMovement()->Velocity = speedByWallRunDirection;
    }
	
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

void ALivreCharacter::SafeLevelReload()
{
	currentLevel->GetTimerManager().ClearAllTimersForObject(this);

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TSoftObjectPtr<UWorld>(currentLevel), false);
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
	/// capsule implementation of wall running
	if (!isWallRunning && wallToRunOn && hasLandedAfterWallRun)
	{
		GetWorld()->GetTimerManager().SetTimer(delayHandle, [&]()
		{
			// call End Wall Run
			EndWallRun(FallOff);
		}, 2, false);

		// sequence 1 where player runs on walls
		GetCharacterMovement()->AirControl = 1.0f;
		GetCharacterMovement()->GravityScale = 0.5f;
		
		isWallRunning = true;
		hasLandedAfterWallRun = false;
	}
}


void ALivreCharacter::CallEndWallRun()
{
	EndWallRun(FallOff);
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
		GetCharacterMovement()->GravityScale = initialGravity; 
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

void ALivreCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	EventJumpReset(maxJump);
	GetCharacterMovement()->bNotifyApex = true;
	GetCharacterMovement()->GravityScale = initialGravity;
	hasLandedAfterWallRun = true;

	UGameplayStatics::PlayWorldCameraShake(this, TSubclassOf<UCameraShakeBase>(), GetActorLocation(), 0.0f, 100.0f, 1.0f);
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