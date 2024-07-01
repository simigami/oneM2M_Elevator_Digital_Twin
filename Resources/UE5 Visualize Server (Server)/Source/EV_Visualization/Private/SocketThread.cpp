// Fill out your copyright notice in the Description page of Project Settings.

#include "SocketThread.h"
#include <Windows.h>

#include "EachElevatorActor.h"
#include "EngineUtils.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define TIMEOUT_MS 3000 // 타임아웃 시간 (3초)

FSocketThread::FSocketThread(AMyActor* Actor) : ThisThreadActor(Actor)
{
	Thread = FRunnableThread::Create(this, TEXT("TEST"));
}

FSocketThread::~FSocketThread()
{
	if(!bIsFinished)
	{
		closesocket(acceptSocket);
		closesocket(clientSocket);
		bIsFinished = true;
	}

	if (clientSocket != INVALID_SOCKET)
	{
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	// 스레드 종료
	if (Thread)
	{
		//PlatformProcess::Sleep(3.0f); 
		Thread->WaitForCompletion();
		Thread->Kill();
		delete Thread;
	}
}

bool FSocketThread::Init()
{
	bIsFinished = false;
	UE_LOG(LogNet, Warning, TEXT("Thread has been initialized"));
	return true;
}

bool FSocketThread::BuildingNameExists(const FString& BuildingName)
{
	return BuildingLevels.Contains(BuildingName);
}

uint32 FSocketThread::Run()
{
	struct sockaddr_in serverAddr;

	// WINSOCK 준비
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(result != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("WSAStartup failed: %s"), result);
		return 1;
	}

	// WINSOCK 생성
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(clientSocket == INVALID_SOCKET)
	{
		UE_LOG(LogTemp, Error, TEXT("Error creating socket\n"));
		WSACleanup();
		return 1;
	}

	// WINSOCK 소켓 포트에 바인딩
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(10052);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	result = bind(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to BIND a port"));
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	// 비동기 소켓으로 세팅
	WSAAsyncSelect(clientSocket, NULL, WM_USER, FD_ACCEPT);
	listen(clientSocket, SOMAXCONN);
	
	while(!bIsFinished)
	{
		while(true)
		{
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(clientSocket, &readSet);

			// SELECT를 이용하여 소켓을 10초 동안 LISTEN 상태로 유지
			result = select(0, &readSet, NULL, NULL, NULL);

			if(result == SOCKET_ERROR)
			{
				UE_LOG(LogTemp, Error, TEXT("ERROR IN SELECT CREATION..."));
				break;
			}
			// JSON 수집 시 데이터 처리
			else
			{
				FCriticalSection Mutex;
				//FScopeLock ScopeLock(&Mutex);
				
				UE_LOG(LogTemp, Log, TEXT("Connection request received, accepting connection"));
				
				sockaddr_in clientSockInfo;
				int clientSize = sizeof(clientSockInfo);
				
				u_long BlockingMode = 0;
				ioctlsocket(clientSocket, FIONBIO, &BlockingMode);
				
				acceptSocket = accept(clientSocket, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);

				if (acceptSocket == INVALID_SOCKET)  //INVALID_SOCKET = 18446744073709551615
				{
					UE_LOG(LogTemp, Error, TEXT("accept failed: %d"), WSAGetLastError());
					closesocket(acceptSocket);
					WSACleanup();
					break;
				}
				
				else
				{
					// While loop: 클라이언트의 메세지를 받아서 출력 후 클라이언트에 다시 보냅니다.
					enum eBufSize { BUF_SIZE = 4096 };
					char buf[BUF_SIZE];
					ZeroMemory(buf, BUF_SIZE);

					int bytesReceived = recv(acceptSocket, buf, BUF_SIZE, 0);
					if (bytesReceived == SOCKET_ERROR)
					{
						UE_LOG(LogTemp, Error, TEXT("Error in recv(). Quitting\n"));
						continue;
					}

					else if (bytesReceived == 0)
					{
						UE_LOG(LogTemp, Error, TEXT("Client disconnected\n"));
						continue;
					}

					FString ReceivedJsonString = ANSI_TO_TCHAR(buf);

					// Wait Until bIsFunctionRunning is false
					while(ThisThreadActor->bIsFunctionRunning)
					{
						// LOG
						UE_LOG(LogTemp, Log, TEXT("Waiting for Blueprint Function to Finish\n"));
						FPlatformProcess::Sleep(0.1f);
					}
					
					deserialJSON(ReceivedJsonString, ThisThreadActor->status);

					// CALL MAIN THREAD
					ThisThreadActor->spawnEachElevatorActor(ThisThreadActor->status.device_name, ThisThreadActor->status);
					
					// LOG Received goToFLoor
					UE_LOG(LogTemp, Log, TEXT("Received goToFLoor: %d"), ThisThreadActor->status.goToFloor);
					FPlatformProcess::Sleep(0.1f);
					
					const char* responseMessage;
					if(ThisThreadActor->status.building_name == "")
					{
						responseMessage = "BuildingDataNULL\n"; // Response message to send
					}
					else
					{
						responseMessage = "Received\n"; // Response message to send
					}
						
					int bytesSent = send(acceptSocket, responseMessage, strlen(responseMessage), 0);
					if (bytesSent == SOCKET_ERROR)
					{
						UE_LOG(LogTemp, Error, TEXT("Error sending response to server\n"));
						continue;
					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("Response sent to server: %s\n"), ANSI_TO_TCHAR(responseMessage));
					}
					closesocket(acceptSocket);
				}
				Mutex.Unlock();
			}
		}

		FPlatformProcess::Sleep(0.1f); // 0.1초 대기
	}
	closesocket(clientSocket);
	return 0;
}

