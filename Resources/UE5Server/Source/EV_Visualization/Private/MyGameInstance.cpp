// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SocketThread.h"
#include <Ws2tcpip.h>

UMyGameInstance::UMyGameInstance()
{
}

UMyGameInstance::~UMyGameInstance()
{
}

void UMyGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Warning, TEXT("Init Started"));
	UE_LOG(LogTemp, Warning, TEXT("Init Ended"));
}

void UMyGameInstance::OnStart()
{
	Super::OnStart();
	UE_LOG(LogTemp, Warning, TEXT("On Start Ended"));
}

bool UMyGameInstance::BuildingNameExists(const FString& BuildingName) const
{
	return true;
}

void UMyGameInstance::GenerateNewLevel(const FString& BuildingName)
{
}