// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameInstance.h"
#include "oneM2MReceiverThread.h"

UMyGameInstance::UMyGameInstance() 
{
	
}

UMyGameInstance::~UMyGameInstance()
{
}

void UMyGameInstance::Init()
{
	Super::Init();

	// Spawn thisActor in the world
	thisActor = GetWorld()->SpawnActor<AAssosiateActor>(AAssosiateActor::StaticClass());
	
	// Operation oneM2M Receiver Thread
	oneM2MReceiverThread = new FoneM2MReceiverThread(this->GetWorld());
}

void UMyGameInstance::Shutdown()
{
	//if (oneM2MReceiverThread)
	if(oneM2MReceiverThread)
	{
		oneM2MReceiverThread->Stop();
		oneM2MReceiverThread->Exit();
		delete oneM2MReceiverThread;
		oneM2MReceiverThread = nullptr;
	}
	
	Super::Shutdown();
}