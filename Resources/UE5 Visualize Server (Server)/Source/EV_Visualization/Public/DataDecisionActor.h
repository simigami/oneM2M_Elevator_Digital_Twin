// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataDecisionActor.generated.h"

USTRUCT(Blueprintable)
struct FDecisionStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString building_name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString device_name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString map_path = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int dilation = 0;
	
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
	int ground_floor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int underground_floor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double acceleration = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double max_velocity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double erd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double esd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double ed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<double> each_floor_altimeter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<double> each_floor_timeline;
};

UCLASS()
class EV_VISUALIZATION_API ADataDecisionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADataDecisionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DataDecision")
	FString building_name = "";
	
	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual FDecisionStruct deserialJSON(const FString& ReceivedJSON);

	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual void InitDecisionStruct();
	
	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual void setLatestDecisionStruct(FDecisionStruct& thisStruct);

	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual void setDecisionStruct(FDecisionStruct& thisStruct);

	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual FDecisionStruct getDecisionStruct();

	UFUNCTION(BlueprintCallable, Category = "DataDecision")
	virtual bool isDecisionStructChanged();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "DataDecision")
	void Execute(FDecisionStruct thisStruct);

	UFUNCTION(BlueprintCallable)
	virtual TArray<double> getEachFloorTimeLines(FDecisionStruct this_status, const int current_floor_index, const int goTo_floor_index);

private:
	FDecisionStruct decisionStruct;
	FDecisionStruct latestDecisionStruct;

};
