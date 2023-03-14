#pragma once
#include "CoreMinimal.h"
#include "Livre/LivreCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LivreMovementComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None	UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_MAX	UMETA(Hidden),
};


UCLASS()
class ULivreMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Livre : public FSavedMove_Character // helper class for keeping state data in movement component per frame
	{
		FSavedMove_Livre();

		typedef FSavedMove_Character Super;

		uint8 saved_aboutToSprint : 1; // sprint state for switching flags

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
		
	};

	// variable for saving sprint state/checking further in code
	bool safe_aboutToSprint;

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

	virtual uint8 GetCompressedFlags() const /*override*/;

public:
	ULivreMovementComponent();

	// for changing movement mode
	UFUNCTION(BlueprintPure) bool isCustomMovementMode(enum ECustomMovementMode InCustomMovementMode) const;
	
};
