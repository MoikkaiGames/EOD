// Copyright 2018 Moikkai Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "EODItemDragDropOperation.generated.h"

class UEODItemContainer;

/**
 * 
 */
UCLASS()
class EOD_API UEODItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:

	UEODItemDragDropOperation(const FObjectInitializer& ObjectInitializer);
	
	/** Reference to EOD item that is being dragged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Payload)
	UEODItemContainer* DraggedEODItemWidget;

	
};