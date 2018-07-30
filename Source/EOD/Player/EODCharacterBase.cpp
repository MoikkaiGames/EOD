// Copyright 2018 Moikkai Games. All Rights Reserved.

#include "EODCharacterBase.h"
#include "CharAnimInstance.h"
#include "Components/StatsComponentBase.h"

#include "UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AEODCharacterBase::AEODCharacterBase(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize Stats Component
	StatsComp = ObjectInitializer.CreateDefaultSubobject<UStatsComponentBase>(this, FName("Character Stats Component"));
	
	// Initialize variables
	CharacterState = ECharacterState::IdleWalkRun;
}

void AEODCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEODCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEODCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEODCharacterBase, CharacterState, COND_SkipOwner);

}

bool AEODCharacterBase::IsAlive() const
{
	return StatsComp->GetCurrentHealth() > 0;
}

bool AEODCharacterBase::IsDead() const
{
	return StatsComp->GetCurrentHealth() <= 0;
}

bool AEODCharacterBase::IsIdle() const
{
	return (CharacterState == ECharacterState::IdleWalkRun && GetVelocity().Size() == 0);
}

bool AEODCharacterBase::IsMoving() const
{
	return (CharacterState == ECharacterState::IdleWalkRun && GetVelocity().Size() != 0);
}

bool AEODCharacterBase::IsJumping() const
{
	return CharacterState == ECharacterState::Jumping;
}

bool AEODCharacterBase::IsBlocking() const
{
	return CharacterState == ECharacterState::Blocking;
}

bool AEODCharacterBase::IsCastingSpell() const
{
	return CharacterState == ECharacterState::CastingSpell;
}

bool AEODCharacterBase::IsNormalAttacking() const
{
	return CharacterState == ECharacterState::Attacking;
}

bool AEODCharacterBase::IsDodging() const
{
	return CharacterState == ECharacterState::Dodging;
}

bool AEODCharacterBase::NeedsHeal() const
{
	return StatsComp->IsLowOnHealth();
}

void AEODCharacterBase::SetCharacterState(ECharacterState NewState)
{
	CharacterState = NewState;

	if (Role < ROLE_Authority)
	{
		Server_SetCharacterState(NewState);
	}
}

void AEODCharacterBase::Server_SetCharacterState_Implementation(ECharacterState NewState)
{
	SetCharacterState(NewState);
}

bool AEODCharacterBase::Server_SetCharacterState_Validate(ECharacterState NewState)
{
	return true;
}

void AEODCharacterBase::SetWalkSpeed(float WalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	if (Role < ROLE_Authority)
	{
		Server_SetWalkSpeed(WalkSpeed);
	}
}

void AEODCharacterBase::SetCharacterRotation(FRotator NewRotation)
{
	GetCharacterMovement()->FlushServerMoves();
	SetActorRotation(NewRotation);

	if (Role < ROLE_Authority)
	{
		Server_SetCharacterRotation(NewRotation);
	}
}

void AEODCharacterBase::SetUseControllerRotationYaw(bool bNewBool)
{
	bUseControllerRotationYaw = bNewBool;

	if (Role < ROLE_Authority)
	{
		Server_SetUseControllerRotationYaw(bNewBool);
	}
}

void AEODCharacterBase::SetNextMontageSection(FName CurrentSection, FName NextSection)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_SetNextSection(CurrentSection, NextSection);
	}

	Server_SetNextMontageSection(CurrentSection, NextSection);
}

void AEODCharacterBase::Server_SetNextMontageSection_Implementation(FName CurrentSection, FName NextSection)
{

	Multicast_SetNextMontageSection(CurrentSection, NextSection);
}

bool AEODCharacterBase::Server_SetNextMontageSection_Validate(FName CurrentSection, FName NextSection)
{
	return true;
}

void AEODCharacterBase::Server_SetUseControllerRotationYaw_Implementation(bool bNewBool)
{
	SetUseControllerRotationYaw(bNewBool);
}

bool AEODCharacterBase::Server_SetUseControllerRotationYaw_Validate(bool bNewBool)
{
	return true;
}

void AEODCharacterBase::Server_SetCharacterRotation_Implementation(FRotator NewRotation)
{
	SetCharacterRotation(NewRotation);
}

bool AEODCharacterBase::Server_SetCharacterRotation_Validate(FRotator NewRotation)
{
	return true;
}

void AEODCharacterBase::Server_SetWalkSpeed_Implementation(float WalkSpeed)
{
	SetWalkSpeed(WalkSpeed);
}

bool AEODCharacterBase::Server_SetWalkSpeed_Validate(float WalkSpeed)
{
	return true;
}

void AEODCharacterBase::Multicast_SetNextMontageSection_Implementation(FName CurrentSection, FName NextSection)
{
	FString Message = FString("Multi cast called");
	UKismetSystemLibrary::PrintString(this, Message, true, false);

	if (!IsLocallyControlled() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_SetNextSection(CurrentSection, NextSection);
	}
}

void AEODCharacterBase::PlayAnimationMontage(UAnimMontage * MontageToPlay, FName SectionToPlay, ECharacterState NewState)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionToPlay);
		CharacterState = NewState;
	}

	Server_PlayAnimationMontage(MontageToPlay, SectionToPlay, NewState);
}

void AEODCharacterBase::Server_PlayAnimationMontage_Implementation(UAnimMontage * MontageToPlay, FName SectionToPlay, ECharacterState NewState)
{
	MultiCast_PlayAnimationMontage(MontageToPlay, SectionToPlay, NewState);
}

bool AEODCharacterBase::Server_PlayAnimationMontage_Validate(UAnimMontage * MontageToPlay, FName SectionToPlay, ECharacterState NewState)
{
	return true;
}

void AEODCharacterBase::MultiCast_PlayAnimationMontage_Implementation(UAnimMontage * MontageToPlay, FName SectionToPlay, ECharacterState NewState)
{
	if (!IsLocallyControlled() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionToPlay);
		CharacterState = NewState;
	}
}

bool AEODCharacterBase::CanMove() const
{
	// Mobs can only move in IdleWalkRun state
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanJump() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanBlock() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanRespawn() const
{
	return false;
}

bool AEODCharacterBase::CanNormalAttack() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanDodge() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

/*
void AEODCharacterBase::AddStatusEffectVisuals(FStatusEffectInfo StatusEffectInfo)
{
	// @todo definition
}

void AEODCharacterBase::RemoveStatusEffectVisuals(FStatusEffectInfo StatusEffectInfo)
{
	// @todo definition
}
*/

FActiveSkill AEODCharacterBase::GetCurrentActiveSkillInfo()
{
	return FActiveSkill();
}

void AEODCharacterBase::HandleMeleeCollision(UAnimSequenceBase * Animation, TArray<FHitResult>& HitResults, bool bHit)
{
	

}

void AEODCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
