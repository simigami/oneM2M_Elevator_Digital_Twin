// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "oneM2MReceiverThread.h"
#include "AssosiateActor.h"
#include "Engine/GameInstance.h"
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
	virtual void Shutdown() override;

	AAssosiateActor* thisActor;
	FoneM2MReceiverThread* oneM2MReceiverThread;
};
