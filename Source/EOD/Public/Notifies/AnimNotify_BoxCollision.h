// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatLibrary.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_BoxCollision.generated.h"

/**
 * 
 */
UCLASS()
class EOD_API UAnimNotify_BoxCollision : public UAnimNotify
{
	GENERATED_BODY()
	
public:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
	/** Position of collision box's center */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BoxInfo)
	FVector Center;
	
	/** Half extent of collision box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BoxInfo)
	FVector BoxHalfExtent;
	
	/** Rotation of collision box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BoxInfo)
	FRotator Rotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SkillInfo)
	FCollisionSkillInfo SkillInfo;

};
