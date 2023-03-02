// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreCharacter.h"
#include "LivreProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ALivreCharacter is LUIZZZZZZ

ALivreCharacter::ALivreCharacter()
{
	// Character doesnt have a rifle at start
	//bHasRifle = false;
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

	// Connecting Collision Detection Functions
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ALivreCharacter::CapsuleTouched);	// Might work?

}

void ALivreCharacter::BeginPlay()
{
	// call the base class  
	Super::BeginPlay();
	printf("BeginPlay()");

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
	printf("jump reset");
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	printf("GetCharacterMovement()->SetPlaneConstraintEnabled(false)");

	//USkinnedMeshComponent::HideBoneByName(Neck, PBO_None); // might need to be a BP specific function

}

//////////////////////////////////////////////////////////////////////////// Input

 void ALivreCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
 {
 	// set up action bindings
 	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
 	{


 		//jumping
 		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);	// Original for Storage
 		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ALivreCharacter::CustomJump);
 		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

 		
 		printf("jumping");

// 		//moving
 		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALivreCharacter::Move);
 		printf("moving");

// 		//looking
 		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALivreCharacter::Look);
 		printf("looking");

 		PlayerInputComponent->BindAxis("Forward", this, &ALivreCharacter::MoveForward);
 		PlayerInputComponent->BindAxis("Right", this, &ALivreCharacter::MoveLateral);
 		PlayerInputComponent->BindAxis("MouseX", this, &ALivreCharacter::LookHorizontal);
 		PlayerInputComponent->BindAxis("MouseY", this, &ALivreCharacter::LookVertical);
 		PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ALivreCharacter::Jump);
 	}
 }

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

void ALivreCharacter::CustomJump()
{
	if (JumpUsed())
	{
		LaunchCharacter(LaunchVelocity(), false, true);

		if (isWallRunning)
		{
			EndWallRun(JumpOff);
		}
	}
	printf("CustomJum()");
}

void ALivreCharacter::CustomSprintPressed()
{
	isSprinting = true;
	StartSprint();
	printf("CustomSprintPressed");
}

void ALivreCharacter::CustomSprintReleased()
{
	isSprinting = false;
	StopSprint();
	printf("zcustomSprintReleased");
}

void ALivreCharacter::CustomSlidePressed()
{
	if (isSprinting && !GetCharacterMovement()->IsFalling())
	{
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		AddMovementInput(FVector(), 200.0f, false);


		// Sliding Anim Goes Here
		// float Duration = PlayAnimMontage(/*Insert Anim Montage Here*/);
		float Duration = 5.0f;

		FTimerHandle CSP_TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(CSP_TimerHandle, [&]()
		{
			UKismetSystemLibrary::PrintString(this, "Sliding Happened After Animation");
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);

			FTimerHandle CSP_InternalTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(CSP_InternalTimerHandle, [&]()
			{
				GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
			}, 0.5f, false);
		}, Duration, false);
	}
	printf("custom slide pressed");
}

