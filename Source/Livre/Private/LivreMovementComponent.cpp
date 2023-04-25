
#include "LivreMovementComponent.h"
#include "Components/CapsuleComponent.h"

bool ULivreMovementComponent::FSavedMove_Livre::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Livre* NewLivreMove = static_cast<FSavedMove_Livre*>(NewMove.Get());

	if (saved_aboutToSprint != NewLivreMove->saved_aboutToSprint)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void ULivreMovementComponent::FSavedMove_Livre::Clear()
{
	FSavedMove_Character::Clear();

	saved_aboutToSprint= 0;
}

uint8 ULivreMovementComponent::FSavedMove_Livre::GetCompressedFlags() const
{
	return FSavedMove_Character::GetCompressedFlags();
}

void ULivreMovementComponent::FSavedMove_Livre::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
}

void ULivreMovementComponent::FSavedMove_Livre::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);
}

void ULivreMovementComponent::EnterSlide()
{
	bWantsToCrouch = true; // UE5 bool
	Velocity += Velocity.GetSafeNormal2D() * slideEnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void ULivreMovementComponent::ExitSlide()
{
	bWantsToCrouch = false; //UE5 bool
	FQuat newRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult hitResult;
	SafeMoveUpdatedComponent(FVector::ForwardVector, newRotation, true, hitResult);
	SetMovementMode(MOVE_Walking);
}
//here we go, the pain of game physics allowing slide to be effected by gravity and forces
void ULivreMovementComponent::PhysSlide(float deltaTime, int32 iterations)
{
	if (deltaTime <MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity(); // restores velocity during root motion, not really needed

	FHitResult surfaceHit;

	if (!isGettingSlideSurface(surfaceHit) || Velocity.SizeSquared() < pow(slideMinSpeed, 2))
	{
		ExitSlide();
		StartNewPhysics(deltaTime, iterations);
		return;
	}

	// surface gravity =
	Velocity += slideGravityForce * FVector::DownVector * deltaTime;

	//strafing god i hate math why am i even here
	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) >0.5)
	{
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else
	{
		Acceleration = FVector::ZeroVector;
	}

	// velocity calculation
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, slideFriction,true, GetMaxBrakingDeceleration());
	}

	// moving is happening (sliding is happening)
	iterations++;
	bJustTeleported = false; // that's UE5 boolean, that's why it looks different plz ignore until

	// set of variables for further calculations
	FVector oldLocation = UpdatedComponent->GetComponentLocation();
	FQuat oldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult hitRes(1.0f);
	FVector adjusted = Velocity * deltaTime;
	FVector velocityPlaneDirection = FVector::VectorPlaneProject(Velocity, surfaceHit.Normal).GetSafeNormal();
	FQuat newRotation = FRotationMatrix::MakeFromXZ(velocityPlaneDirection, surfaceHit.Normal).ToQuat();
	SafeMoveUpdatedComponent(adjusted, newRotation, true, hitRes);

	if (hitRes.Time < 1.0f)
	{
		HandleImpact(hitRes, deltaTime, adjusted);
		SlideAlongSurface(adjusted, (1.0f - hitRes.Time), hitRes.Normal, hitRes, true);
	}

	FHitResult newSurfaceHitResult;
	if (!isGettingSlideSurface(newSurfaceHitResult) || Velocity.SizeSquared() < pow(slideMinSpeed, 2))
	{
		ExitSlide();
	}

	// update acceleration and velocity
	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - oldLocation) / deltaTime;
	}
}

bool ULivreMovementComponent::isGettingSlideSurface(FHitResult& hit) const
{
	FVector start = UpdatedComponent->GetComponentLocation();
	FVector end = start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f *FVector::DownVector;
	FName profileName =  TEXT("Block all");
	return GetWorld()->LineTraceSingleByProfile(hit, start, end,profileName, LivreCharacterOwner->GetIgnoreCharacterParams());
}

void ULivreMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

uint8 ULivreMovementComponent::GetCompressedFlags() const
{
	// uint8 result =FSavedMove_Livre::GetCompressedFlags();
	// if (saved_aboutToSprint) result |= FLAG_CUSTOM_0;

	// return result;
	return false;
}

ULivreMovementComponent::ULivreMovementComponent()
{
	LivreCharacterOwner = Cast<ALivreCharacter>(GetOwner());
}

bool ULivreMovementComponent::isCustomMovementMode(::ECustomMovementMode InCustomMovementMode) const
{
	return (MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode);
}
