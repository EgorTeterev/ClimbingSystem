// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

class UAnimaMontage;
class UAnimInstance;

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		Move_Climb UMETA(DisplayName = "Climb Mode")
	};
}


UCLASS()
class CLIMBINGSYSTEM_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
//interface for Climbing
	bool TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance,float TraceStartOffset = 0.f);
	void ToggleClimb(bool bEnableClimb);
	bool IsClimbing() const;
	FORCEINLINE FVector GetClimableSurfaceNormal() const { return CurrentClimableSurfaceNormal; }
	FVector GetUnrotatedClimbVelocity() const;
protected:
	//Traces params
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceHalfHeight = 72.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;

	//Trace Querry types of surfaces 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> SurfaceTraceTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimbMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;


	//Traced Surfaces container
	TArray<FHitResult> ClimbableSurfaces;
	FVector CurrentClimableSurfaceLocation;
	FVector CurrentClimableSurfaceNormal;


	//Overriden CharacterMovementComponent interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;

	UFUNCTION()
	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);
private:
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebug = false,bool bDrawPersistentShape = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug = false, bool bDrawPersistentShape = false);
	bool CanStartClimbing();
	void StartClimbing();
	void StopClimbing();
	void HandleClimbPhys(float DeltaTime, int32 Iterations);
	void ProcessClimableSurfaceInfo();
	FQuat GetClimbRotation(float DeltaTime);
	void SnapMovementToClimableSurfaces(float DeltaTime);
	bool ShouldStopClimbing() const;
	bool CheckHasReachedFloor();
	bool CheckHasReachedLedge();
	
};
