// Fill out your copyright notice in the Description page of Project Settings.


#include "oneM2MListenSever.h"

// Sets default values
AoneM2MListenSever::AoneM2MListenSever()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AoneM2MListenSever::BeginPlay()
{
	Super::BeginPlay();

	// Create WSADATA
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Create a Socket to ListenSocket
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// SET IP, PORT To Socket by inet_pton
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);

	// Bind the socket
	bind(ListenSocket, (SOCKADDR*)&service, sizeof(service));

	// Set Socket As AySync
	WSAAsyncSelect(ListenSocket, NULL, WM_USER, FD_ACCEPT);

	// Listen the Socket
	listen(ListenSocket, SOMAXCONN);
}

void AoneM2MListenSever::Destroyed()
{
	Super::Destroyed();
	
}

// Called every frame
// This Tick Function Will be Used to Receive Data from ListenSocket
void AoneM2MListenSever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if Socket is Connected To Client
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(ListenSocket, &readSet);

	// SELECT를 이용하여 소켓을 10초 동안 LISTEN 상태로 유지
	auto result = select(0, &readSet, NULL, NULL, NULL);

	if(result == SOCKET_ERROR)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR IN SELECT CREATION..."));
	}
	// JSON 수집 시 데이터 처리
	else
	{
		FCriticalSection Mutex;
				
		sockaddr_in clientSockInfo;
		int clientSize = sizeof(clientSockInfo);
				
		u_long BlockingMode = 0;
		ioctlsocket(ListenSocket, FIONBIO, &BlockingMode);
				
		AcceptSocket = accept(ListenSocket, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);

		if (AcceptSocket == INVALID_SOCKET)  //INVALID_SOCKET = 18446744073709551615
		{
			UE_LOG(LogTemp, Error, TEXT("accept failed: %d"), WSAGetLastError());
			closesocket(AcceptSocket);
			WSACleanup();
		}
				
		else
		{
			// While loop: 클라이언트의 메세지를 받아서 출력 후 클라이언트에 다시 보냅니다.
			enum eBufSize { BUF_SIZE = 4096 };
			char buf[BUF_SIZE];
			ZeroMemory(buf, BUF_SIZE);

			int bytesReceived = recv(AcceptSocket, buf, BUF_SIZE, 0);

			FString ReceivedJsonString = ANSI_TO_TCHAR(buf);
		}
	}
}

