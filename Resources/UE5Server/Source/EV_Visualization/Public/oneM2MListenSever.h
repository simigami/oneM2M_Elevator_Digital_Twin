// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "GameFramework/Actor.h"
#include "oneM2MListenSever.generated.h"

UCLASS()
class EV_VISUALIZATION_API AoneM2MListenSever : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AoneM2MListenSever();

	// WinSOCK Properties
	WSADATA wsaData;
	SOCKET ListenSocket;
	SOCKET AcceptSocket;

	// Buffer
	char RecvBuffer[1024];

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IP Used in SOCKET
	const int PORT = 10052;

};
