// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateTimelineComponent.h"

// Sets default values for this component's properties
UCreateTimelineComponent::UCreateTimelineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCreateTimelineComponent::CreateTimeline(double duration)
{
	FTimeline timeline;
	UCurveFloat CurveFloat;

	timeline.SetLooping(false);
	timeline.SetTimelineLength(duration);
	timeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	
	CurveFloat.FloatCurve.AddKey(0.0f, 0.0f);
	CurveFloat.FloatCurve.AddKey(duration, 1.0f);
}

void UCreateTimelineComponent::HandleProgress(float Value)
{
	return;
}

// Called when the game starts
void UCreateTimelineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCreateTimelineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

