// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "Elevator.generated.h"

UCLASS()
class EV_VISUALIZATION_API AElevator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AElevator();

	FOnTimelineFloat progressFunction;
	FOnTimelineEvent progressEnd;

	UPROPERTY(BlueprintReadWrite)
	bool flag = true;

	UPROPERTY()
	FTimeline Timeline;

	UPROPERTY()
	UTimelineComponent* TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComp"));

	UPROPERTY(EditAnywhere)
	UCurveFloat* CurveFloat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ground_floor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int underground_floor = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double acceleration = 1.25;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double max_velocity = 2.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double realityConstant = 125.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<double> eachfloorAlt;
	
	UPROPERTY(BlueprintReadWrite)
	FTransform StartTransform;
	
	UPROPERTY(BlueprintReadWrite)
	FTransform DestTransform;

	UPROPERTY(BlueprintReadWrite)
	FVector StartTranslation;

	UPROPERTY(BlueprintReadWrite)
	FVector DestTranslation;

	UFUNCTION()
	void TimelineProgress(float Value);

	UFUNCTION()
	void TimelineEnd();

	UFUNCTION(BlueprintCallable)
	void Init();

	UFUNCTION(BlueprintCallable)
	void SetupTimeline(float tta, float ttm, float ttd, float current_Z, float dest_Z);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int count = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
