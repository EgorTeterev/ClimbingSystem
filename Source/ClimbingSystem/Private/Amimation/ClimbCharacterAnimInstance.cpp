// Fill out your copyright notice in the Description page of Project Settings.


#include "Amimation/ClimbCharacterAnimInstance.h"
#include "ClimbingSystem/ClimbingSystemCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void UClimbCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ClimbingCharacter = Cast<AClimbingSystemCharacter>(TryGetPawnOwner());

	if (ClimbingCharacter)
	{
		CustomMovementComponent = ClimbingCharacter->GetCustomMovementComponent();
	}
}


void UClimbCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ClimbingCharacter || !CustomMovementComponent)
	{
		return;
	}

	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
}

void UClimbCharacterAnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(ClimbingCharacter->GetVelocity());
}

void UClimbCharacterAnimInstance::GetAirSpeed()
{
	AirSpeed = ClimbingCharacter->GetVelocity().Z;
}

void UClimbCharacterAnimInstance::GetShouldMove()
{
	bShouldMove = (CustomMovementComponent->GetCurrentAcceleration().Size() > 0 && GroundSpeed > 5.f && !bIsFalling) ? true : false ;
}

void UClimbCharacterAnimInstance::GetIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling();
}
