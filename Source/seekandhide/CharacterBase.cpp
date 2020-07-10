// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Engine.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);

	// Configure character movement
	CharacterMovement->bOrientRotationToMovement = false; // Character doesn't move in the direction of input...	
	CharacterMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	CharacterMovement->JumpZVelocity = 600.f;
	CharacterMovement->AirControl = 0.2f;
	WalkSpeed = CharacterMovement->MaxWalkSpeed;
	SprintSpeed = WalkSpeed * SprintSpeedMultiplier;

	//Configure camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(GetMesh(), "Head");
	FollowCamera->SetRelativeLocationAndRotation(FVector(0.0f, -0.3f, 0.3f), FRotator(100.0f, 90.0f, 0.0f));
	FollowCamera->SetRelativeScale3D(FVector(0.0125f, 0.0125f, 0.0125f));
	FollowCamera->SetFieldOfView(103.0f);
	FollowCamera->bUsePawnControlRotation = true;

	//Creating SlidingTimeline
	SlidingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	//Set replication
	bReplicates = false;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Check if curve asset refrence is valid
	if (SlidingCurve)
	{
		FOnTimelineFloat TimelineCallback;
		FOnTimelineEvent TimelineFinishedCallback;

		TimelineCallback.BindUFunction(this, FName("Slide"));
		TimelineFinishedCallback.BindUFunction(this, FName("ResolveMovement"));

		SlidingTimeline->AddInterpFloat(SlidingCurve, TimelineCallback);
		SlidingTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);
	}
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Control sliding
	//SlidingOnSlope();
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACharacterBase::MoveRight);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacterBase::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ACharacterBase::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ACharacterBase::StopSprinting);

	PlayerInputComponent->BindAction("Slide", IE_Pressed, this, &ACharacterBase::StartSliding);
	PlayerInputComponent->BindAction("Slide", IE_Released, this, &ACharacterBase::StopSliding);

}

void ACharacterBase::MoveForward(float Value)
{
	if (!CheckMovementState(Sliding))
	{
		if (!CheckMovementState(Sprinting))
		{
			SetMovementState(Walking);
		}

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACharacterBase::MoveRight(float Value)
{
	if (!CheckMovementState(Sliding))
	{
		if (!CheckMovementState(Sprinting))
		{
			SetMovementState(Walking);
		}

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ACharacterBase::Jump()
{
	if (!CheckMovementState(Sliding))
	{
		ACharacter::Jump();
	}
}

void ACharacterBase::StartSprinting()
{
	SprintKeyDown = true;

	if (CheckMovementState(Standing) || CheckMovementState(Walking))
	{
		SetMovementState(Sprinting);
		CharacterMovement->MaxWalkSpeed = SprintSpeed;
		SetOnServerMovementSpeed(SprintSpeed);
	}
}

void ACharacterBase::StopSprinting()
{
	SprintKeyDown = false;

	if (CheckMovementState(Sprinting))
	{
		SetMovementState(Standing);
		CharacterMovement->MaxWalkSpeed = WalkSpeed;
		SetOnServerMovementSpeed(WalkSpeed);
	}
}

void ACharacterBase::StartSliding()
{
	SlideKeyDown = true;

	if (!CheckMovementState(Standing))
	{
		if (!CharacterMovement->IsFalling() && IsCharacterMoving())
		{
			SetMovementState(Sliding);

			FindSlideDirection();

			IsSlidingUp = false;
			IsSlidingOnSlope = false;
			PreviousSlidingLocation = GetActorLocation();

			SlidingTimeline->PlayFromStart();
		}
	}
}

void ACharacterBase::StopSliding()
{
	SlideKeyDown = false;

	if (CheckMovementState(Sliding))
	{
		SetMovementState(Standing);
		if (SlidingTimeline->IsPlaying())
		{
			SlidingTimeline->Stop();
		}
	}
}

void ACharacterBase::FindSlideDirection()
{
	SlideDirection = GetVelocity().GetSafeNormal();

	if (UKismetMathLibrary::DegAcos(FVector::DotProduct(SlideDirection, GetActorForwardVector())) < 90.0f)
	{

	}
}


void ACharacterBase::SlidingOnSlope()
{
	if (IsSlidingOnSlope && !CharacterMovement->IsFalling())
	{
		SlidingTimeline->PlayFromStart();
	}
}

void ACharacterBase::Slide(float Speed)
{
	SlidingSpeed = Speed * GetWorld()->GetDeltaSeconds();

	/* SlidingOnSlope
	if (PreviousSlidingLocation.Z > GetActorLocation().Z)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, TEXT("SlidingDown"));
		IsSlidingOnSlope = true;
	}
	else
	{
		IsSlidingOnSlope = false;
	}*/

	PreviousSlidingLocation = GetActorLocation();

	AddMovementInput(SlideDirection * Speed * SlidingSpeedModifier);
	//SetOnServerSlidingOffset(SlideDirection * Speed * SlidingSpeedModifier);

	//AddActorWorldOffset(SlideDirection * Speed * SlidingSpeedModifier, true);
	//CharacterMovement->AddInputVector(SlideDirection * Speed * SlidingSpeedModifier, true);
	//CharacterMovement->AddInputVector(SlideDirection * Speed * 10);
	//AddActorLocalOffset(SlideDirection * Speed * 10, true);
	//AddMovementInput(SlideDirection * Speed * SlidingSpeedModifier);
}

bool ACharacterBase::IsCharacterMoving()
{
	return((CharacterMovement->Velocity.Size()) > 0 && !CharacterMovement->IsFalling());
}

bool ACharacterBase::CheckMovementState(EMovement Statement)
{
	return MovementState == Statement;
}

void ACharacterBase::SetMovementState(EMovement Statement)
{
	if (MovementState != Statement)
	{
		MovementState = Statement;
		ResolveMovement();
	}
}

void ACharacterBase::ResolveMovement()
{
	switch (MovementState)
	{
	case Standing:
		//UE_LOG(LogTemp, Warning, TEXT("Standing"))
		break;
	case Walking:
		//UE_LOG(LogTemp, Warning, TEXT("Walking"))
		break;
	case Sprinting:
		//UE_LOG(LogTemp, Warning, TEXT("Sprinting"))
		break;
	case Sliding:
		//UE_LOG(LogTemp, Warning, TEXT("Sliding"))
		break;
	default:
		break;
	}
}

void ACharacterBase::SetOnServerMovementSpeed_Implementation(float Value)
{
	CharacterMovement->MaxWalkSpeed = Value;
}

void ACharacterBase::SetOnServerSlidingOffset_Implementation(FVector Value)
{
	//AddActorWorldOffset(Value);
	AddMovementInput(Value);
}