// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EODItemContainer.generated.h"

UENUM(BlueprintType)
enum class EEODItemType : uint8
{
	None,
	ActiveSkill,
	PassiveSkill,
	Armor,
	Weapon,
	Necklace,
	Belt,
	Ring,
	Earring,
	Ingredient,
	Consumable,
	QuestItem,
	Potion,
	Scrap,
};

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class EOD_API UEODItemContainer : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UEODItemContainer(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EODItemInfo)
	bool bCanBeClicked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EODItemInfo)
	bool bCanBeDragged;

	UPROPERTY(Transient, BlueprintReadWrite, Category = EODItemInfo)
	bool bInCooldown;

	UPROPERTY(Transient)
	float CooldownTimeRemaining;

	UPROPERTY(Transient)
	float CooldownInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EODItemInfo)
	FName EODItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EODItemInfo)
	EEODItemType EODItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EODItemInfo)
	UTexture* EODItemIcon;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	// UImage* ItemImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* ItemButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_StackCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* Text_Cooldown;

	UFUNCTION(BlueprintCallable, Category = EODItemContainer)
	void StartCooldown(float Duration, float Interval = 1.f);

	UFUNCTION(BlueprintCallable, Category = EODItemContainer)
	void StopCooldown();

private:

	FTimerHandle CooldownTimerHandle;

	// void UpdateItemImage();
	
	void UpdateItemButton();

	void UpdateCooldown();

};
