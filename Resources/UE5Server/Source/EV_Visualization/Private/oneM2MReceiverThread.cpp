// Fill out your copyright notice in the Description page of Project Settings.

#include "oneM2MReceiverThread.h"
#include "DataDecisionActor.h"
#include "EngineUtils.h"
#include <Windows.h>
#include <Ws2tcpip.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS

FoneM2MReceiverThread::FoneM2MReceiverThread(UWorld* World) : thisWorld(World)
{
	Thread = FRunnableThread::Create(this, TEXT("TEST"));
}

FoneM2MReceiverThread::~FoneM2MReceiverThread()
{
	if (clientSocket != INVALID_SOCKET)
	{
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}

	if (acceptSocket != INVALID_SOCKET)
	{
		closesocket(acceptSocket);
		acceptSocket = INVALID_SOCKET;
	}
	
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

bool FoneM2MReceiverThread::Init()
{
	bIsFinished = false;
	UE_LOG(LogNet, Warning, TEXT("Thread has been initialized"));
	return true;
}

uint32 FoneM2MReceiverThread::Run()
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
			struct timeval timeout;
			timeout.tv_sec = 10; // 10 seconds
			timeout.tv_usec = 0; // No microseconds
			
			result = select(0, &readSet, NULL, NULL, &timeout);

			if(result == SOCKET_ERROR)
			{
				UE_LOG(LogTemp, Error, TEXT("ERROR IN SELECT CREATION..."));
				break;
			}
			else if(result == 0)
			{
				UE_LOG(LogTemp, Log, TEXT("No connection request received"));
				FPlatformProcess::Sleep(0.1f); // 0.1초 대기
				break;
			}
			
			// JSON 수집 시 데이터 처리
			else
			{
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
					FCriticalSection Mutex;
					FScopeLock ScopeLock(&Mutex);
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
					data = deserialJSON(ReceivedJsonString);
					data.map_path = FPaths::Combine(FPaths::ProjectContentDir(), "EV_VIS/Maps/", data.building_name + ".umap");

					UE_LOG(LogTemp, Error, TEXT("Receive Data From Building : %s > Elevator : %s\n"), *data.building_name, *data.device_name);
					
					const char* responseMessage;
					if(data.building_name == "")
					{
						responseMessage = "BuildingDataNULL\n"; // Response message to send
					}
					else
					{
						responseMessage = "Received\n"; // Response message to send
					}

					//Find ADataDecisionActor Based on buliding_name using thisWorld, and Subclass
					auto SetDecisionStructLambda = [](UWorld* World, FDecisionStruct& Data)
					{
						for (TActorIterator<ADataDecisionActor> ActorItr(World); ActorItr; ++ActorItr)
						{
							if (ActorItr->building_name == Data.building_name + "_" + Data.device_name)
							{
								// Set Decision Struct
								ActorItr->setDecisionStruct(Data);
								break;
							}
						}
					};
					
					AsyncTask(ENamedThreads::GameThread, [this, SetDecisionStructLambda]()
					{
						SetDecisionStructLambda(this->thisWorld, this->data);
					});

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
					FPlatformProcess::Sleep(0.1f); // 0.1초 대기
				}
			}
		}
		FPlatformProcess::Sleep(0.1f); // 0.1초 대기
	}
	
	closesocket(clientSocket);
	return 0;
}

FDecisionStruct FoneM2MReceiverThread::deserialJSON(const FString& ReceivedJSON)
{
	FDecisionStruct newStruct;
	
	// Deserialize the JSON string 1. Make Reader
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ReceivedJSON);
	
	// Deserialize the JSON string 2. Make Object That Saves Deserialized JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// Deserialize the JSON string 3. Do Deserialization
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		newStruct.building_name = JsonObject->GetStringField(TEXT("building_name"));
		newStruct.device_name = JsonObject->GetStringField(TEXT("device_name"));

		// Set Map Path using content dir + building name + device name + umap
		newStruct.map_path = FPaths::Combine(FPaths::ProjectContentDir(), newStruct.building_name, "EV_VIS/Maps/", newStruct.building_name + ".umap");

		newStruct.goToFloor = JsonObject->GetIntegerField(TEXT("goToFloor"));

		newStruct.erd = JsonObject->GetNumberField(TEXT("Erd"));
		newStruct.esd = JsonObject->GetNumberField(TEXT("Esd"));
		newStruct.ed = JsonObject->GetNumberField(TEXT("Ed"));

		newStruct.tta = JsonObject->GetNumberField(TEXT("tta"));
		newStruct.ttm = JsonObject->GetNumberField(TEXT("ttm"));
		newStruct.ttd = JsonObject->GetNumberField(TEXT("ttd"));
		
		newStruct.underground_floor = JsonObject->GetIntegerField(TEXT("underground_floor"));
		newStruct.ground_floor = JsonObject->GetIntegerField(TEXT("ground_floor"));
		
		newStruct.acceleration = JsonObject->GetNumberField(TEXT("acceleration"));
		newStruct.max_velocity = JsonObject->GetNumberField(TEXT("max_velocity"));
		
		if(newStruct.each_floor_altimeter.IsEmpty())
		{
			TArray<TSharedPtr<FJsonValue>> temp = JsonObject->GetArrayField(TEXT("each_floor_altimeter"));
			for (const TSharedPtr<FJsonValue>& JsonValue : temp)
			{
				// Ensure the JsonValue is valid and its type is number
				if (JsonValue.IsValid() && JsonValue->Type == EJson::Number)
				{
					// Get the number value as a float and add it to the DecimalsArray
					float DecimalValue = JsonValue->AsNumber();
					newStruct.each_floor_altimeter.Add(DecimalValue);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON data"));
	}

	return newStruct;
}

void FoneM2MReceiverThread::Stop()
{
	FRunnable::Stop();

	if(!bIsFinished)
	{
		closesocket(acceptSocket);
		closesocket(clientSocket);
		bIsFinished = true;
	}
}

void FoneM2MReceiverThread::Exit()
{
	FRunnable::Exit();
}
