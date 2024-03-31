// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

USTRUCT(Blueprintable)
struct FStatus
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool mapExists = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ElevatorExists = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int default_start_floor_index = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int goToFloor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double tta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double ttm = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double ttd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString building_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString device_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString map_path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ground_floor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int underground_floor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double acceleration = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double max_velocity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<double> each_floor_altimeter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<double> each_floor_timeline;
};

class AEachElevatorActor;

UCLASS()
class EV_VISUALIZATION_API AMyActor : public AActor
{
	GENERATED_BODY()
public:
	AMyActor();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual  void Tick(float DeltaSeconds) override;
	
	bool BuildingNameExists(const FString& BuildingName);
	
	UFUNCTION(BlueprintCallable)
	virtual void RunSocket();

	UFUNCTION(BlueprintCallable)
	virtual void printAllStats();

	UFUNCTION(BlueprintCallable)
	virtual void spawnEachElevatorActor(FString ThisElevatorName, FStatus this_status);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GetStats")
	void Execute();
	virtual void Execute_Implementation();

	UFUNCTION(BlueprintCallable)
	const UObject* GetWorldContextObjectFromPath(const FString& MapPath);

	UFUNCTION(BlueprintCallable)
	void ChangeEachFloorAltimeterToABS();

	//BlueprintImplementable Function
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "GetStats")
	AActor* GetActorFromWorld();

	UFUNCTION(BlueprintCallable)
	virtual TArray<double> getEachFloorTimeLines(FStatus this_status, const int current_floor_index, const int goTo_floor_index);

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual bool bStatusExists() { return (status.building_name == "" && status.device_name == "") ? false : true; }

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual void SetMapPath();

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual bool CheckAndAppendMapList();

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	void emptyMapList();

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual void FilpFlopMapList(FString building_name);
	
	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual FStatus GetStatus() const { return status; }

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual void SetStatus(FStatus newStatus) { status = newStatus; }

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	virtual void InitStatus() {
		status.building_name = "";
		status.device_name = "";
		status.ground_floor = 0;
		status.underground_floor = 0;
		status.acceleration = 0;
		status.max_velocity = 0;
		status.goToFloor = 0;
		status.default_start_floor_index = 0;
		status.tta = 0;
		status.ttm = 0;
		status.ttd = 0;
		status.mapExists = false;
		status.ElevatorExists = false;
	}

	UPROPERTY(BlueprintReadWrite, Category = "GetStats")
	bool bIsFunctionRunning = false;

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	void SetFunctionRunning(const bool bIsRunning) { this->bIsFunctionRunning = bIsRunning; }

	UFUNCTION(BlueprintCallable, Category = "GetStats")
	bool GetFunctionRunning() { return bIsFunctionRunning; }

protected:
	class FSocketThread* ReceiveThread;
	const double reality_constant = 125.5;

public:
	TArray<FString> BuildingLevels;
	
	UPROPERTY(EditAnywhere)
	FStatus status;

	UPROPERTY(EditAnywhere)
	TMap<FString, bool> loaded_building_list;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AEachElevatorActor>> EachsubElevatorActorClass;
};
