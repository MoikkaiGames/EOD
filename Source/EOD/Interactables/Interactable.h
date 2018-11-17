// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

class AEODCharacterBase;

UCLASS()
class EOD_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Called when a character attempts to interact with this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Interaction)
	void OnInteract(const AEODCharacterBase* Character);

	virtual void OnInteract_Implementation(const AEODCharacterBase* Character);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Interaction)
	void EnableCustomDepth();

	virtual void EnableCustomDepth_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Interaction)
	void DisableCustomDepth();

	virtual void DisableCustomDepth_Implementation();

};
