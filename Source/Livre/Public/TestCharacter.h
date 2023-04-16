// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "TestCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UENUM()
enum EWallRunSide
{
	Left,
	Right
};

UENUM()
enum EWallRunEnd
{
	FallOff,
	JumpOff
};

UCLASS(config=Game)
class ADefaultTestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SprintAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SlideAction;

	
public:
	ADefaultTestCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	// FUNCTIONS
	// Helper Functions
	bool LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const;

	// Ex-Macros
	bool JumpUsed();
	
	// VARIABLES

public:
	// FUNCTIONS
	// Putting all here for now, can shift to private as needed
	void StartSprint();
	void StopSprint();
	std::tuple<FVector, EWallRunSide> FindRunDirectionAndSide(FVector WallNormal);
	bool IsSurfaceWallRan(FVector SurfaceNormal);
	FVector LaunchVelocity();
	bool AreKeysRequired();
	FVector2D GetHorizontalVelocity();
	void SetHorizontalVelocity(FVector2D HorizontalVelocity);
	void UpdateWallRun();
	void ClampHorizontalVelocity();

	// Functions that use Events as start nodes instead of being functions :/
	void JumpReset(int NewJumpsValue);
	void BeginWallRun();
	void EndWallRun(EWallRunEnd Why);

	// Functions bound to InputActions (UE4 Version)
	// Jump
	void CustomJumpStart();
	// Slide
	void CustomSlideStart();
	// Sprint
	void CustomSprintStart();
	void CustomSprintEnd();
	// Any Damage - Not implemented due to lack of health implementation

	// Functions bound to events that need to be delegates for Unreal Functions
	// OnLanded
	virtual void Landed(const FHitResult& Hit) override;
	// On Capsule Hit
	UFUNCTION()
	void CapsuleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
		);

	// Timeline Functions
	UFUNCTION()
	void ProcessTimeline(float TimelineProgress);

	// VARIABLES
	// Timeline Setup
	FTimeline UpdateWallRunTimeline;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UCurveFloat* UpdateWallRunCurve;

	// TimerHandle to reset wall running
	FTimerHandle WallRunDurationTimerHandle;
	FTimerHandle EndSlideTimerHandle;

	// Normal Variables
	const float WalkSpeed = 600.0f;
	const float SprintSpeed = 1000.0f;

	// For Wall Running Category
	FVector WallRunDirection;
	bool IsWallRunning = false;
	int JumpLeft = 0;
	const int MaxJump = 1;
	float AxisRight = 0.0f;
	float AxisForward = 0.0f;
	EWallRunSide WallRunSide_Global = Left;

	// For Default Category
	bool IsSprinting = false;
	const float InitialGravity = 1.0f;
	const float FallingGravity = 2.0f;
};
