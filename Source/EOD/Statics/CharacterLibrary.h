// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponLibrary.h"
#include "CombatLibrary.h"
#include "UObject/NoExportTypes.h"
#include "CharacterLibrary.generated.h"

class UAnimMontage;

/**
 * This enum describes the character movement direction relative to character's line of sight.
 * e.g, If the character is moving to the left of the direction it is looking in then ECharMovementDirection would be Left.
 */
UENUM(BlueprintType)
enum class ECharMovementDirection : uint8
{
	None,
	F 		UMETA(DisplayName = "Forward"),
	B 		UMETA(DisplayName = "Backward"),
	L 		UMETA(DisplayName = "Left"),
	R 		UMETA(DisplayName = "Right"),
	FL 		UMETA(DisplayName = "Forward Left"),
	FR 		UMETA(DisplayName = "Forward Right"),
	BL 		UMETA(DisplayName = "Backward Left"),
	BR 		UMETA(DisplayName = "Backward Right"),
};

/** This enum describes the current action/state of character */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	IdleWalkRun,
	AutoRun,
	Jumping,
	Dodging,
	Blocking,
	Attacking,
	Looting,
	SpecialAction,
	Interacting, 			// Interacting with another character, i.e., engaged in dialogue
	UsingActiveSkill,
	CastingSpell,
	SpecialMovement,
	GotHit,
	Dead
};

/**
 * This struct holds strong pointers to animation montages that are compatible with player's current equipped weapon.
 * @note This struct does not contain pointer to facial animations because facial animations are independent of the weapon equipped.
 */
USTRUCT(BlueprintType)
struct EOD_API FPlayerAnimationReferences
{
	GENERATED_USTRUCT_BODY()

public:

	//~ @note Words like `JumpStart` and `JumpEnd` have been intentionally capitalized because
	//~ they are animation montage section names

	/** Contains animations for player JumpStart, JumpLoop, and JumpEnd */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_Jump;
	
	/** Contains animations for ForwardDodge, BackwardDodge, LeftDodge, and RightDodge */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_Dodge;
	
	/** Contains animations for:
	 * FirstSwing, FirstSwingEnd
	 * SecondSwing, SecondSwingEnd
	 * ...
	 * ForwardSwing, ForwardSwingEnd
	 * BackwardSwing, BackwardSwingEnd
	 * ...
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_NormalAttacks;

	//~ @note Add AnimationMontage_WeaponChange animations here
	//~ @todo List montage section names for AnimationMontage_SpecialActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_SpecialActions;
	
	/**
	 * Contains animations for instant skills.
	 * Section name will be same as skill name
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_Skills;
	
	/**
	 * Contains animations for spells
	 * Section name will be same as spell name
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_Spells;
	
	//~ @todo documentation
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_SpecialMovement;
	
	//~ @todo List montage section names for AnimationMontage_CrowdControlEffects
	/** Contains animations for crowd control effects like interrupted, frozen, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_CrowdControlEffects;
	
	/** Contains animations for player flinching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimationMontage_Flinch;

	/** Handle proper destruction of player animation references */
	~FPlayerAnimationReferences();

	// @todo Death animations

};


/**
 * This struct is equivalent of FPlayerAnimationReferences except that it contains
 * string references to animations instead of strong pointers. This will be used to
 * construct the player animation references data table
 * @see FPlayerAnimationReferences
 */
USTRUCT(BlueprintType)
struct EOD_API FPlayerAnimationSoftReferences : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	
	/** Reference to player animation montage that contains animations for jumping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Jump;
	
	/** Reference to player animation montage that contains animations for dodging */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Dodge;
	
	/** Reference to player animation montage that contains animations for normal attacks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath NormalAttacks;

	//~ @note Add AnimationMontage_WeaponChange animations here
	/** Reference to player animation montage that contains animations for special actions (@todo list special actions) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath SpecialActions;
	
	/** Reference to player animation montage that contains animations for using weapon skils */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Skills;
	
	/** Reference to player animation montage that contains animations for spells */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Spells;
	
	/** Reference to player animation montage that contains animations for special movement (@todo more info) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath SpecialMovement;
	
	/** Reference to player animation montage that contains animations for crowd control effects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath CrowdControlEffects;
	
	/** Reference to player animation montage that contains animations for flinching */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath Flinch;

	// @todo Death animations

};

UENUM(BlueprintType)
enum class ESkillCastType : uint8
{
	// Skills that are cast instantly, i.e., almost all physical attacks and magic attacks such as rapid blast and divine punishment
	InstantCast,
	// Skills that require time to cast, i.e., spells
	DelayedCast,
	// Passives skill that can't be cast
	NoCast
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical,
	Magickal
};

/** This struct contains info related to in-game class skills */
USTRUCT(BlueprintType)
struct EOD_API FSkillLevelUpInfo
{
	GENERATED_USTRUCT_BODY()

public:

	/** Minimum stamina required to use this skill */
	UPROPERTY(EditDefaultsOnly)
	int StaminaRequired;
	
	/** Minimum mana required to use this skill */
	UPROPERTY(EditDefaultsOnly)
	int ManaRequired;
	
	/** Skill cooldown */
	UPROPERTY(EditDefaultsOnly)
	float Cooldown;

	/** Damage in percentage of player's magickal or physical attack that will inflicted on enemy */
	UPROPERTY(EditDefaultsOnly)
	float DamagePercent;

	/** Crowd control effect on hit */
	ECrowdControlEffect CrowdControlEffect;

	/** Immunities from crowd control effects granted on using this skill */
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "ECrowdControlEffect"))
	uint8 CrowdControlImmunities;

	/** Status effect that this skill triggers */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UStatusEffectBase> StatusEffect;

};

/** This struct contains info related to in-game class skills */
USTRUCT(BlueprintType)
struct EOD_API FSkillInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FSoftObjectPath Icon;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FString InGameName;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FString Description;
	
	/** The montage that contains the animation(s) for this skill */
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FName SkillStartMontageSectionName;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FName SkillLoopMontageSectionName;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FName SkillEndMontageSectionName;		
	
	/** Type of damage inflicted from this skill */
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	EDamageType DamageType;
	
	//~ Maximum number of level ups = SkillLevelUpsInfo.Num()
	UPROPERTY(EditAnywhere, Category = SkillInfo)
	TArray<FSkillLevelUpInfo> SkillLevelUpsInfo;


	// @todo What does this skill do (heal/damage/buff)? It will be useful for AI logic

};

/** Struct containing information of the skill that a character is currently using */
USTRUCT(BlueprintType)
struct FActiveSkill
{
	GENERATED_USTRUCT_BODY()

public:

	uint8 SkillIndex;

	FName CurrentSkillInUse;

	FName AnimationMontageSectionName;

	UAnimMontage* SkillAnimationMontage;

	UDataTable* SkillDataTableReference;

};

/**
 * CharacterLibrary contains static helper functions for in-game characters.
 * @note Do not derive from this class
 */
UCLASS()
class EOD_API UCharacterLibrary : public UObject
{
	GENERATED_BODY()
	
public:

	UCharacterLibrary(const FObjectInitializer& ObjectInitializer);
	
	//~ @note Blueprints don't support raw struct pointers, therefore it can't be BlueprintCallable
	/** Returns player animation references based on the EWeaponAnimationType of player */
	static FPlayerAnimationReferences* GetPlayerAnimationReferences(EWeaponAnimationType PlayerWeaponAnimationType);

};
