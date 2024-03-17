#pragma once

//THIS IS FOR DT_SERVER information
#define DT_SERVER_ADDR L"192.168.0.134"
#define EMBEDDED_LISTEN_PORT 10050
#define oneM2M_NOTIFICATION_LISTEN_PORT 10053

#define HTTP
#define oneM2M_tinyIoT
#define UE5

#define oneM2M_CSE_Server L"192.168.0.178"
#define oneM2M_CSE_Port L 10051

// DT SERVER NEEDS WHEN IT USES HTTP PROTOCOL TO oneM2M CSE SERVER
#ifdef HTTP
	#define PROTOCOL_NAME L"http://"
	#define CSE_ADDR L"192.168.0.178"
	#define CSE_LISTEN_PORT 10051
	#define BUFFER_SIZE 1024
#endif // HTTP

// THIS IS  DEFAULT oneM2M ATTRIBUTES WHEN CREATE oneM2M RESOURCES
#define ACOP_ALL 63
#define MNI 128
#define MBS 1024

// DEFAULT oneM2M PRIMITIVE WHEN IT USES tinyIoT as oneM2M CSE SERVER
#ifdef oneM2M_tinyIoT
	#define DEFAULT_ACP_NAME L"DT_SERVER"
	#define DEFAULT_ORIGINATOR_NAME L"CAdmin"
	#define DEFAULT_CSE_NAME L"tinyIoT"
	#define DEFAULT_RI L"DT_RI"
	#define DEFAULT_RVI L"2a"
#endif

// DEFAULT oneM2M PRIMITIVE WHEN IT USES ACME as oneM2M CSE SERVER
#ifndef oneM2M_tinyIoT
	#define oneM2M_ACME
	#define DEFAULT_ACP_NAME L"DT_SERVER"
	#define DEFAULT_ORIGINATOR_NAME L"CAdmin"
	#define DEFAULT_CSE_NAME L"ACME"
	#define DEFAULT_RI "DT_RI"
	#define DEFAULT_RVI "4"
#endif

// THIS IS FOR UE5 SERVER INFORMATION
#ifdef UE5
	#define UE5_SERVER_ADDR "127.0.0.1"
	#define UE5_SERVER_LISTEN_PORT 10052
#endif // UE5

// THIS IS FOR DT SERVER RTS MODE WHEN SIMULATION ELEVATORS
#define SECOND 1000.0
#define TICK 100.0

// THIS IS FOR DT SERVER SIMULATION MODE
#define MAXFILEPATHLENGTH 1024
#define _WIN32_WINNT_WIN10 0x0A00

enum elevator_resource_status
{
	//if there is no ACP in oneM2M CSE SERVER
	ACP_NOT_FOUND, 

	//if there is ACP in oneM2M CSE SERVER but not AE(building)
	BUILDING_NOT_FOUND,

	//if there is AE(building) but not CNT(elevator) in oneM2M CSE SERVER
	ELEVATOR_NOT_FOUND, 

	//if there is AE(building) and CNT(elevator) in oneM2M CSE SERVER but AE(building) not in DT Server
	BUILDING_DATA_NOT_FOUND,

	//if there is AE(building) and CNT(elevator) in oneM2M CSE SERVER but CNT(Elevator) not in DT Server
	ELEVATOR_DATA_NOT_FOUND,

	//if there is AE, and CNT in oneM2M CSE SERVER and AE, CNT in DT Server
	ELEVATOR_FOUND
};