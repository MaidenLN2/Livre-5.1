// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <tuple>
#include "InputActionValue.h"
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

	
public:
	ALivreCharacter();

protected:
	virtual void BeginPlay();

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	UPROPERTY(BlueprintReadWrite)
	float sensitivity = 1.0f;

	/** Bool for AnimBP to switch to another animation set 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;*/

	/** Setter to set the bool 
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);*/

	/** Getter for the bool 
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();*/

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void MoveForward(float value);
	void MoveLateral(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	// ENUMS
	enum WallRunSide
	{
		Left = 0,
		Right
	};
	
	WallRunSide WallRunSide;

	enum WallRunEnd
	{
		FallOff = 0,
		JumpOff
	};

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	// InputAction Overrides
	void CustomJump();
	void CustomSprintPressed();
	void CustomSprintReleased();
	void CustomSlidePressed();
	void CustomVaultingPressed();
	
	//general functions
	void StartSprint(float NewSprintSpeed = 1000.0f);
	void StopSprint(float NewWalkSpeed = 600.0f);
	void SetHorizontalVelocity(float velocityX,float velocityY);
	void UpdateWallRun();
	void ClampHorizontalVelocity();	// Expose to blueprints as this character doesn't have tick for some reason.
	
	// pure functions
	std::tuple<FVector, int> FindRunDirectionAndSide(FVector InputWallNormal);
	bool IsSurfaceWallRan(FVector surfaceVector);
	FVector LaunchVelocity();
	bool AreKeysRequired();
	FVector2d GetHorizontalVelocity();

	// macros
	bool JumpUsed();

	// events
	void EventJumpReset(int Jumps);
	void EventAnyDamage(float Damage);
	void EventOnLanded();
	// collision event
	UFUNCTION()	// not sure if this will work, the CapsuleComponent is protected and therefore inaccessible. Am hoping this overrides its base collision detection
	void CapsuleTouched(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
		);
	// ervents continued
	void BeginWallRun();
	void EndWallRun(WallRunEnd Why);
	
private:

	// vector variables for wall climbing/running
	FVector wallLocation;
	FVector wallNormal;
	FVector wallHeight;
	FVector wallHeight2;
	FVector wallNormal2;
	FVector forwardImpulse;

	// bools for wall climbing/running
	bool aboutToClimb = false;
	bool wallIsThicc = false;
	bool canClimb = true;
	bool isClimbing = false;
	bool isSprinting = false;
	bool isDead = false;

	// floats for wall climbing/running
	float walkSpeed;
	float sprintSpeed;
	float health;
	float initialGravity;
	float fallingGravity;

	// wall running variables
	FVector wallRunDirection;
	bool isWallRunning;
	int jumpLeft;
	int maxJump;
	float axisRight;
	float axisForward;

	// UPROPERTY(EditInstanceOnly)
	void Landed(const FHitResult& Hit) override;
};