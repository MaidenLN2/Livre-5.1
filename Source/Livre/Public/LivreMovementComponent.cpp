
#include "LivreMovementComponent.h"
#include "Components/CapsuleComponent.h"

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

ULivreMovementComponent::ULivreMovementComponent()
{
	LivreCharacterOwner = Cast<ALivreCharacter>(GetOwner());
}

bool ULivreMovementComponent::isCustomMovementMode(::CustomMovementMode InCustomMovementMode) const
{
	return (MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode);
}
