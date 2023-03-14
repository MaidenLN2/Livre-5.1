#pragma once
#include "CoreMinimal.h"
#include "Livre/LivreCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LivreMovementComponent.generated.h"

UENUM(BlueprintType)
enum CustomMovementMode
{
	CMOVE_None	UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_MAX	UMETA(Hidden),
};


UCLASS()
class ULivreMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_LivreCharacter : FSavedMove_Character
	{
		uint8 Saved_bPrevWantsToCrouch:1;
	};

	//move variables for the rest of movement here?

	//additional parametres for sliding
	UPROPERTY(EditDefaultsOnly) float slideMinSpeed = 350;
	UPROPERTY(EditDefaultsOnly) float slideEnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly) float slideGravityForce = 5000;
	UPROPERTY(EditDefaultsOnly) float slideFriction = 1.3;
	
	// transient
	UPROPERTY(Transient) ALivreCharacter* LivreCharacterOwner;

private:
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 iterations);
	bool isGettingSlideSurface(FHitResult& hit) const;
	
protected:
virtual void InitializeComponent() override;

public:
	ULivreMovementComponent();

	// for changing movement mode
	UFUNCTION(BlueprintPure) bool isCustomMovementMode(enum CustomMovementMode InCustomMovementMode) const;
	
};
