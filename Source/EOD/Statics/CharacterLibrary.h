// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponLibrary.h"
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

	//~ @note Words like `JumpStart` and `JumpEnd` have been intentionally capitalized because they are animation montage section names

	/** Contains animations for player JumpStart, JumpLoop, and JumpEnd */
	UAnimMontage* AnimationMontage_Jump;
	
	/** Contains animations for ForwardDodge, BackwardDodge, LeftDodge, and RightDodge */
	UAnimMontage* AnimationMontage_Dodge;
	
	/** Contains animations for:
	 * FirstSwing, FirstSwingEnd
	 * SecondSwing, SecondSwingEnd
	 * ...
	 * ForwardSwing, ForwardSwingEnd
	 * BackwardSwing, BackwardSwingEnd
	 * ...
	 */
	UAnimMontage* AnimationMontage_NormalAttacks;

	//~ @note Add AnimationMontage_WeaponChange animations here
	//~ @todo List montage section names for AnimationMontage_SpecialActions
	UAnimMontage* AnimationMontage_SpecialActions;
	
	/**
	 * Contains animations for instant skills.
	 * Section name will be same as skill name
	 */
	UAnimMontage* AnimationMontage_Skills;
	
	/**
	 * Contains animations for spells
	 * Section name will be same as spell name
	 */
	UAnimMontage* AnimationMontage_Spells;
	
	//~ @todo documentation
	UAnimMontage* AnimationMontage_SpecialMovement;
	
	//~ @todo List montage section names for AnimationMontage_CrowdControlEffects
	/** Contains animations for crowd control effects like interrupted, frozen, etc. */
	UAnimMontage* AnimationMontage_CrowdControlEffects;
	
	/** Contains animations for player flinching */
	UAnimMontage* AnimationMontage_Flinch;

};

/** This struct contains info related to in-game class skills */
USTRUCT(BlueprintType)
struct EOD_API FSkillInfoTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FSoftObjectPath Icon;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FString InGameName;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FString Description;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	FString MontageSectionName;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	int StaminaRequired;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	int ManaRequired;
	
	UPROPERTY(EditAnywhere, Category = BaseInfo)
	float Cooldown;
	
	// @todo skill type (support/damage)

	// @todo instant use skill? spell?

	// UPROPERTY(EditAnywhere, Category = BaseInfo)
	// @todo damage type (magick/physical)

	// @todo damage info

	// @todo crowd control effect

	// @todo crown control immunities

	// error - Issue #6
	// UPROPERTY(EditAnywhere, Category = BaseInfo)
	// TSubclassOf<class UStatusEffect> StatusEffect;

	// @todo Skill max level up and changes that occur at each level
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
