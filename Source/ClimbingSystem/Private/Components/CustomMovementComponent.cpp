// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "ClimbingSystem/ClimbingSystemCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "ClimbingSystem/DebugHelper.h"

void UCustomMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

    if (OwningPlayerAnimInstance)
    {
        OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this,&UCustomMovementComponent::OnClimbMontageEnded);
        OwningPlayerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
    }

}

void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //TraceClimbableSurfaces();
    //TraceFromEyeHeight(100.f);
    //SnapMovementToClimableSurfaces(DeltaTime);
}

//public functions to use
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
    const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
    const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
    const FVector End = Start + UpdatedComponent->GetForwardVector();

    ClimbableSurfaces = DoCapsuleTraceMultiByObject(Start, End,true);
    
    return !ClimbableSurfaces.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
    const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
    const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
    
    const FVector Start = ComponentLocation + EyeHeightOffset;
    const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance; 
    
    return DoLineTraceSingleByObject(Start, End);
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

        //??????????
        const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
        const FRotator CleanStandRotation = FRotator(0.0f, DirtyRotation.Yaw, 0.f);
        UpdatedComponent->SetRelativeRotation(CleanStandRotation);
        // 

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

void UCustomMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
    StartClimbing();

    if (MontageToPlay && OwningPlayerAnimInstance && !OwningPlayerAnimInstance->IsAnyMontagePlaying())
    {
        OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
    }
}

void UCustomMovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == IdleToClimbMontage)
    {
        StartClimbing();
    } 
    else
    {
        SetMovementMode(MOVE_Walking);
    }
}

void UCustomMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (IsClimbing())
    {
        HandleClimbPhys(DeltaTime, Iterations);
    }

    Super::PhysCustom(DeltaTime, Iterations);
}

void UCustomMovementComponent::HandleClimbPhys(float DeltaTime, int32 Iterations)
{
    if (DeltaTime < MIN_TICK_TIME)
    {
        return;
    }

    //Process all climbing surfaces
    TraceClimbableSurfaces();
    ProcessClimableSurfaceInfo();

    //Chack if we should stop Climbing
    if (ShouldStopClimbing() || CheckHasReachedFloor())
    {
        StopClimbing();
    }

    RestorePreAdditiveRootMotionVelocity();

    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        //Define max Climb speed and acceleration
        CalcVelocity(DeltaTime, 0.f, true, MaxBreakClimbDeceleration);
    }

    ApplyRootMotionToVelocity(DeltaTime);

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FVector Adjusted = Velocity * DeltaTime;
    FHitResult Hit(1.f);

    //Handle Climb Rotation
    SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(DeltaTime), true, Hit);

    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, DeltaTime, Adjusted);
        SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
    }

    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
    }

    //Snap movement to climable surfaces
    SnapMovementToClimableSurfaces(DeltaTime);

    if (CheckHasReachedLedge())
    {
        PlayClimbMontage(ClimbToTopMontage);
    }
}

float UCustomMovementComponent::GetMaxSpeed() const
{
    if (IsClimbing())
    {
        return MaxClimbSpeed;
    }
    else
    {
        return Super::GetMaxSpeed();
    }
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
    if (IsClimbing())
    {
        return MaxClimbAcceleration;
    }
    else
    {
        return Super::GetMaxAcceleration();
    }
}

void UCustomMovementComponent::ProcessClimableSurfaceInfo()
{
    CurrentClimableSurfaceLocation = FVector::ZeroVector;
    CurrentClimableSurfaceNormal = FVector::ZeroVector;

    if (!ClimbableSurfaces.IsEmpty())
    {
        for (const FHitResult& SurfaceHit : ClimbableSurfaces)
        {
            CurrentClimableSurfaceLocation += SurfaceHit.ImpactPoint;
            CurrentClimableSurfaceNormal += SurfaceHit.ImpactNormal;
        }
        CurrentClimableSurfaceLocation /= ClimbableSurfaces.Num(); // Get avaragge location to snap between different surfaces.
        CurrentClimableSurfaceNormal = CurrentClimableSurfaceNormal.GetSafeNormal();
    }
}

FQuat UCustomMovementComponent::GetClimbRotation(float DeltaTime)
{
    const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();

    if (HasAnimRootMotion() ||CurrentRootMotion.HasOverrideVelocity())
    {
        return CurrentQuat;
    }

    const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimableSurfaceNormal).ToQuat();

    return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime,5.f);

}

void UCustomMovementComponent::SnapMovementToClimableSurfaces(float DeltaTime)
{
    const FVector ConmponentForward = UpdatedComponent->GetForwardVector();
    const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

    const FVector ProjectCharacterToSurface = (CurrentClimableSurfaceLocation - ComponentLocation).ProjectOnTo(ConmponentForward);
    const FVector SnapVector = -CurrentClimableSurfaceNormal * ProjectCharacterToSurface.Length();

    UpdatedComponent->MoveComponent(SnapVector * DeltaTime * MaxClimbSpeed,UpdatedComponent->GetComponentQuat(),true);
}

bool UCustomMovementComponent::ShouldStopClimbing() const
{
    if (ClimbableSurfaces.IsEmpty())
    {
        return true;
    }

    const float DotProduct = FVector::DotProduct(FVector::UpVector, CurrentClimableSurfaceNormal);
    const float DegreeDifference = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    if (DegreeDifference < 30.f)
    {
        return true;
    }

    return false;
}

bool UCustomMovementComponent::CheckHasReachedFloor()
{
    const FVector DownVector = -(UpdatedComponent->GetUpVector());
    const FVector StartOffset = DownVector * 50.0f;

    const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
    const FVector End = Start + DownVector;

    TArray<FHitResult> Floors = DoCapsuleTraceMultiByObject(Start, End, true);

    if (!Floors.IsEmpty())
    {
        for (const FHitResult& PossibleFloor : Floors)
        {
         const bool bClimberReachedFloor = FVector::Parallel(-PossibleFloor.ImpactNormal, FVector::UpVector) && GetUnrotatedClimbVelocity().Z < -10.f;

         if (bClimberReachedFloor)
         {
             return true;
         }
        }

        return false;
    }

    return false;
}

bool UCustomMovementComponent::CheckHasReachedLedge()
{
    FHitResult LedgeHitDetection = TraceFromEyeHeight(100.0f, 50.0f);
    
    if (!LedgeHitDetection.bBlockingHit)
    {
        const FVector WalkableSurfaceTraceStart = LedgeHitDetection.TraceEnd;
        const FVector DownVector = -UpdatedComponent->GetUpVector();
        const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 100.0f;
        
        FHitResult WalkableSurfaceHitResult = DoLineTraceSingleByObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd);
        
        if (WalkableSurfaceHitResult.bBlockingHit && GetUnrotatedClimbVelocity().Z > 10.0f) // character should move up
        {
            return true;
        }
    }

    return false;
}

FVector UCustomMovementComponent::GetUnrotatedClimbVelocity() const
{
    return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(),Velocity);
}

void UCustomMovementComponent::ToggleClimb(bool bEnableClimb)
{
    if (bEnableClimb)
    {
        if (CanStartClimbing())
        {
            PlayClimbMontage(IdleToClimbMontage);
        }
    }
    else
    {
        StopClimbing();
    }
}

FVector UCustomMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const
{
    const bool bIsPlayingAnyMontages = IsFalling() && OwningPlayerAnimInstance && OwningPlayerAnimInstance->IsAnyMontagePlaying();

    if (bIsPlayingAnyMontages)
    {
        return RootMotionVelocity;
    }
    else
    {
        return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
    }
}
