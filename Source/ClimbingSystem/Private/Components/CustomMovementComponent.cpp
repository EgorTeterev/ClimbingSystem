// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "ClimbingSystem/ClimbingSystemCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "ClimbingSystem/DebugHelper.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    //TraceClimbableSurfaces();
    //TraceFromEyeHeight(100.f);
}

//public functions to use
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
    const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
    const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
    const FVector End = Start + UpdatedComponent->GetForwardVector();

    ClimbableSurfaces = DoCapsuleTraceMultiByObject(Start, End,true,true);
    
    return !ClimbableSurfaces.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
    const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
    const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
    
    const FVector Start = ComponentLocation + EyeHeightOffset;
    const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance; 
    
    return DoLineTraceSingleByObject(Start, End, true,true);
}

//private trace functions
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebug, bool bDrawPersistentShape)
{
    TArray<FHitResult> Result;

    EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;


    if (bShowDebug)
    {
        DebugTraceType = EDrawDebugTrace::ForOneFrame;

        if (bDrawPersistentShape)
        {
            DebugTraceType = EDrawDebugTrace::Persistent;
        }
    }


    UKismetSystemLibrary::CapsuleTraceMultiForObjects(
        this,       //context
        Start, End, //trace start, trace end
        CapsuleTraceRadius, CapsuleTraceHalfHeight, //trace capsule params 
        SurfaceTraceTypes, false, TArray<AActor*>(),//surface types, trace complex,actors to ignore,
        DebugTraceType, // if Show debug - DrawDebug else not
        Result,false                                 //result and ignore itself 
    );

    return Result;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug, bool bDrawPersistentShape)
{
    FHitResult Result;

    EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;


    if (bShowDebug)
    {
        DebugTraceType = EDrawDebugTrace::ForOneFrame;

        if (bDrawPersistentShape)
        {
            DebugTraceType = EDrawDebugTrace::Persistent;
        }
    }

    UKismetSystemLibrary::LineTraceSingleForObjects(
        this,       //context
        Start, End, //trace start, trace end
        SurfaceTraceTypes, false, TArray<AActor*>(),//surface types, trace complex,actors to ignore,
        DebugTraceType, // if Show debug - DrawDebug else not
        Result, false                                 //result and ignore itself 
    );

    return Result;
}

bool UCustomMovementComponent::CanStartClimbing()
{
    // Character cant climb if he is falling, or find out, that there is no surfaces to climb
    if (IsFalling() || !TraceClimbableSurfaces())
    {
        return false;
    }
    //Cant climg, head see clif
    if (!TraceFromEyeHeight(100.0f).bBlockingHit)
    {
        return false;
    }

    return true;
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    if (IsClimbing())
    {
        bOrientRotationToMovement = false;
        CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);// character collision takes smaller space while Climbing
    }

    if (PreviousMovementMode == MOVE_Custom && PreviousMovementMode == EMovementMode::MOVE_Custom)
    {
        bOrientRotationToMovement = true;
        CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);//character collision takse normal size after Climbing end

        StopMovementImmediately();
    }

    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

}

void UCustomMovementComponent::StartClimbing()
{
    SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::Move_Climb);
}

void UCustomMovementComponent::StopClimbing()
{
    SetMovementMode(EMovementMode::MOVE_Falling);

}

bool UCustomMovementComponent::IsClimbing() const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::Move_Climb;
}


void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    if (IsClimbing())
    {
        HandleClimbPhys(deltaTime, Iterations);
    }

    Super::PhysCustom(deltaTime, Iterations);
}

void UCustomMovementComponent::HandleClimbPhys(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    //Process all climbing surfaces

    //Chack if we should stop Climbing

    RestorePreAdditiveRootMotionVelocity();

    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        //Define max Climb speed and acceleration
        CalcVelocity(deltaTime, 0.f, true, GetMaxBrakingDeceleration());
    }

    ApplyRootMotionToVelocity(deltaTime);

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FVector Adjusted = Velocity * deltaTime;
    FHitResult Hit(1.f);

    //Handle Climb Rotation
    SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, deltaTime, Adjusted);
        SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
    }

    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }

    //Snap movement to climable surfaces

}

void UCustomMovementComponent::ToggleClimb(bool bEnableClimb)
{
    if (bEnableClimb)
    {
        if (CanStartClimbing())
        {
            StartClimbing();
        }
    }
    else
    {
        StopClimbing();
    }
}


