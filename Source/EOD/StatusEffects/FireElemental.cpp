// Copyright 2018 Moikkai Games. All Rights Reserved.

#include "FireElemental.h"
#include "Player/BaseCharacter.h"

#include "Engine/World.h"

TMap<TWeakObjectPtr<ABaseCharacter>, FStatusTimerInfo> UFireElemental::BurningCharactersMap = TMap<TWeakObjectPtr<ABaseCharacter>, FStatusTimerInfo>();

UFireElemental::UFireElemental()
{
	bTriggersOnCriticalHit = true;
	
	// The burning status can stack 3 times by default
	StackLimit = 3;
}

void UFireElemental::OnInitialize(ABaseCharacter* Owner, AActor* Instigator)
{
}

void UFireElemental::OnDeinitialize()
{
}

void UFireElemental::OnActivation(TArray<TWeakObjectPtr<ABaseCharacter>> RecipientCharacters)
{
	switch (ReactivationCondition)
	{
	case EStatusEffectReactivationCondition::None:
		for(TWeakObjectPtr<ABaseCharacter>& RecipientCharacter : RecipientCharacters)
		{
			if (BurningCharactersMap.Contains(RecipientCharacter))
			{
				continue;
			}


			


		}
		break;
	case EStatusEffectReactivationCondition::Reset:
		break;
	case EStatusEffectReactivationCondition::Stack:
		break;
	default:
		break;
	}
}

void UFireElemental::OnDeactivation()
{
}

void UFireElemental::ActivateBurnStatus(TWeakObjectPtr<ABaseCharacter> TargetCharacter)
{
	FTimerHandle* TimerHandle = new FTimerHandle;
	FTimerDelegate TimerDelegate;

	// TimerDelegate.BindUFunction(this, FName(""), )

	// TimerDelegate.BindUFunction()
	// 	NewDelegate.BindStatic(&APlayerCharacter::StaticTestFunction, HandlePtr, this); 
    // NewDelegate.BindUFunction(this, FName("LocalTestFunction"), THandle); 

	
    // GetWorld()->GetTimerManager().SetTimer(*HandlePtr, TimerDelegate, 2.f, true);

}

void UFireElemental::DeactivateBurnStatus(TWeakObjectPtr<ABaseCharacter> TargetCharacter)
{
}

void UFireElemental::OnBurn(FStatusTimerInfo & TimerInfo)
{
}

void UFireElemental::RemoveSingleStackOfBurn(ABaseCharacter * TargetCharacter)
{
}

void UFireElemental::RemoveAllStacksOfBurn(ABaseCharacter * TargetCharacter)
{
}
