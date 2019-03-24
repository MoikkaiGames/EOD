// Copyright 2018 Moikkai Games. All Rights Reserved.

#include "EOD/Characters/EODCharacterBase.h"
#include "EOD/Characters/RideBase.h"
#include "EOD/AnimInstances/CharAnimInstance.h"
#include "EOD/Interactives/InteractionInterface.h"
#include "EOD/Statics/EODBlueprintFunctionLibrary.h"
#include "EOD/Player/EODPlayerController.h"
#include "EOD/AI/EODAIControllerBase.h"
#include "EOD/UI/HUDWidget.h"
#include "EOD/Characters/Components/SkillsComponent.h"
#include "EOD/Characters/Components/StatsComponentBase.h"
#include "EOD/Characters/Components/GameplaySkillsComponent.h"
#include "EOD/Characters/Components/EODCharacterMovementComponent.h"

#include "DynamicHUDWidget.h"

#include "States/IdleWalkRunState.h"
#include "States/DeadState.h"
#include "States/DodgeState.h"
#include "States/GuardState.h"
#include "States/HitInCombatState.h"
#include "States/NormalAttackState.h"
#include "States/UsingSkillState.h"

#include "UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

/**
 * EOD Character stats
 */
DECLARE_CYCLE_STAT(TEXT("EOD ChararaterTick"), STAT_EODCharacterTick, STATGROUP_EOD);

const FName AEODCharacterBase::CameraComponentName(TEXT("Camera"));
const FName AEODCharacterBase::SpringArmComponentName(TEXT("Camera Boom"));
const FName AEODCharacterBase::CharacterStatsComponentName(TEXT("Character Stats"));
const FName AEODCharacterBase::GameplaySkillsComponentName(TEXT("Skill Manager"));
const FName AEODCharacterBase::InteractionSphereComponentName(TEXT("Interaction Sphere"));

AEODCharacterBase::AEODCharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UEODCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	CharacterStatsComponent = ObjectInitializer.CreateDefaultSubobject<UStatsComponentBase>(this, AEODCharacterBase::CharacterStatsComponentName);
	SkillManager = ObjectInitializer.CreateDefaultSubobject<UGameplaySkillsComponent>(this, AEODCharacterBase::GameplaySkillsComponentName);

	CameraBoomComponent = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, AEODCharacterBase::SpringArmComponentName);
	if (CameraBoomComponent)
	{
		CameraBoomComponent->bUsePawnControlRotation = true;
		CameraBoomComponent->SetupAttachment(RootComponent);
		CameraBoomComponent->AddLocalOffset(FVector(0.f, 0.f, 60.f));
	}

	CameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, AEODCharacterBase::CameraComponentName);
	if (CameraComponent)
	{
		CameraComponent->SetupAttachment(CameraBoomComponent, USpringArmComponent::SocketName);
	}

	InteractionSphereComponent = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, AEODCharacterBase::InteractionSphereComponentName);
	if (InteractionSphereComponent)
	{
		InteractionSphereComponent->SetupAttachment(RootComponent);
		InteractionSphereComponent->SetSphereRadius(150.f);
		// No need to enable interaction sphere unless the character is possessed by player controller
		InteractionSphereComponent->Deactivate();
		InteractionSphereComponent->SetCollisionProfileName(CollisionProfileNames::NoCollision);
	}

	SetReplicates(true);
	SetReplicateMovement(true);
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetIsReplicated(true);
	}

	CameraZoomRate = 15;
	CameraArmMinimumLength = 50;
	CameraArmMaximumLength = 500;

	DefaultWalkSpeed = 400.f;
	DefaultRunSpeed = 600.f;
	DefaultWalkSpeedWhileBlocking = 150.f;

	CharacterState = ECharacterState::IdleWalkRun;
	bGodMode = false;
	TargetSwitchDuration = 0.1f;

	// By default the weapon should be sheathed
	bWeaponSheathed = true;

	// MaxNumberOfSkills = 30;
	DodgeStaminaCost = 30;
	DodgeImmunityTriggerDelay = 0.1f;
	DodgeImmunityDuration = 0.4;
	DamageBlockTriggerDelay = 0.2f;

	Faction = EFaction::Player;

	MovementSpeedModifier = 1.f;

}