void ALivreCharacter::CustomVaultingPressed()
{
	FVector StartPos = GetActorLocation() - FVector(0.0, 0.0, 44.0);
	FVector EndPos = StartPos + (GetActorForwardVector() * 70.0f);
	FHitResult HitTracking;
	// look more in here
	bool FirstLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		StartPos,
		EndPos,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		true,
		TArray<AActor*>(),
		EDrawDebugTrace::ForDuration,
		HitTracking,
		true
		);

	if (FirstLineTraceDidHit)
	{
		wallLocation = HitTracking.Location;
		wallNormal = HitTracking.Normal;
		
		FVector EndPos2 = (
			wallLocation +
			(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -10.0f)
			);
		FVector StartPos2 = EndPos2 - FVector(0.0f, 0.0f, 200.0f);
		FHitResult HitTracking2;
		
		bool SecondLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
			this,
			StartPos2,
			EndPos2,
			TArray<TEnumAsByte<EObjectTypeQuery>>(),
			true,
			TArray<AActor*>(),
			EDrawDebugTrace::ForDuration,
			HitTracking2,
			true
			);

		if (SecondLineTraceDidHit)
		{
			wallHeight = HitTracking2.Location;
			// sequence 0
			aboutToClimb = ((wallHeight - wallLocation).Z > 60.0f);

			// sequence 1
			FVector StartPos3 = (
				wallLocation +
				(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * -50.0f) +
				FVector(0.0f, 0.0f, 250.0f)
				);
			FVector EndPos3 = StartPos3 - FVector(0.0f, 0.0f, 300.0f);
			FHitResult HitTracking3;

			bool ThirdLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
				this,
				StartPos3,
				EndPos3,
				TArray<TEnumAsByte<EObjectTypeQuery>>(),
				true,
				TArray<AActor*>(),
				EDrawDebugTrace::ForDuration,
				HitTracking3,
				true
				);

			if (ThirdLineTraceDidHit)
			{
				wallHeight2 = HitTracking3.Location;

				wallIsThicc = (wallHeight - wallHeight2).Z <= 30.0f;	//  negation was removed for simplicity
			}
			else
			{
				wallIsThicc = false;
			}

			if (aboutToClimb)
			{
				// sequence 0
				FVector StartPos4 = GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
				
				bool FourthLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
					this,
					StartPos4,
					StartPos4 + GetActorForwardVector() * 70.0f,
					TArray<TEnumAsByte<EObjectTypeQuery>>(),
					true,
					TArray<AActor*>(),
					EDrawDebugTrace::ForDuration,
					HitTracking3,
					true
					);

				if (FourthLineTraceDidHit) canClimb = false;
				
				// this last trace may not be necessary, none of the outputs are used.
				bool FifthLineTraceDidHit = UKismetSystemLibrary::LineTraceSingleForObjects(
					this,
					GetActorLocation(),
					GetActorLocation() + FVector(0.0f, 0.0f, 200.0f),
					TArray<TEnumAsByte<EObjectTypeQuery>>(),
					true,
					TArray<AActor*>(),
					EDrawDebugTrace::ForDuration,
					HitTracking3,
					true
					);

				// sequence 1
				if (canClimb)
				{
					GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					GetCharacterMovement()->SetMovementMode(MOVE_Flying);
					SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
					
					SetActorLocation(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f + GetActorLocation());
					SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 44.0f));
					
					// play anim montage here
					// Duration = PlayAnimMontage();
					float Duration = 4.0f;

					UKismetSystemLibrary::PrintString(this, "landing happened after animation");

					FTimerHandle CVP_ClimbTimerHandle;
					GetWorld()->GetTimerManager().SetTimer(CVP_ClimbTimerHandle, [&]()
					{
						GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						GetCharacterMovement()->SetMovementMode(MOVE_Walking);
					}, Duration, false);
				}
			}
			else
			{
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				GetCharacterMovement()->SetMovementMode(MOVE_Flying);

				FVector NewActorLocation = (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(wallNormal)) * 50.0f) + GetActorLocation();
				SetActorLocation(NewActorLocation);

				float Duration;
				if (wallIsThicc)
				{
					// NOTICE 
					// getting Up Animation goes here
					// Duration = PlayAnimMontage();
					Duration = 4.0f;
				}
				else
				{
					SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, wallHeight.Z - 20.0f));
					
					// Duration = PlayAnimMontage();
					Duration = 3.0f;
					UKismetSystemLibrary::PrintString(this, "vaulting happened after animation");
				}

				FTimerHandle CVP_TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(CVP_TimerHandle, [&]()
				{
					GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					GetCharacterMovement()->SetMovementMode(MOVE_Walking);
					SetActorRotation(FRotator(GetActorRotation().Roll, GetActorRotation().Pitch, UKismetMathLibrary::MakeRotFromX(wallNormal).Yaw + 180.0f));
				}, Duration, false);
			}
		}
	}
}

void ALivreCharacter::StartSprint(float NewSprintSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSprintSpeed;
	printf("start sprint");
}

void ALivreCharacter::StopSprint(float NewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
	printf("stop sprint");
}

void ALivreCharacter::SetHorizontalVelocity(float velocityX, float velocityY)
{
	float z = GetCharacterMovement()->Velocity.Z;
	GetCharacterMovement()->Velocity = FVector(velocityX, velocityY, z);
	printf("set horizontal velocity");
}

