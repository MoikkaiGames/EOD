// Copyright 2018 Moikkai Games. All Rights Reserved.

#include "EOD/Characters/Components/EODCharacterMovementComponent.h"
#include "EOD/Core/EOD.h"
#include "EODCharacterBase.h"

#include "UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

UEODCharacterMovementComponent::UEODCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RotationRate = FRotator(300.f, 600.f, 300.f);
	// bCanRotate = true;
}

void UEODCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEODCharacterMovementComponent, DesiredCustomRotation, COND_SkipOwner);
}

void UEODCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	const float AngleTolerance = 1e-3f;
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
	CurrentRotation.DiagnosticCheckNaN(TEXT("EODCharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

	if (!(bOrientRotationToMovement || bUseControllerDesiredRotation || !CurrentRotation.Equals(DesiredCustomRotation, AngleTolerance)) ||
		!HasValidData() ||
		(!CharacterOwner->Controller && !bRunPhysicsWithNoController))
	{
		return;
	}

	FRotator DeltaRot = GetDeltaRotation(DeltaTime);
	DeltaRot.DiagnosticCheckNaN(TEXT("EODCharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

	FRotator DesiredRotation = CurrentRotation;
	if (bOrientRotationToMovement)
	{
		DesiredRotation = ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRot);
	}
	else if (CharacterOwner->Controller && bUseControllerDesiredRotation)
	{
		DesiredRotation = CharacterOwner->Controller->GetDesiredRotation();
	}
	else
	{
		DesiredRotation = DesiredCustomRotation;
	}

	if (ShouldRemainVertical())
	{
		DesiredRotation.Pitch = 0.f;
		DesiredRotation.Yaw = FRotator::NormalizeAxis(DesiredRotation.Yaw);
		DesiredRotation.Roll = 0.f;
	}
	else
	{
		DesiredRotation.Normalize();
	}

	if (!CurrentRotation.Equals(DesiredRotation, AngleTolerance))
	{
		// PITCH
		if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
		{
			DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
		}

		// YAW
		if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
		{
			DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
		}

		// ROLL
		if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
		{
			DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
		}

		// Set the new rotation.
		DesiredRotation.DiagnosticCheckNaN(TEXT("EODCharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
		MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
	}
}

void UEODCharacterMovementComponent::ServerMoveDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent * ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	ServerMove_Implementation(TimeStamp0, InAccel0, FVector(1.f, 2.f, 3.f), PendingFlags, ClientRoll, View0, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
	// Prevents anim notifies from being dropped from server
	if (CharacterOwner)
	{
		CharacterOwner->GetMesh()->ConditionallyDispatchQueuedAnimEvents();
	}
	ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

float UEODCharacterMovementComponent::GetMaxSpeed() const
{
	AEODCharacterBase* EODOwner = Cast<AEODCharacterBase>(PawnOwner);
	//~ Apply movement speed modifier
	float MaxSpeed = Super::GetMaxSpeed();
	MaxSpeed = EODOwner ? (MaxSpeed * EODOwner->MovementSpeedModifier) : MaxSpeed;
	return MaxSpeed;
}

void UEODCharacterMovementComponent::Server_SetDesiredCustomRotation_Implementation(const FRotator& NewRotation)
{
	SetDesiredCustomRotation(NewRotation);
}

bool UEODCharacterMovementComponent::Server_SetDesiredCustomRotation_Validate(const FRotator& NewRotation)
{
	return true;
}