// Called by Thread->Kill() in Destroyer
void FSocketThread::Stop()
{
	UE_LOG(LogNet, Warning, TEXT("Thread is being Stop"));
	bIsFinished = true;

	// Erase All Actors That has been spawned from this->myActor
	for (TActorIterator<AEachElevatorActor> ActorItr(ThisThreadActor->GetWorld()); ActorItr; ++ActorItr)
	{
		AEachElevatorActor* ThisActor = *ActorItr;
		ThisActor->Destroy();
	}
}

void FSocketThread::deserialJSON(const FString& ReceivedJSON, FStatus& stats)
{
	// Deserialize the JSON string 1. Make Reader
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ReceivedJSON);
	
	// Deserialize the JSON string 2. Make Object That Saves Deserialized JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// Deserialize the JSON string 3. Do Deserialization
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if(stats.underground_floor == 0)
		{
			stats.underground_floor = JsonObject->GetIntegerField(TEXT("underground_floor"));
			stats.ground_floor = JsonObject->GetIntegerField(TEXT("ground_floor"));
		}

		if(stats.acceleration == 0)
		{
			stats.acceleration = JsonObject->GetNumberField(TEXT("acceleration"));
			stats.max_velocity = JsonObject->GetNumberField(TEXT("max_velocity"));
		}
		
		stats.building_name = JsonObject->GetStringField(TEXT("building_name"));
		stats.device_name = JsonObject->GetStringField(TEXT("device_name"));
		
		stats.goToFloor = JsonObject->GetIntegerField(TEXT("goToFloor"));

		stats.tta = JsonObject->GetNumberField(TEXT("tta"));
		stats.ttm = JsonObject->GetNumberField(TEXT("ttm"));
		stats.ttd = JsonObject->GetNumberField(TEXT("ttd"));
		
		if(stats.each_floor_altimeter.IsEmpty())
		{
			TArray<TSharedPtr<FJsonValue>> temp = JsonObject->GetArrayField(TEXT("each_floor_altimeter"));
			for (const TSharedPtr<FJsonValue>& JsonValue : temp)
			{
				// Ensure the JsonValue is valid and its type is number
				if (JsonValue.IsValid() && JsonValue->Type == EJson::Number)
				{
					// Get the number value as a float and add it to the DecimalsArray
					float DecimalValue = JsonValue->AsNumber();
					stats.each_floor_altimeter.Add(DecimalValue);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON data"));
	}
}


void FSocketThread::Exit()
{
	FRunnable::Exit();
}