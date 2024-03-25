#pragma once
#include "config.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

class elevatorStatus
{
public:
    double wake_up_time = 0.0;
    double velocity = 0.0;
    double altimeter = 0.0;

    int go_to_floor = 0;
    bool direction = NULL;
    double total_move_distance = 0.0;

    double tta = 0.0;
    double ttm = 0.0;
    double ttd = 0.0;

    vector<vector<double>>* current_goTo_floor_vector_info;
    vector<double> current_goTo_Floor_single_info;

    vector<int>* button_inside = new vector<int>;
    vector<vector<int>>* button_outside = new vector<vector<int>>;

    void set_names(wstring buildingName, wstring deviceName);
    void set_physical_information(physics* p);

    wstring get_building_name();
    wstring get_device_name();
    int get_underground_floor();
    int get_ground_floor();
    vector<double> get_each_floor_altimeter();
    double get_max_velocity();
    double get_acceleration();

private:
    wstring building_name = L"";
    wstring device_name = L"";

    int underground_floor = 0;
    int ground_floor = 0;

    vector<double> each_floor_altimeter;

    double max_velocity = 0.0;
    double acceleration = 0.0;
};

struct notificationContent
{
	double current_velocity = 0.0;
	double current_altimeter = 0.0;

    vector<int> button_inside = vector<int>{};

	bool button_outside_direction;
    int button_outside_floor = 0;
    double button_outside_altimeter = 0.0;
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

    std::chrono::system_clock::time_point worldClock;
    std::chrono::system_clock::time_point someClock;

    vector<vector<double>>::iterator it;
    vector<wstring> log_list;

    elevatorAlgorithmDefault(wstring buildingName, wstring deviceName);
    virtual ~elevatorAlgorithmDefault();

    virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy);
    virtual void stopThread();
	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* p);
    virtual void stop(physics* p);

    virtual void writeLog();
    virtual void appendLogToLogList(int code, ...);
    virtual void IDLELog(va_list args);
    virtual void CALLLog(va_list args);
    virtual void PRESSLog(va_list args);
    virtual void UNPRESSLog(va_list args);
    virtual void STOPLog(va_list args);
    virtual void MOVLog(va_list args);

    virtual void set_physical_information(physics* p);
	virtual void rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* p);

    virtual void set_elevator_Status_JSON_STRING();
 
	virtual void printTimeDeltaWhenRearrange();
    virtual int printTimeDeltaNow();
	virtual void printTimeDeltaWhenStop();
    virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy);

    virtual string wstringToString(const std::wstring& wstr) const;
    virtual wstring stringToWstring(const std::string& str) const;

    elevatorStatus* getElevatorStatus();
    flags* getElevatorFlag();

protected:
    //elevatorStatus의 값들을 UE5로 전송할 JSON STRING
    string elevatorStatus_JSON_STRING;

    //각 Loop를 돌 간격
    int RETRIEVE_interval_millisecond;
};