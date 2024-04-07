// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataDecisionActor.h"
#include "EachElevatorBarWidgetPanel.h"
#include "EnergyConsumptionPanel.generated.h"

/**
 * 
 */
UCLASS()
class EV_VISUALIZATION_API UEnergyConsumptionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	// A Function that gets all widget class of  BP_EachElevatorBarWidgetPanel and sort them by building name and elevator name or energy consumption 
	UFUNCTION(BlueprintCallable)
	TArray<UEachElevatorBarWidgetPanel*> GetSortedElevatorBarWidgetPanels(bool updown, int sortType);

	// A Function That Returns UEachElevatorBarWidgetPanel that holds specific building name and elevator name
	UFUNCTION(BlueprintCallable)
	UEachElevatorBarWidgetPanel* GetElevatorBarWidgetPanel(FString buildingName, FString elevatorName);

	// A Function that sets members of BP_EachElevatorBarWidgetPanel
	UFUNCTION(BlueprintCallable)
	void SetElevatorBarWidgetPanelMembers(FDecisionStruct decisionStruct, UEachElevatorBarWidgetPanel* elevatorBarWidgetPanel);
	
	// A TArray That Conatins All Widget Class of BP_EachElevatorBarWidgetPanel
	UPROPERTY(BlueprintReadWrite)
	TArray<UEachElevatorBarWidgetPanel*> ElevatorBarWidgetPanels;
};