void AEODCharacterBase::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_EODCharacterTick);

	Super::Tick(DeltaTime);

	ResetTickDependentData();

	if (Controller && Controller->IsLocalController())
	{
		bool bCanGuardAgainstAttacks = CanGuardAgainstAttacks();
		// If character wants to guard but it's guard is not yet active
		if (bWantsToGuard && !IsGuardActive() && bCanGuardAgainstAttacks)
		{
			StartBlockingAttacks();
		}
		// If the character guard is active but it doesn't want to guard anymore
		else if (!bWantsToGuard && IsGuardActive())
		{
			StopBlockingAttacks();
		}
	}

	UpdateCharacterState(DeltaTime);




	/*
	if (Controller && Controller->IsLocalPlayerController())
	{
		// Update guard state only if either the character wants to guard or if character guard is active
		if (bWantsToGuard || IsGuardActive())
		{
			UpdateGuardState(DeltaTime);
		}

		// Update normal attack state only if either the character wants to normal attack or if character is actively normal attacking
		if (bNormalAttackKeyPressed || IsNormalAttacking())
		{
			UpdateNormalAttackState(DeltaTime);
		}

		if (IsDodging())
		{

		}

		// Update fall state only if either the character is failling or jumping
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		if ((MoveComp && MoveComp->IsFalling()) || IsJumping())
		{
			UpdateFallState(DeltaTime);
		}

		UpdateMovement(DeltaTime);
		UpdateRotation(DeltaTime);
	}
	*/
}

void AEODCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEODCharacterBase, CurrentRide);
	DOREPLIFETIME(AEODCharacterBase, MovementSpeedModifier);
	DOREPLIFETIME(AEODCharacterBase, Server_CharacterStateInfo);

	DOREPLIFETIME_CONDITION(AEODCharacterBase, bIsRunning, COND_SkipOwner);
	// DOREPLIFETIME_CONDITION(AEODCharacterBase, bGuardActive, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AEODCharacterBase, bWeaponSheathed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AEODCharacterBase, bPCTryingToMove, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AEODCharacterBase, BlockMovementDirectionYaw, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AEODCharacterBase, CharacterMovementDirection, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AEODCharacterBase, bCharacterStateAllowsMovement, COND_SkipOwner);
}

void AEODCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Intentional additional call to BindUIDelegates (another in Restart())
	BindUIDelegates();

	UEODCharacterMovementComponent* MoveComp = Cast<UEODCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->SetDesiredCustomRotation(GetActorRotation());
	}
}

void AEODCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void AEODCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// @todo - Enable interaction sphere on client.
	if (NewController && NewController->IsLocalPlayerController())
	{
		EnableInteractionSphere();
	}
	else
	{
		DisableInteractionSphere();
	}
}

void AEODCharacterBase::UnPossessed()
{
	Super::UnPossessed();
}

void AEODCharacterBase::Restart()
{
	Super::Restart();

	// Intentional additional call to BindUIDelegates (another in BeginPlay())
	BindUIDelegates();
}

void AEODCharacterBase::UpdateCharacterState(float DeltaTime)
{
	// Update client state
	switch (Client_CharacterStateInfo.CharacterState)
	{
	case ECharacterState::IdleWalkRun:
		UpdateIdleWalkRunState(DeltaTime);
		break;
	case ECharacterState::SwitchingWeapon:
		break;
	case ECharacterState::Jumping:
		UpdateFallState(DeltaTime);
		break;
	case ECharacterState::Dodging:
		break;
	case ECharacterState::Blocking:
		UpdateBlockState(DeltaTime);
		break;
	case ECharacterState::Attacking:
		break;
	case ECharacterState::Looting:
		break;
	case ECharacterState::SpecialAction:
		break;
	case ECharacterState::Interacting:
		break;
	case ECharacterState::UsingActiveSkill:
		break;
	case ECharacterState::CastingSpell:
		break;
	case ECharacterState::SpecialMovement:
		break;
	case ECharacterState::GotHit:
		break;
	case ECharacterState::Dead:
		break;
	default:
		break;
	}
}

float AEODCharacterBase::BP_GetRotationYawFromAxisInput()
{
	return GetRotationYawFromAxisInput();
}

float AEODCharacterBase::BP_GetControllerRotationYaw() const
{
	return (Controller ? FMath::UnwindDegrees(Controller->GetControlRotation().Yaw) : 0.0f);
}

void AEODCharacterBase::TriggeriFrames(float Duration, float Delay)
{
	if (Role < ROLE_Authority)
	{
		Server_TriggeriFrames(Duration, Delay);
	}
	else
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &AEODCharacterBase::EnableiFrames, Duration);
			World->GetTimerManager().SetTimer(DodgeImmunityTimerHandle, TimerDelegate, Delay, false);
		}
	}
}

