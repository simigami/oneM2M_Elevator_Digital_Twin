// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "HAL/Runnable.h"
#include "MyActor.h"
#include "MyGameInstance.h"

class FRunnableThread;

class EV_VISUALIZATION_API FSocketThread : public FRunnable
{

public:
	FSocketThread(AMyActor* Actor);
	~FSocketThread() override;

	bool Init() override;
	uint32 Run() override;

	bool BuildingNameExists(const FString& BuildingName);
	
	void Stop() override;
	void Exit() override;

	AMyActor* ThisThreadActor;
	UMyGameInstance* GameInstance;

	void deserialJSON(const FString& ReceivedJSON, FStatus& stats);
	
private:
	mutable FCriticalSection CriticalSection;
	
	SOCKET clientSocket;
	SOCKET acceptSocket;
	WSADATA wsaData;

	TArray<FString> BuildingLevels;

	bool bIsFinished;
	
	FRunnableThread* Thread;
};