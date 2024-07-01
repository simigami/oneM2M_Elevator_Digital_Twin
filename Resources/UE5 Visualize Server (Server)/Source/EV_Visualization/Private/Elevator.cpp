// Fill out your copyright notice in the Description page of Project Settings.


#include "Elevator.h"

// Sets default values
AElevator::AElevator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AElevator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AElevator::Init()
{
	//Timeline = FTimeline{};
	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComp"));
	CurveFloat = nullptr;

	progressFunction.BindUFunction(this, "TimelineProgress");
	progressEnd.BindUFunction(this, "TimelineEnd");
	
}

void AElevator::TimelineProgress(float Value)
{
	if (!StartTransform.IsValid() || !DestTransform.IsValid())
	{
		return;
	}

	else
	{
		//StartTranslation = {7818.979781781909, 10141.729834779555 ,339.99999971176351};
		//DestTranslation = {7818.979781781909 ,10141.729834779555 ,3703.5};

		StartTranslation = StartTransform.GetTranslation();
		DestTranslation = DestTransform.GetTranslation();
	
		// Interpolate between Start and Dest transforms using timeline value
		FVector interpTranslation = FMath::Lerp(StartTranslation,DestTranslation, Value);

		// Update actor's location with the interpolated transform
		SetActorLocation(interpTranslation);	
	}
}

void AElevator::TimelineEnd()
{
	flag = false;
	UE_LOG(LogTemp, Log, TEXT("Timeline Ended\n"));
}

void AElevator::SetupTimeline(float tta, float ttm, float ttd, float current_Z, float dest_Z)
{
	CurveFloat = nullptr;
	
	progressFunction.BindUFunction(this, "TimelineProgress");
	progressEnd.BindUFunction(this, "TimelineEnd");

	UE_LOG(LogTemp, Log, TEXT("Timeline Progress Started\n"));
	if (ttm > 0)
	{
		// Create a curve float object
		CurveFloat = NewObject<UCurveFloat>();

		// Calculate intermediate values
		const double deltaDistance = FMath::Abs(current_Z-dest_Z);
		const double timetomax = (max_velocity / acceleration);
		const double timenow = timetomax - tta;
		const double timeend = timetomax - ttd;

		const double tta_distance = 0.5 * acceleration * (FMath::Pow(timetomax, 2) - FMath::Pow(timenow, 2));
		const double ttm_distance = max_velocity * ttm;
		const double ttd_distance = 0.5 * acceleration * (FMath::Pow(timetomax, 2) - FMath::Pow(timeend, 2));

		const double tot = tta_distance + ttm_distance + ttd_distance;
		const double tta_and_ttd_per = tta_distance / tot;
		const double ttm_per = 1 - 2 * tta_and_ttd_per;
		
		// round tta + ttm + ttd to 2 decimal places
		double temp = tta + ttm + ttd;
		const double rounded = FMath::RoundToDouble(temp * 100.0f) / 100.0f;

		// LOG immediate values
		UE_LOG(LogTemp, Log, TEXT("total dst is : %.2f\n"), tot);
		UE_LOG(LogTemp, Log, TEXT("tta and ttd per is : %.2f\n"), tta_and_ttd_per);
		UE_LOG(LogTemp, Log, TEXT("ttm per is : %.2f\n"), ttm_per);
		
		if(temp == 4.38)
		{
			UE_LOG(LogTemp, Log, TEXT("Timeline Length is : %.2f\n"), temp);
		}
		
		// To-Do: Fix Error in Timeline Length Calculation
		// Add keyframes to the curve
		CurveFloat->FloatCurve.AddKey(0.0f, 0.0f);
		CurveFloat->FloatCurve.AddKey(tta, tta_and_ttd_per);
		CurveFloat->FloatCurve.AddKey(tta + ttm, tta_and_ttd_per+ttm_per);
		CurveFloat->FloatCurve.AddKey(tta + ttm + ttd, 1.0f);

		// Bind the curve to the timeline
		TimelineComponent->AddInterpFloat(CurveFloat, progressFunction, FName{TEXT("ABCD")});
		TimelineComponent->SetTimelineFinishedFunc(progressEnd);

		// Set playback settings
		TimelineComponent->SetLooping(false);
		TimelineComponent->SetTimelineLength(tta + ttm + ttd);

		UE_LOG(LogTemp, Log, TEXT("Timeline Length is : %.2f\n"), tta + ttm + ttd);

		TimelineComponent->PlayFromStart();
	}

	else
	{
		// Create a curve float object
		CurveFloat = NewObject<UCurveFloat>();

		// Calculate intermediate values

		// Add keyframes to the curve
		CurveFloat->FloatCurve.AddKey(0.0f, 0.0f);
		CurveFloat->FloatCurve.AddKey(tta, 0.5f);
		CurveFloat->FloatCurve.AddKey(tta + ttd, 1.0f);

		// Bind the curve to the timeline
		TimelineComponent->AddInterpFloat(CurveFloat, progressFunction, FName{TEXT("ABCD")});
		TimelineComponent->SetTimelineFinishedFunc(progressEnd);

		// Set playback settings
		TimelineComponent->SetLooping(false);
		TimelineComponent->SetTimelineLength(tta +  ttd);

		UE_LOG(LogTemp, Error, TEXT("Timeline Length is : %.2f\n"), tta + ttd);

		TimelineComponent->PlayFromStart();
	}

	count += 1;
	UE_LOG(LogTemp, Log, TEXT("Timeline Count is : %d\n"), count);
}
