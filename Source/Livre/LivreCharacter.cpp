// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreCharacter.h"
#include "LivreProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//////////////////////////////////////////////////////////////////////////
// ALivreCharacter is LUIZZZZZZ

ALivreCharacter::ALivreCharacter()
{
	// Character doesnt have a rifle at start
	//bHasRifle = false;
	
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

}

void ALivreCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

//////////////////////////////////////////////////////////////////////////// Input

void ALivreCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALivreCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALivreCharacter::Look);
	}
}

void ALivreCharacter::StartSprint(float NewSprintSpeed)
{
	
	GetCharacterMovement()->MaxWalkSpeed = NewSprintSpeed;
}

void ALivreCharacter::StopSprint(float NewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
}

void ALivreCharacter::SetHorizontalVelocity(float velocityX, float velocityY)
{
	float z = GetCharacterMovement()->Velocity.Z;
	GetCharacterMovement()->Velocity = FVector(velocityX, velocityY, z);
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
                // Call End Wall Run Event Here - Reason is FallOff
            }
        }
        else
        {
            // Call End Run Event here. Reason is FallOff
        }
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
    float ArcCosDotResult = UKismetMathLibrary::DegAcos(DotResult);    // Had to check the old project to see what it used to get this function. Answer is the KismetMathLibrary
    
    float WalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
    
    return (ArcCosDotResult < WalkableFloorAngle);
}

FVector ALivreCharacter::LaunchVelocity()
{
    FVector LaunchDirection;
    // Sequence 0
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

    // Sequence 1
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
    return false;
}

FVector2d ALivreCharacter::GetHorizontalVelocity()
{
    return FVector2d(GetCharacterMovement()->GetLastUpdateVelocity());
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

