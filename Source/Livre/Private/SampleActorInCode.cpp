// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleActorInCode.h"

#include "Kismet/GameplayStatics.h"
#include "Livre/LivreCharacter.h"

// Sets default values
ASampleActorInCode::ASampleActorInCode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Mesh Setup
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = SceneRoot;

	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Base"));
	Base->SetupAttachment(RootComponent);
	ConstructorHelpers::FObjectFinder<UStaticMesh> BaseObj(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube'"));
	Base->SetStaticMesh(BaseObj.Object);
	//Base->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	//Base->SetWorldScale3D(FVector(5.0f, 5.0f, 0.25f));

	// Maglifts = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Maglifts"));
	// Maglifts->SetupAttachment(Base);
	// ConstructorHelpers::FObjectFinder<UStaticMesh> MagliftObj(TEXT("StaticMesh'/Game/VericalSlice/Train/Handcart/Handcart_Merged_Maglifts.Handcart_Merged_Maglifts'"));	// Needs path using current project
	// Maglifts->SetStaticMesh(MagliftObj.Object);
	// Maglifts->SetRelativeLocation(FVector(0.0f, 368.0f, 0.0f));

	// Middle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Pole"));
	// Middle->SetupAttachment(RootComponent);
	// Middle->SetStaticMesh(MeshObj.Object);
	// Middle->SetWorldScale3D(FVector(0.1f, 0.1f, 2.0f));

	// Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Handle"));
	// Handle->SetupAttachment(Base);
	// ConstructorHelpers::FObjectFinder<UStaticMesh> HandleObj(TEXT("StaticMesh'/Game/VericalSlice/Train/Handcart/Handcart_Merged_Handles.Handcart_Merged_Handles'")); // Needs path using current project
	// Handle->SetStaticMesh(HandleObj.Object);
	// Handle->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	// // Handle->SetWorldScale3D(FVector(1.5f, 0.05f, 0.05f));
	// Handle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(RootComponent);
	Arrow->SetArrowColor(FColor::Purple);
	Arrow->SetHiddenInGame(false);
	Arrow->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	
	// End Mesh Setup
	// Start Collision Setup
	FrontCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontCollider"));
	FrontCollision->SetupAttachment(RootComponent);
	FrontCollision->SetRelativeLocation(FVector(80.0f, 0.0f, 150.0f));
	FrontCollision->SetWorldScale3D(FVector(1.25f, 3.0f, 2.0f));
	// FrontCollision->SetHiddenInGame(false);
	
	BackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackCollider"));
	BackCollision->SetupAttachment(RootComponent);
	BackCollision->SetRelativeLocation(FVector(-80.0f, 0.0f, 150.0f));
	BackCollision->SetWorldScale3D(FVector(1.25f, 3.0f, 2.0f));
	// BackCollision->SetHiddenInGame(false);
	// End Collision Setup
	// Start Collision Assignment
	FrontCollision->OnComponentBeginOverlap.AddDynamic(this, &ASampleActorInCode::BeginFrontOverlap);
	FrontCollision->OnComponentEndOverlap.AddDynamic(this, &ASampleActorInCode::EndFrontOverlap);
	
	BackCollision->OnComponentBeginOverlap.AddDynamic(this, &ASampleActorInCode::BeginBackOverlap);
	BackCollision->OnComponentEndOverlap.AddDynamic(this, &ASampleActorInCode::EndBackOverlap);
	// End Collision Assignment
}

// Called when the game starts or when spawned
void ASampleActorInCode::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = UGameplayStatics::GetActorOfClass(GetWorld(), ALivreCharacter::StaticClass());
	GEngine->AddOnScreenDebugMessage(15, 5, FColor::Green, FString::Printf(TEXT("Player Ref populated: %d"), PlayerRef != nullptr));
	
}

// Called every frame
void ASampleActorInCode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASampleActorInCode::BeginFrontOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	
}

void ASampleActorInCode::EndFrontOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ASampleActorInCode::BeginBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ASampleActorInCode::EndBackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

