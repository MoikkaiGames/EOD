// Copyright 2018 Moikkai Games. All Rights Reserved.


#include "NPCPawn.h"

ANPCPawn::ANPCPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANPCPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ANPCPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
