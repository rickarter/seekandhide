// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "CharacterBase.generated.h"

UCLASS()
class SEEKANDHIDE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
		class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Movement")
		class UCurveFloat* SlidingCurve;

	UFUNCTION()
		void Slide(float Speed);

	UFUNCTION(Reliable, Server)//, WithValidation)
		void SetOnServerMovementSpeed(float Value);
	void SetOnServerMovementSpeed_Implementation(float Value);
	//bool SetOnServerMovementSpeed_Validate(float Value);

	UFUNCTION(Reliable, Server)//, WithValidation)
		void SetOnServerSlidingOffset(FVector Value);
	void SetOnServerSlidingOffset_Implementation(FVector);
	//bool SetOnServerSlidingOffset_Validate(FVector Value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** References to Object */
	UCapsuleComponent* CapsuleComponent = GetCapsuleComponent();
	UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();

	/** Movement varibles and functions*/
	enum EMovement
	{
		Standing,
		Walking,
		Sprinting,
		Sliding,
	};

	EMovement MovementState;

	bool SprintKeyDown;
	bool SlideKeyDown;

	UTimelineComponent* SlidingTimeline;
	float SlidingSpeed;
	float SlidingSpeedModifier = 5;
	FVector PreviousSlidingLocation;
	FVector SlideDirection;
	bool IsSlidingOnSlope = false;
	bool IsSlidingUp = false;

	float WalkSpeed = 600.0f;
	float SprintSpeedMultiplier = 2.0f;
	float SprintSpeed;

	bool CheckMovementState(EMovement Statement);
	void SetMovementState(EMovement Statement);
	void ResolveMovement();
	bool IsCharacterMoving();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Jump();

	void StartSprinting();
	void StopSprinting();

	void StartSliding();
	void StopSliding();
	void FindSlideDirection();
	void SlideWhenLand();
	void SlidingOnSlope();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
