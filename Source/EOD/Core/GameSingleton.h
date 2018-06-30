// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "UObject/NoExportTypes.h"
#include "GameSingleton.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class EOD_API UGameSingleton : public UObject
{
	GENERATED_BODY()
	
public:

	UGameSingleton(const FObjectInitializer& ObjectInitializer);

	/** A singleton instance of FStreamableManager */
	FStreamableManager StreamableManager;
	
	UPROPERTY(EditDefaultsOnly, Category = PlayerAnimationsReferences)
	UDataTable* PlayerAnimationReferencesDataTable;
	
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	UDataTable* WeaponDataTable;

};
