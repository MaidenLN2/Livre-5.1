// Copyright Epic Games, Inc. All Rights Reserved.

#include "LivreCharacter.h"
#include "LivreProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	return std::tuple(FVector(), int());
}

bool ALivreCharacter::IsSurfaceWallRan(FVector surfaceVector)
{
	return false;
}

FVector ALivreCharacter::LaunchVelocity()
{
	return FVector();
}

bool ALivreCharacter::AreKeysRequired()
{
	return false;
}

FVector2d ALivreCharacter::GetHorizontalVelocity()
{
	return FVector2d();
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

