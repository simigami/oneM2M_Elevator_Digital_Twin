// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class EV_VISUALIZATION_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UMyGameInstance();
	~UMyGameInstance();
	
public:
	virtual void Init() override;
	void OnStart() override;
	//EV_struct deserialJSON(const FString& ReceivedJSON);
	
	//EV_struct struct_EV;
	TArray<FString> BuildingLevels;

	bool BuildingNameExists(const FString& BuildingName) const;

	void GenerateNewLevel(const FString& BuildingName);
	
	SOCKET Socket;
	WSADATA wsaData;

private:
};
