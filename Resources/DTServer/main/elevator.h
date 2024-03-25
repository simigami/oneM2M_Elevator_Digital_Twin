#pragma once
#include "config.h"
#include "parse_json.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "elevatorAlgorithmSingle.h"
#include "elevatorAlgorithmMultiple.h"

class Elevator
{
public:
    //oneM2M에 CRUD를 하기 위해 사용하는 클래스
	socket_oneM2M* sock;

    //엘리베이터의 시뮬레이션을 수행하는 클래스
	physics* p;

    //UE5로 데이터를 전송하는 클래스
    socket_UnrealEngine* UEsock;

    elevatorAlgorithmSingle* thisElevatorAlgorithmSingle;
    elevatorAlgorithmMultiple* thisElevatorAlgorithmMultiple;

    Elevator(elevator_resource_status sc, Wparsed_struct parsed_struct, vector<wstring> ACP_NAMES, int algorithmNumber);

    void runElevator();
    void setNotificationContent(notificationContent* sendTo);
    void setUpdateElevatorTick(socket_UnrealEngine* ueSock, physics* phy);

    double getAltimeterFromFloor(int floor);

    wstring getBuildingName() const;
    wstring getDeviceName() const;

    int algorithmNumber;
};