void ALivreCharacter::UpdateWallRun()
{
    if (AreKeysRequired())
    {
        FHitResult HitResultCapture;
        FVector TraceEnd = GetActorLocation() + (GetActorRightVector() * (250 * (WallRunSide == Left ? 1 : -1)));
        bool LineTraceDidHit = UKismetSystemLibrary::LineTraceSingle(
            this,
            GetActorLocation(),
            TraceEnd,
            UEngineTypes::ConvertToTraceType(ECC_Visibility),
            false,
            TArray<AActor*>(),
            EDrawDebugTrace::None,
            HitResultCapture,
            true
            );

        if (LineTraceDidHit)
        {
            auto [Direction, Side] = FindRunDirectionAndSide(HitResultCapture.ImpactNormal);

            if (Side == WallRunSide)
            {
                wallRunDirection = Direction;

                FVector SpeedByWallRunDirection = wallRunDirection * GetCharacterMovement()->GetMaxSpeed();
                GetCharacterMovement()->Velocity = FVector(SpeedByWallRunDirection.X, SpeedByWallRunDirection.Y, 0.0f);
            }
            else
            {
            	EndWallRun(FallOff);
            }
        }
        else
        {
        	EndWallRun(FallOff);
        }
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
	printf("clamp horizontal velocity");
}

std::tuple<FVector, int> ALivreCharacter::FindRunDirectionAndSide(FVector InputWallNormal)
{
	enum WallRunSide SideLocal;
    FVector2d test1(InputWallNormal);
    FVector2d test2(GetActorRightVector());

    float DotProd = FVector2d::DotProduct(test1, test2);

    float zFlip = 1;
    if  (DotProd > 0)
    {
        SideLocal = Right;
    }
    else
    {
        SideLocal = Left;
        zFlip *= -1;
    }

    FVector CrossProduct = FVector::CrossProduct(InputWallNormal, FVector(0, 0, zFlip));
	printf("find run direction and side");
    return std::tuple(CrossProduct, SideLocal);
}

bool ALivreCharacter::IsSurfaceWallRan(FVector surfaceVector)
{
    if (surfaceVector.Z < -0.05f)
    {
        return false;
    }
    
    FVector DotComp = FVector(surfaceVector.X, surfaceVector.Y, 0.0f);
    DotComp.Normalize();

    float DotResult = FVector::DotProduct(DotComp, surfaceVector);
    float ArcCosDotResult = UKismetMathLibrary::DegAcos(DotResult);    // KismetMathLibrary broooo
    
    float WalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	printf("is surface wall run");
    return (ArcCosDotResult < WalkableFloorAngle);
}

FVector ALivreCharacter::LaunchVelocity()
{
    FVector LaunchDirection;
    // sequence 0
    if (isWallRunning)
    {
        FVector CrossComp = FVector(0, 0, WallRunSide == Left ? 1 : -1 );

        LaunchDirection = FVector::CrossProduct(wallRunDirection, CrossComp);
    }
    else
    {
        if (GetCharacterMovement()->IsFalling())
        {
            LaunchDirection = (GetActorRightVector() * axisRight) + (GetActorForwardVector() * axisForward);
        }
    }
	printf("launch velocity");
    // sequence 1
    return (LaunchDirection + FVector(0, 0, 1)) * GetCharacterMovement()->JumpZVelocity;
}

bool ALivreCharacter::AreKeysRequired()
{
    if (axisForward > 0.1f)
    {
        switch (WallRunSide)
        {
        case Left:
            return axisRight > 0.1f;
        case Right:
            return axisRight < 0.1f;
        default: ;
        }
    }
	printf("are keys required");
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
	printf("jump used");
	return false;
}

void ALivreCharacter::EventJumpReset(int Jumps)
{
	jumpLeft = UKismetMathLibrary::Clamp(Jumps, 0, maxJump);
}

void ALivreCharacter::EventAnyDamage(float Damage)
{
	health -= Damage;

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

void ALivreCharacter::CapsuleTouched(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (isWallRunning) return;

	if (IsSurfaceWallRan(SweepResult.ImpactNormal) && GetCharacterMovement()->IsFalling())
	{
		auto [OutDir, OutSide] = FindRunDirectionAndSide(SweepResult.ImpactNormal);
		wallRunDirection = OutDir;

		switch (OutSide)
		{
		case Left:
			WallRunSide = Left;
			break;
		case Right:
			WallRunSide = Right;
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
	// sequence 0
	FTimerHandle BWR_Delayhandle;
	GetWorld()->GetTimerManager().SetTimer(BWR_Delayhandle, [&]()
	{
		// call End Wall Run
	}, 2, false);
	
	// sequence 1
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->GravityScale = 0.0f;

	isWallRunning = true;
	//  place for camera tilt begin

	// Timeline the UpdateWallRun function for 5 seconds
}

void ALivreCharacter::EndWallRun(WallRunEnd Why)
{
	switch (Why)
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
	GetCharacterMovement()->GravityScale = 1.0f;
	isWallRunning = false;

	//  place for camera tilt end


}

void ALivreCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALivreCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

