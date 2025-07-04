// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TraceClimbableSurfaces();
}


void UCustomMovementComponent::TraceClimbableSurfaces()
{
    const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
    const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
    const FVector End = Start + UpdatedComponent->GetForwardVector();

    DoCapsuleTraceMultiByObject(Start, End,true);
}

TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebug)
{
    TArray<FHitResult> Result;
    UKismetSystemLibrary::CapsuleTraceMultiForObjects(
        this,       //context
        Start, End, //trace start, trace end
        CapsuleTraceRadius, CapsuleTraceHalfHeight, //trace capsule params 
        SurfaceTraceTypes, false, TArray<AActor*>(),//surface types, trace complex,actors to ignore,
        bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, // if Show debug - DrawDebug else not
        Result,false                                 //result and ignore itself 
    );
    return Result;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebug)
{
    FHitResult Result;

    UKismetSystemLibrary::LineTraceSingleForObjects(
        this,       //context
        Start, End, //trace start, trace end
        SurfaceTraceTypes, false, TArray<AActor*>(),//surface types, trace complex,actors to ignore,
        bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, // if Show debug - DrawDebug else not
        Result, false                                 //result and ignore itself 
    );

    return Result;
}
