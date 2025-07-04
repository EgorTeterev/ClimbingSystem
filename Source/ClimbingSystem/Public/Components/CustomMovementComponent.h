// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"


UCLASS()
class CLIMBINGSYSTEM_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	void TraceClimbableSurfaces();

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceHalfHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> SurfaceTraceTypes;


	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebug = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug = false);
};
