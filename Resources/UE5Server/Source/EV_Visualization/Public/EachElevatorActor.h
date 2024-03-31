// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyActor.h"
#include "EachElevatorActor.generated.h"

/**
 * 
 */

UCLASS()
class EV_VISUALIZATION_API AEachElevatorActor : public AActor
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintCallable, Category = "Elevator")
	void Init(FString ThisElevatorName);

	UFUNCTION(BlueprintCallable, Category = "Elevator")
	virtual void SetThisElevatorStatus(FStatus ThisElevator);

	UFUNCTION(BlueprintCallable, Category = "Elevator")
	void SetCurrentReceiveStatus(FStatus ThisElevator);

	UFUNCTION(BlueprintCallable, Category = "Elevator")
	FStatus GetThisElevatorStatus();

	UFUNCTION(BlueprintCallable, Category = "Elevator")
	FStatus GetCurrentRecieverStatus();

	UFUNCTION(BlueprintCallable, Category = "Elevator")
	FString GetElevatorName();
	
	UPROPERTY(BlueprintReadWrite, Category = "Elevator")
	FString BuildingName;
	
	UPROPERTY(BlueprintReadWrite, Category = "Elevator")
	FString ElevatorName;

	UPROPERTY(BlueprintReadWrite, Category = "Elevator")
	FStatus ThisElevatorStatus;
	
	UPROPERTY(BlueprintReadWrite, Category = "Elevator")
	FStatus CurrentReceiveStatus;
};
