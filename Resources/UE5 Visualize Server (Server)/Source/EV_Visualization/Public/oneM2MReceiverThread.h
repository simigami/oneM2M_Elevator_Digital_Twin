// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssosiateActor.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include "HAL/Runnable.h"
#include "DataDecisionActor.h"

class EV_VISUALIZATION_API FoneM2MReceiverThread : public FRunnable
{

public:
	FoneM2MReceiverThread(UWorld* World);
	~FoneM2MReceiverThread() override;

	bool Init() override;
	uint32 Run() override;
	FDecisionStruct deserialJSON(const FString& ReceivedJSON);

	void Stop() override;
	
	void Exit() override;

	TSubclassOf<ADataDecisionActor> DataDecisionActorClass;

	UWorld* thisWorld;
	
private:
	FRunnableThread* Thread;
	FDecisionStruct data;
	AAssosiateActor* thisActor;
	
	mutable FCriticalSection CriticalSection;
	
	SOCKET clientSocket;
	SOCKET acceptSocket;
	WSADATA wsaData;

	bool bIsFinished;
};