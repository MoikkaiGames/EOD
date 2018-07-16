// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

/**
 * HUDWidget is used to display player HUD
 */
UCLASS()
class EOD_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UHUDWidget(const FObjectInitializer& ObjectInitializer);

	bool Initialize() override;
	
protected:

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ManaBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaBar;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LevelText;

	
	/** TextBlock to display current/max health */
	// UPROPERTY(meta = (BindWidget))
	// class UTextBlock* HealthText;
	
	/** TextBlock to display current/max mana */
	// UPROPERTY(meta = (BindWidget))
	// class UTextBlock* ManaText;
	
	/** TextBlock to display current/max stamina */
	// UPROPERTY(meta = (BindWidget))
	// class UTextBlock* StaminaText;
	
	
};