bool AEODCharacterBase::BP_IsDead() const
{
	return IsDead();
}

bool AEODCharacterBase::BP_HasBeenHit() const
{
	return HasBeenHit();
}

bool AEODCharacterBase::CanMove() const
{
	// Mobs can only move in IdleWalkRun state
	return CharacterState == ECharacterState::IdleWalkRun || (IsUsingAnySkill() && bSkillAllowsMovement);
}

bool AEODCharacterBase::CanJump() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanDodge() const
{
	return CharacterState == ECharacterState::IdleWalkRun;
}

bool AEODCharacterBase::CanGuardAgainstAttacks() const
{
	return (IsIdleOrMoving() || IsNormalAttacking()) && !(IsWeaponSheathed());
}

bool AEODCharacterBase::CanBlock() const
{
	return IsIdleOrMoving();
}

bool AEODCharacterBase::CanRespawn() const
{
	return false;
}

bool AEODCharacterBase::CanNormalAttack() const
{
	return IsIdleOrMoving();
}

bool AEODCharacterBase::CanUseAnySkill() const
{
	return IsIdleOrMoving();
}

bool AEODCharacterBase::CanUseSkill(FSkillTableRow * Skill)
{
	return false;
}

bool AEODCharacterBase::BP_NeedsHealing() const
{
	return NeedsHealing();
}

bool AEODCharacterBase::IsHealing() const
{
	return false;
}

bool AEODCharacterBase::CCEInterrupt_Implementation(const float BCAngle)
{
	return false;
}

bool AEODCharacterBase::CCEStun_Implementation(const float Duration)
{
	return false;
}

void AEODCharacterBase::CCERemoveStun_Implementation()
{
}

bool AEODCharacterBase::CCEFreeze_Implementation(const float Duration)
{
	return false;
}

void AEODCharacterBase::CCEUnfreeze_Implementation()
{
}

bool AEODCharacterBase::CCEKnockdown_Implementation(const float Duration)
{
	return false;
}

void AEODCharacterBase::CCEEndKnockdown_Implementation()
{
}

bool AEODCharacterBase::CCEFlinch_Implementation(const float BCAngle)
{
	return false;
}

bool AEODCharacterBase::CCEKnockback_Implementation(const float Duration, const FVector & ImpulseDirection)
{
	return false;
}

void AEODCharacterBase::InitiateDeathSequence_Implementation()
{
}

bool AEODCharacterBase::Stun(const float Duration)
{
	return false;
}

void AEODCharacterBase::EndStun()
{
}

bool AEODCharacterBase::Freeze(const float Duration)
{
	return false;
}

void AEODCharacterBase::EndFreeze()
{
}

bool AEODCharacterBase::Knockdown(const float Duration)
{
	return false;
}

void AEODCharacterBase::EndKnockdown()
{
}

bool AEODCharacterBase::Knockback(const float Duration, const FVector & ImpulseDirection)
{
	return false;
}

void AEODCharacterBase::BlockAttack()
{
}

void AEODCharacterBase::EnableiFrames(float Duration)
{
	UWorld* World = GetWorld();
	if (Duration > 0.f && World)
	{
		bActiveiFrames = true;
		World->GetTimerManager().SetTimer(DodgeImmunityTimerHandle, this, &AEODCharacterBase::DisableiFrames, Duration, false);
	}
}

void AEODCharacterBase::DisableiFrames()
{
	bActiveiFrames = false;
}

void AEODCharacterBase::BindUIDelegates()
{
	if (GetController() && GetController()->IsLocalPlayerController())
	{
		AEODPlayerController* PC = Cast<AEODPlayerController>(Controller);
		if (IsValid(PC))
		{
			PC->InitializeHUDWidget();
			// PC->CreateHUDWidget();
		}
		else
		{
			return;
		}

		if (IsValid(PC->GetHUDWidget()))
		{
			UStatusIndicatorWidget* StatusIndicatorWidget = PC->GetHUDWidget()->GetStatusIndicatorWidget();
			if (IsValid(CharacterStatsComponent) && IsValid(StatusIndicatorWidget))
			{
				CharacterStatsComponent->OnHealthChanged.AddUniqueDynamic(StatusIndicatorWidget, &UStatusIndicatorWidget::UpdateHealthBar);
				CharacterStatsComponent->OnManaChanged.AddUniqueDynamic(StatusIndicatorWidget, &UStatusIndicatorWidget::UpdateManaBar);
				CharacterStatsComponent->OnStaminaChanged.AddUniqueDynamic(StatusIndicatorWidget, &UStatusIndicatorWidget::UpdateStaminaBar);
			}

			UDynamicSkillBarWidget* SkillBarWidget = PC->GetHUDWidget()->GetSkillBarWidget();
			// if (IsValid(SkillManager) && IsValid(SkillBarWidget))
			{
				// The saved skill bar layout may not have already been loaded
				// SkillManager->LoadSkillBarLayout();
				// SkillBarWidget->UpdateSkillBarLayout(SkillManager->GetSkillBarLayout());
				// SkillBarWidget->OnNewSkillAdded.AddUniqueDynamic(SkillManager, &UGameplaySkillsComponent::AddNewSkill);
			}
		}
	}
}

