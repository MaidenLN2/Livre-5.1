// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <tuple>
#include "InputActionValue.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LivreCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class UInputMappingContext;

UCLASS(config=Game)
class ALivreCharacter : public ACharacter
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

protected:
	
	virtual void BeginPlay();
	
	/** Called for movement input */
	void Move(const FInputActionValue& value);

	/** Called for looking/moving input */
	void Look(const FInputActionValue& value);
	void MoveForward(float value);
	void MoveLateral(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* inputComponent) override;
	// End of APawn interface

	// Enums
	enum WallRunSide
	{
		Left = 0,
		Right
	};
	
	WallRunSide wallRunSide;

	enum WallRunEnd
	{
		FallOff = 0,
		JumpOff
	};
	
public:
	ALivreCharacter();

	// movement floats for accessibility in editor
	UPROPERTY(EditInstanceOnly, Category = "Movement Testing")
	float walkSpeed = 1000.0f;
	UPROPERTY(EditInstanceOnly, Category = "Movement Testing")
	float sprintSpeed = 5000.0f;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* lookAction;
	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* sprintAction;
	/** Slide Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* slideAction;
	/** Wallrun Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* wallrunAction;
	
	UPROPERTY(BlueprintReadWrite)
	float sensitivity = 1.0f;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	// InputAction Overrides
	void CustomJump();
	void CustomJumpEnded();
	void CustomSprintPressed();
	void CustomSprintReleased();
	void CustomSlidePressed();
	void CustomSlideReleased();
	void CustomVaultingPressed();
	void CustomVaultingReleased();
	
	// wall events
	void BeginWallRun();
	void CallEndWallRun();
	void EndWallRun(WallRunEnd why);
	
	//profile collision function
	FCollisionQueryParams GetIgnoreCharacterParams();
	
	//general functions
	void StartSprint(float newSprintSpeed = 1000.0f);
	void StopSprint(float newWalkSpeed = 600.0f);
	void SetHorizontalVelocity(float velocityX,float velocityY);
	void UpdateWallRun();
	void ClampHorizontalVelocity();	// expose to blueprints as this character doesn't have tick for some reason
	
	// pure functions/helper functions
	std::tuple<FVector, int> FindRunDirectionAndSide(FVector inputWallNormal);
	bool IsSurfaceWallRan(FVector surfaceVector);
	FVector LaunchVelocity();
	bool AreKeysRequired();
	FVector2d GetHorizontalVelocity();
	void EventJumpReset(int jumps);
	void EventAnyDamage(float damage);
	void EventOnLanded();
	bool LineTrace(FVector startPos, FVector endPos, EDrawDebugTrace::Type durationType, FHitResult& hitResult);
	
	// macros
	bool JumpUsed();

	// capsule collision event
	UFUNCTION()	// not sure if this will work, the CapsuleComponent is protected and therefore inaccessible
	void CapsuleTouched(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
		);

	
private:
	
	// capsule functionality
	UStaticMeshComponent* wallToRunOn;
	UPROPERTY(EditInstanceOnly)
	UCapsuleComponent* wallDetectionCapsule;

	UFUNCTION()	// not sure if this will work, the CapsuleComponent is protected and therefore inaccessible
	void WallDetectionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);
	
	UFUNCTION()	// not sure if this will work, the CapsuleComponent is protected and therefore inaccessible
	void WallDetectionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
	//health system
	bool isDead = false;
	
	// vector variables for wall climbing/running
	FVector wallLocation;
	FVector wallNormal;
	FVector wallHeight;
	FVector wallHeight2;
	FVector wallNormal2;
	FVector forwardImpulse;
	FVector velocity;

	// bools for wall climbing/running
	bool aboutToClimb = false;
	bool wallIsThick = false;
	bool canClimb = true;
	bool isClimbing = false;
	bool isSprinting = false;

	bool wantsToCrouch = false;
	bool orientRotationToMovement = false;

	// floats for wall climbing/running

	float health;
	float initialGravity;
	float fallingGravity;

	//sprint variables
	float defaultMaxWalkingSpeed = 0.0f;

	// wall running variables
	FVector wallRunDirection;
	FVector2D inputStorage;
	UPROPERTY(BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	bool isWallRunning;
	bool isUpdatingWallRun = false;
	int timeDelay = 5;
	int jumpLeft;
	const int maxJump = 2;
	float axisRight;
	float axisForward;	
};