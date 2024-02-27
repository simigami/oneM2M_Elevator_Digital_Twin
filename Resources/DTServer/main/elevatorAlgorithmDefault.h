#pragma once
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

struct elevatorStatus
{
    bool direction;

    double velocity = 0;
    double altimeter = -55;

    const double max_velocity = 2.5;
    const double acceleration = 1.25;
    const double time_to_reach_max = 2.0;

    int goTo = 0;

    vector<int>* button_inside = new vector<int>;
    vector<vector<int>>* button_outside = new vector<vector<int>>;
};

struct notificationContent
{
	double velocity = 0.0;
	double altimeter = 0.0;

	vector<int> button_inside_list;

    int goTo = 0;

	bool added_button_outside_direction;
    int added_button_outside_floor = 0;
    double added_button_outside_altimeter = 0.0;

    int erased_button_outside_floor = 0;
};

struct flags
{
    //각 반복문을 돌기 위해 사용되는 플래그
	bool isRunning = true;

    //처리 받는 입력이 맨 처음의 입력인지 확인하는 플래그
    bool firstOperation = true;

    //엘리베이터가 IDLE상태로 전환된 상태를 나타내는 플래그
    bool IDLEFlag = true;
};

class elevatorAlgorithmDefault
{
public:
    //NOTIFICATION으로 받은 정보를 저장하는 구조체
    notificationContent* thisNotificationContent;

    //엘리베이터의 매 순간 상태 정보들을 모은 구조체
    elevatorStatus* thisElevatorStatus;

    //알고리즘에 사용되는 flag들을 모은 구조체
    flags* thisFlags;

    vector<vector<double>>* current_goTo_Floor_vector_info;
    vector<double> current_goTo_Floor_single_info;
    vector<vector<double>>::iterator it;

    string notification_body;

    string building_name;
    string device_name;

    long double Elevator_current_time, Elevator_current_velocity, Elevator_current_altimeter;
    int go_To_Floor;

    elevatorAlgorithmDefault(string buildingName, string deviceName);
    virtual ~elevatorAlgorithmDefault();

    virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy);
    virtual void stopThread();
	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* p);
	virtual void rearrangeVector(socket_UnrealEngine* ueSock, physics* p);
    virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy);

    elevatorStatus* getElevatorStatus();
    flags* getElevatorFlag();

private:
    //각 Loop를 돌 간격
    int RETRIEVE_interval_millisecond;
};