void AEODCharacterBase::UnbindUIDelegates()
{
}

void AEODCharacterBase::ResetTickDependentData()
{
	bDesiredRotationYawFromAxisInputUpdated = false;
}

void AEODCharacterBase::EnableDamageBlocking()
{
	bBlockingDamage = true;
}

void AEODCharacterBase::DisableDamageBlocking()
{
	bBlockingDamage = false;
	// Clear block damage timer just in case it is still active
	GetWorld()->GetTimerManager().ClearTimer(BlockTimerHandle); 
}

bool AEODCharacterBase::BP_IsInCombat() const
{
	return IsInCombat();
}

void AEODCharacterBase::BP_SetCharacterState(const ECharacterState NewState)
{
	SetCharacterState(NewState);
}

ECharacterState AEODCharacterBase::BP_GetCharacterState() const
{
	return GetCharacterState();
}

void AEODCharacterBase::BP_SetWalkSpeed(const float WalkSpeed)
{
	SetWalkSpeed(WalkSpeed);
}

void AEODCharacterBase::BP_SetCharacterRotation(const FRotator NewRotation)
{
	SetCharacterRotation(NewRotation);
}

void AEODCharacterBase::BP_SetUseControllerRotationYaw(const bool bNewBool)
{
	SetUseControllerRotationYaw(bNewBool);
}

bool AEODCharacterBase::UseSkill_Implementation(FName SkillID)
{
	return false;
}

EEODTaskStatus AEODCharacterBase::CheckSkillStatus(FName SkillID)
{
	return EEODTaskStatus();
}

FName AEODCharacterBase::GetMostWeightedMeleeSkillID(AEODCharacterBase const * const TargetCharacter) const
{
	return FName();
}

FName AEODCharacterBase::BP_GetCurrentActiveSkillID() const
{
	return GetCurrentActiveSkillID();
}

void AEODCharacterBase::BP_SetCurrentActiveSkillID(FName SkillID)
{
	SetCurrentActiveSkillID(SkillID);
}

FLastUsedSkillInfo& AEODCharacterBase::BP_GetLastUsedSkill()
{
	return GetLastUsedSkill();
}

void AEODCharacterBase::ApplyStatusEffect(const UStatusEffectBase * StatusEffect)
{
	// @todo definition
}

void AEODCharacterBase::RemoveStatusEffect(const UStatusEffectBase * StatusEffect)
{
	// @todo definition
}

void AEODCharacterBase::OnMontageBlendingOut(UAnimMontage * AnimMontage, bool bInterrupted)
{
}

void AEODCharacterBase::OnMontageEnded(UAnimMontage * AnimMontage, bool bInterrupted)
{
}

void AEODCharacterBase::BP_PlayAnimationMontage(UAnimMontage * MontageToPlay, FName SectionToPlay, ECharacterState NewState)
{
	PlayAnimationMontage(MontageToPlay, SectionToPlay, NewState);
}

void AEODCharacterBase::SetNextMontageSection(FName CurrentSection, FName NextSection)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_SetNextSection(CurrentSection, NextSection);
	}

	Server_SetNextMontageSection(CurrentSection, NextSection);
}

