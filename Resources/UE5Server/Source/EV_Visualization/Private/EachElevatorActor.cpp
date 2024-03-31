// Fill out your copyright notice in the Description page of Project Settings.


#include "EachElevatorActor.h"

void AEachElevatorActor::Init(FString ThisElevatorName)
{
	ThisElevatorStatus = FStatus();
	CurrentReceiveStatus = FStatus();
	ElevatorName = ThisElevatorName; 
}

void AEachElevatorActor::SetThisElevatorStatus(FStatus Retrievedstatus)
{
	ThisElevatorStatus = Retrievedstatus;
}

void AEachElevatorActor::SetCurrentReceiveStatus(FStatus Retrievedstatus)
{
	CurrentReceiveStatus = Retrievedstatus;
}

FStatus AEachElevatorActor::GetThisElevatorStatus()
{
	return ThisElevatorStatus;
}

FStatus AEachElevatorActor::GetCurrentRecieverStatus()
{
	return CurrentReceiveStatus;
}

FString AEachElevatorActor::GetElevatorName()
{
	return ElevatorName;
}
