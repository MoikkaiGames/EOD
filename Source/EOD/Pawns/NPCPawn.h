// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NPCPawn.generated.h"

UCLASS()
class EOD_API ANPCPawn : public APawn
{
	GENERATED_BODY()

public:

	ANPCPawn(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

};