bool AEODCharacterBase::DeltaRotateCharacterToDesiredYaw(float DesiredYaw, float DeltaTime, float Precision, float RotationRate)
{
	PrintToScreen(this, FString("Rotating"));
	float CurrentYaw = GetActorRotation().Yaw;
	if (FMath::IsNearlyEqual(CurrentYaw, DesiredYaw, Precision))
	{
		return true;
	}

	float DeltaRotationYaw = (RotationRate >= 0.f) ? (RotationRate * DeltaTime) : 360.f;
	float NewRotationYaw = FMath::FixedTurn(CurrentYaw, DesiredYaw, DeltaRotationYaw);
	SetCharacterRotation(FRotator(0.f, NewRotationYaw, 0.f));

	if (FMath::IsNearlyEqual(CurrentYaw, NewRotationYaw, Precision))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AEODCharacterBase::Die(ECauseOfDeath CauseOfDeath, AEODCharacterBase* InstigatingChar)
{
	if (bGodMode || IsDead())
	{
		// cannot die
		return;
	}

	if (CauseOfDeath == ECauseOfDeath::ZeroHP)
	{

	}
	else
	{
		// Set current hp to 0
		GetCharacterStatsComponent()->ModifyBaseHealth(-GetCharacterStatsComponent()->GetMaxHealth());
		SetCharacterState(ECharacterState::Dead);

		// @todo play death animation and death sound
	}
}

float AEODCharacterBase::GetOrientationYawToActor(AActor* TargetActor)
{
	FVector OrientationVector = TargetActor->GetActorLocation() - GetActorLocation();
	FRotator OrientationRotator = OrientationVector.ToOrientationRotator();
	return OrientationRotator.Yaw;
}

void AEODCharacterBase::TurnOnTargetSwitch()
{
	GetMesh()->SetScalarParameterValueOnMaterials(FName("Target_Switch_On"), 1.f);
	GetWorld()->GetTimerManager().SetTimer(TargetSwitchTimerHandle, this, &AEODCharacterBase::TurnOffTargetSwitch, TargetSwitchDuration, false);
}

void AEODCharacterBase::TurnOffTargetSwitch()
{
	GetMesh()->SetScalarParameterValueOnMaterials(FName("Target_Switch_On"), 0.f);
}

void AEODCharacterBase::OnRep_WeaponSheathed()
{
	PlayToggleSheatheAnimation();
}

void AEODCharacterBase::OnRep_GuardActive()
{
	if (bGuardActive)
	{
		EnableCharacterGuard();
	}
	else
	{
		DisableCharacterGuard();
	}
}

void AEODCharacterBase::OnRep_CharacterStateInfo(FCharacterStateInfo LastStateInfo)
{
	// If the replicated character state info is already same as local state info then the client should be the owner
	if (Server_CharacterStateInfo == Client_CharacterStateInfo)
	{
		// check(Controller && Controller->IsLocalController());
		return;
	}

	if (LastStateInfo.CharacterState == ECharacterState::Blocking)
	{
		StopBlockingAttacks();
	}

	if (Server_CharacterStateInfo.CharacterState == ECharacterState::IdleWalkRun)
	{
		ResetState();
	}
	else if (Server_CharacterStateInfo.CharacterState == ECharacterState::Dodging)
	{
		StartDodge();
	}
	else if (Server_CharacterStateInfo.CharacterState == ECharacterState::Blocking)
	{
		StartBlockingAttacks();
	}
}

//~ @todo
void AEODCharacterBase::OnRep_ServerCharacterState(ECharacterState LastState)
{
}

void AEODCharacterBase::Server_SetCharacterStateInfo_Implementation(FCharacterStateInfo NewStateInfo)
{
	FCharacterStateInfo OldStateInfo = Server_CharacterStateInfo;
	Server_CharacterStateInfo = NewStateInfo;
	if (Controller && Controller->IsLocalPlayerController())
	{
		return;
	}
	else
	{
		OnRep_CharacterStateInfo(OldStateInfo);
	}
}

bool AEODCharacterBase::Server_SetCharacterStateInfo_Validate(FCharacterStateInfo NewStateInfo)
{
	return true;
}

void AEODCharacterBase::Server_SpawnAndMountRideableCharacter_Implementation(TSubclassOf<ARideBase> RideCharacterClass)
{
	SpawnAndMountRideableCharacter(RideCharacterClass);
}

bool AEODCharacterBase::Server_SpawnAndMountRideableCharacter_Validate(TSubclassOf<ARideBase> RideCharacterClass)
{
	return true;
}

void AEODCharacterBase::Client_DisplayTextOnPlayerScreen_Implementation(const FString& Message, const FLinearColor& TextColor, const FVector& TextPosition)
{
	if (IsPlayerControlled())
	{
		CreateAndDisplayTextOnPlayerScreen(Message, TextColor, TextPosition);
	}
}

void AEODCharacterBase::Server_StopBlockingDamage_Implementation()
{
	StopBlockingDamage();
}

bool AEODCharacterBase::Server_StopBlockingDamage_Validate()
{
	return true;
}

void AEODCharacterBase::Server_StartBlockingDamage_Implementation(float Delay)
{
	StartBlockingDamage(Delay);
}

bool AEODCharacterBase::Server_StartBlockingDamage_Validate(float Delay)
{
	return true;
}

void AEODCharacterBase::Server_TriggeriFrames_Implementation(float Duration, float Delay)
{
	TriggeriFrames(Duration, Delay);
}

bool AEODCharacterBase::Server_TriggeriFrames_Validate(float Duration, float Delay)
{
	return true;
}

void AEODCharacterBase::Server_SetCharacterState_Implementation(ECharacterState NewState)
{
	SetCharacterState(NewState);
}

bool AEODCharacterBase::Server_SetCharacterState_Validate(ECharacterState NewState)
{
	return true;
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
	Multicast_SetCharacterRotation(NewRotation);
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

void AEODCharacterBase::StartDodge()
{
}

void AEODCharacterBase::CancelDodge()
{
}

void AEODCharacterBase::FinishDodge()
{
}

void AEODCharacterBase::ResetState()
{
	if (GetNetMode() != ENetMode::NM_Client)
	{
		Server_CharacterStateInfo = FCharacterStateInfo();
	}

	Client_CharacterStateInfo = FCharacterStateInfo();
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bUseControllerDesiredRotation = false;
	bCharacterStateAllowsMovement = false;
	bCharacterStateAllowsRotation = false;
}

void AEODCharacterBase::StartBlockingAttacks()
{
	bGuardActive = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = DefaultWalkSpeedWhileBlocking * MovementSpeedModifier;
	MoveComp->bUseControllerDesiredRotation = true;
	bCharacterStateAllowsMovement = true;
	bCharacterStateAllowsRotation = false; // no custom rotation allowed since we will be using controller rotation yaw

	if (GetNetMode() != ENetMode::NM_Client)
	{
		StartBlockingDamage(DamageBlockTriggerDelay);
	}

	bool bIsLocalPlayerController = Controller && Controller->IsLocalPlayerController();
	if (bIsLocalPlayerController)
	{
		FCharacterStateInfo StateInfo;
		StateInfo.CharacterState = ECharacterState::Blocking;
		Client_CharacterStateInfo = StateInfo;
		Server_SetCharacterStateInfo(StateInfo);
	}
	else
	{
		Client_CharacterStateInfo = Server_CharacterStateInfo;
	}
}

void AEODCharacterBase::StopBlockingAttacks()
{
	bGuardActive = false;

	if (GetNetMode() != ENetMode::NM_Client)
	{
		StopBlockingDamage();
	}

	if (Controller && Controller->IsLocalController())
	{
		ResetState();
		Server_SetCharacterStateInfo(FCharacterStateInfo());
	}
}

void AEODCharacterBase::UpdateBlockState(float DeltaTime)
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		if (ForwardAxisValue == 0)
		{
			if (RightAxisValue > 0)
			{
				SetBlockMovementDirectionYaw(90.f);
			}
			else if (RightAxisValue < 0)
			{
				SetBlockMovementDirectionYaw(-90.f);
			}
			else
			{
				SetBlockMovementDirectionYaw(0.f);
			}
		}
		else
		{
			float NewYaw = FMath::RadiansToDegrees(FMath::Atan2(RightAxisValue, ForwardAxisValue));
			SetBlockMovementDirectionYaw(NewYaw);
		}
	}
}

