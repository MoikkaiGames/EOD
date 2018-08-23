// Copyright 2018 Moikkai Games. All Rights Reserved.

#include "BTDecorator_CanMove.h"
#include "Player/EODCharacterBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CanMove::UBTDecorator_CanMove(const FObjectInitializer & ObjectInitializer): Super(ObjectInitializer)
{
}

bool UBTDecorator_CanMove::CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const
{
	// The owner of 'OwnerComp' is a controller (not pawn)
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	AEODCharacterBase* OwningCharacter = Cast<AEODCharacterBase>(AIController->GetPawn());

	return OwningCharacter->CanMove();
}
