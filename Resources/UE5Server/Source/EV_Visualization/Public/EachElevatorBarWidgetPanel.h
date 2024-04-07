// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EachElevatorBarWidgetPanel.generated.h"

/**
 * 
 */
UCLASS()
class EV_VISUALIZATION_API UEachElevatorBarWidgetPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	FString BuildingNameString;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	FString ElevatorNameString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	double ErdDouble;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	double EsdDouble;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	double EdDouble;
};