void AEODCharacterBase::StartJumping()
{
}

void AEODCharacterBase::StopJumping()
{
}

void AEODCharacterBase::SetCharacterStateInfo(FCharacterStateInfo NewStateInfo)
{
	Client_CharacterStateInfo = NewStateInfo;
	if (Role < ROLE_Authority)
	{
		Server_SetCharacterStateInfo(NewStateInfo);
	}
}

void AEODCharacterBase::EnableCharacterGuard()
{
	// @todo wait for normal attack section to finish before blocking?
	if (IsNormalAttacking())
	{
		StopNormalAttack();
	}
	SetCharacterState(ECharacterState::Blocking);
	// GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void AEODCharacterBase::DisableCharacterGuard()
{
	SetCharacterState(ECharacterState::IdleWalkRun);
	// GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void AEODCharacterBase::MoveForward(const float Value)
{
	ForwardAxisValue = Value;

	if (Value != 0 && CanMove())
	{
		FRotator Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
		FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AEODCharacterBase::MoveRight(const float Value)
{
	RightAxisValue = Value;

	if (Value != 0 && CanMove())
	{
		FVector Direction = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AEODCharacterBase::UpdateRotation(float DeltaTime)
{
	if (IsGuardActive())
	{
		SetUseControllerRotationYaw(true);
		return;
		
	}
	else
	{
		SetUseControllerRotationYaw(false);
	}
	
	if (CharacterState == ECharacterState::IdleWalkRun || bCharacterStateAllowsRotation)
	{
		SetUseControllerRotationYaw(false);
		FRotator DesiredRotation = FRotator(0.f, GetRotationYawFromAxisInput(), 0.f);
		UEODCharacterMovementComponent* MoveComp = Cast<UEODCharacterMovementComponent>(GetCharacterMovement());
		if (MoveComp)
		{
			MoveComp->SetDesiredCustomRotation(DesiredRotation);
		}
	}
}

void AEODCharacterBase::UpdateMovement(float DeltaTime)
{
	UStatsComponentBase* StatsComp = GetCharacterStatsComponent();
	if (!StatsComp)
	{
		return;
	}	

	if (IsGuardActive())
	{
		float NewSpeed = DefaultWalkSpeedWhileBlocking * StatsComp->GetMovementSpeedModifier();
		SetWalkSpeed(NewSpeed);
	}
	else if (CharacterState == ECharacterState::IdleWalkRun || bCharacterStateAllowsMovement)
	{
		UpdatePCTryingToMove();
		UpdateCharacterMovementDirection();

		if (ForwardAxisValue < 0)
		{
			float NewSpeed = (DefaultWalkSpeed * StatsComp->GetMovementSpeedModifier()) * (5.f / 16.f);
			SetWalkSpeed(NewSpeed);
		}
		else
		{
			float NewSpeed = DefaultWalkSpeed * StatsComp->GetMovementSpeedModifier();
			SetWalkSpeed(NewSpeed);
		}
	}
}

void AEODCharacterBase::UpdateFallState(float DeltaTime)
{
	SetCharacterStateAllowsMovement(false);
	SetCharacterStateAllowsRotation(false);
}

void AEODCharacterBase::TriggerInteraction()
{
	// If Character is already interacting
	if (GetCharacterState() == ECharacterState::Interacting)
	{
		UpdateInteraction();
	}
	else
	{
		StartInteraction();
	}
}

void AEODCharacterBase::StartInteraction()
{
}

void AEODCharacterBase::UpdateInteraction()
{
}

void AEODCharacterBase::StopInteraction()
{
}

void AEODCharacterBase::ToggleSheathe()
{
}

void AEODCharacterBase::StartNormalAttack()
{
}

void AEODCharacterBase::StopNormalAttack()
{
}

void AEODCharacterBase::UpdateNormalAttackState(float DeltaTime)
{
}

void AEODCharacterBase::PlayToggleSheatheAnimation()
{
}

void AEODCharacterBase::InitiateRotationToYawFromAxisInput()
{
	float UpdatedYaw = GetRotationYawFromAxisInput();
	FRotator DesiredRotation = FRotator(0.f, GetRotationYawFromAxisInput(), 0.f);
	UEODCharacterMovementComponent* MoveComp = Cast<UEODCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->SetDesiredCustomRotation(DesiredRotation);
	}
}

void AEODCharacterBase::Jump()
{


	/*
	if (IsGuardActive())
	{
		DeactivateGuard();
	}
	*/

	// SetCharacterStateAllowsRotation(false);
	// Super::Jump();
}

void AEODCharacterBase::OnPressedForward()
{
}

void AEODCharacterBase::OnPressedBackward()
{
}

void AEODCharacterBase::OnReleasedForward()
{
}

void AEODCharacterBase::OnReleasedBackward()
{
}

void AEODCharacterBase::UpdateIdleWalkRunState(float DeltaTime)
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		if (ForwardAxisValue < 0)
		{
			SetWalkSpeed(DefaultWalkSpeed * 5.f / 16.f);
		}
		else
		{
			SetWalkSpeed(DefaultWalkSpeed);
		}

		FRotator DesiredRotation = FRotator(0.f, GetRotationYawFromAxisInput(), 0.f);
		UEODCharacterMovementComponent* MoveComp = Cast<UEODCharacterMovementComponent>(GetCharacterMovement());
		if (MoveComp)
		{
			MoveComp->SetDesiredCustomRotation(DesiredRotation);
		}
		UpdateCharacterMovementDirection();
	}
}

void AEODCharacterBase::SpawnAndMountRideableCharacter(TSubclassOf<ARideBase> RideCharacterClass)
{
	// Only call the server RPC if RideCharacterClass points to a valid class
	if (Role < ROLE_Authority && RideCharacterClass.Get())
	{
		Server_SpawnAndMountRideableCharacter(RideCharacterClass);
		return;
	}

	//~ @todo Cleanup/destroy ride spawned previously

	UWorld* World = GetWorld();
	AController* PlayerController = GetController();
	if (World && PlayerController && RideCharacterClass.Get())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.Owner = this->Controller;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		CurrentRide = World->SpawnActor<ARideBase>(RideCharacterClass, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (CurrentRide)
		{
			PlayerController->Possess(CurrentRide);
			CurrentRide->MountCharacter(this);
		}
	}
}

void AEODCharacterBase::OnMountingRide(ARideBase* RideCharacter)
{
	if (IsValid(RideCharacter))
	{
		UCharacterMovementComponent* MoveComp = GetCharacterMovement();
		UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
		// Disable movement and collision
		if (MoveComp && CapsuleComp)
		{
			MoveComp->DisableMovement();
			CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(RideCharacter->MountedCharacter_IdealAnimation);
		}
	}
}

void AEODCharacterBase::UpdateGuardState(float DeltaTime)
{
	// If character wants to guard but the guard is not yet active 
	if (bWantsToGuard && !IsGuardActive() && CanGuardAgainstAttacks())
	{
		ActivateGuard();
	}
	else if (!bWantsToGuard && IsGuardActive())
	{
		DeactivateGuard();
	}

	if (IsGuardActive())
	{
		if (ForwardAxisValue == 0)
		{
			if (RightAxisValue > 0)
			{
				if (BlockMovementDirectionYaw != 90.f)
					SetBlockMovementDirectionYaw(90.f);
			}
			else if (RightAxisValue < 0)
			{
				if (BlockMovementDirectionYaw != -90.f)
					SetBlockMovementDirectionYaw(-90.f);
			}
			else
			{
				if (BlockMovementDirectionYaw != 0.f)
					SetBlockMovementDirectionYaw(0.f);
			}
		}
		else
		{
			float NewYaw = FMath::RadiansToDegrees(FMath::Atan2(RightAxisValue, ForwardAxisValue));
			if (BlockMovementDirectionYaw != NewYaw)
			{
				SetBlockMovementDirectionYaw(NewYaw);
			}
		}
	}
}

void AEODCharacterBase::Server_SetIsRunning_Implementation(bool bValue)
{
	SetIsRunning(bValue);
}

bool AEODCharacterBase::Server_SetIsRunning_Validate(bool bValue)
{
	return true;
}

void AEODCharacterBase::Server_SetCharacterStateAllowsMovement_Implementation(bool bNewValue)
{
	SetCharacterStateAllowsMovement(bNewValue);
}

bool AEODCharacterBase::Server_SetCharacterStateAllowsMovement_Validate(bool bNewValue)
{
	return true;
}

void AEODCharacterBase::Server_SetPCTryingToMove_Implementation(bool bNewValue)
{
	SetPCTryingToMove(bNewValue);
}

bool AEODCharacterBase::Server_SetPCTryingToMove_Validate(bool bNewValue)
{
	return true;
}

void AEODCharacterBase::Server_SetCharMovementDir_Implementation(ECharMovementDirection NewDirection)
{
	SetCharacterMovementDirection(NewDirection);
}

bool AEODCharacterBase::Server_SetCharMovementDir_Validate(ECharMovementDirection NewDirection)
{
	return true;
}

void AEODCharacterBase::Server_SetWeaponSheathed_Implementation(bool bNewValue)
{
	SetWeaponSheathed(bNewValue);
}

bool AEODCharacterBase::Server_SetWeaponSheathed_Validate(bool bNewValue)
{
	return true;
}

void AEODCharacterBase::Server_SetGuardActive_Implementation(bool bValue)
{
	SetGuardActive(bValue);
}

bool AEODCharacterBase::Server_SetGuardActive_Validate(bool bValue)
{
	return true;
}

void AEODCharacterBase::Server_SetBlockMovementDirectionYaw_Implementation(float NewYaw)
{
	SetBlockMovementDirectionYaw(NewYaw);
}

bool AEODCharacterBase::Server_SetBlockMovementDirectionYaw_Validate(float NewYaw)
{
	return true;
}

void AEODCharacterBase::Multicast_SetCharacterRotation_Implementation(FRotator NewRotation)
{
	if (!IsLocallyControlled())
	{
		SetCharacterRotation(NewRotation);
	}
}
