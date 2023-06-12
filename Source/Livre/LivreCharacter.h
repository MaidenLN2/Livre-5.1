// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <tuple>
#include "InputActionValue.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TimelineComponent.h"
#include "LivreCharacter.generated.h"

// default classes
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	
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
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float normalSpeed = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Testing")
	float walkSpeed = 250.0f;
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float wallRunSpeed = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Testing")
	float dashForce = 2500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Testing")
	float dashTime = 0.7f;
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float slideForce = 1500.0f;
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float slideTime = 0.7f;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* lookAction;
	/** Walk Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* walkAction;
	/** Dash Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* dashAction;
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
	void CustomWalkPressed();
	void CustomWalkReleased();
	void CustomDashPressed();
	//void CustomDashReleased();
	void CustomSlidePressed();
	void CustomSlideReleased();
	void BeginCameraTiltWall();
	void EndCameraTiltWall();
	
	UFUNCTION(BlueprintCallable)
	void ProcessDash();
	
	// wall events
	void BeginWallRun();
	void CallEndWallRun();
	void EndWallRun(WallRunEnd why);
	
	//profile collision function
	FCollisionQueryParams GetIgnoreCharacterParams();
	
	// timer and world global variables
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int time = 180;
	FTimerHandle timeLimit;
	FTimerHandle delayHandle;
	FTimerHandle resetJumps;
	FTimerHandle internalTimerHandle;
	UWorld* currentLevel;

	// timeline functions
	UFUNCTION()
	void ProcessCameraTimeline(float timelineProgress);
	
	// timeline setup
	FTimeline UpdateCameraTiltTimeline;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UCurveFloat* UpdateCameraTiltCurve;
	
	//general functions
	void StartWalk(float newWalkSpeed = 1000.0f);
	void StopWalk(float newNormalSpeed = 10000.0f);
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
	bool LineTrace(FVector startPos, FVector endPos, EDrawDebugTrace::Type durationType, FHitResult& hitResult);
	void SafeLevelReload();

	// Health
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DealDamageToPlayer(int DamageToDeal);
	
	// macros
	bool JumpUsed();

	// capsule collision event
	UFUNCTION()	// not sure if this will work, the CapsuleComponent is protected and therefore inaccessible
	void CapsuleTouched(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult &SweepResult
		);

	
private:
	
	// capsule functionality
	UStaticMeshComponent* wallToRunOn;
	UPROPERTY(EditInstanceOnly)
	UCapsuleComponent* wallDetectionCapsule;

	UFUNCTION()	
	void WallDetectionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);
	
	UFUNCTION()	
	void WallDetectionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyJumpApex() override;
	
	//health system
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int health = 5;
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
	bool isWalking = false;
	bool hasLandedAfterWallRun = true;
	bool wasSlidingLongTime = false;
	bool isWallRunningLong = false;
	bool wantsToCrouch = false;
	bool orientRotationToMovement = false;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool isDashing = false;

	// floats for wall climbing/running
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float initialGravity = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float jumpGravity = 1.0f;
	float fallingGravity;

	//walk variables
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
	UPROPERTY(EditAnywhere, Category = "Movement Testing")
	float wallrunTime = 5.0;
};

inline void ALivreCharacter::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (UpdateCameraTiltTimeline.IsPlaying())
	{
		UpdateCameraTiltTimeline.TickTimeline(deltaSeconds);
	}
}

inline void ALivreCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();

	if (!isWallRunning)
	{
		GetCharacterMovement()->GravityScale = jumpGravity;
	}
}

/// 