#pragma once
#include "config.h"
#include "logger.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

enum ElevatorStatusEnum
{
    RTS_SPAWN = 0,
    RTS_GO_TO_CHANGE,
    RTS_STOP,
    RTS_REARRANGE,
    RTS_IDLE,
};

struct elevatorTypeInfo
{
    elevator_type this_elevator_type = CounterBalance;
    double type_load_rate = 0.5;
};

class elevatorStatus
{
public:
    elevatorTypeInfo this_elevator_type_info;

    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point measure_time;

    int dilation = 1;

    bool isElevatorStopped = false;

    double wake_up_time = 0.0;
    double velocity = 0.0;
    double altimeter = 0.0;

    int go_to_floor = 0;
    int CurrentFloor = 0;
    bool direction = NULL;

    double total_move_distance = 0.0;
    int number_of_trips = 0;

    double tta = 0.0;
    double ttm = 0.0;
    double ttd = 0.0;
    double door_open_time = 4.0;

    double erd = 0.0;
    double esd = 0.0;
    double ed = 0.0;

    vector<vector<double>>* current_goTo_floor_vector_info;
    vector<double> current_goTo_Floor_single_info;

    vector<int>* button_inside = new vector<int>;
    vector<vector<int>>* button_outside = new vector<vector<int>>;

    void set_names(wstring buildingName, wstring deviceName);
    void set_physical_information(physics* p);

    void set_building_name(wstring buildingName);
    void set_device_name(wstring deviceName);
    wstring get_building_name();
    wstring get_device_name();

    void set_underground_floor(int floor);
    void set_ground_floor(int floor);
    void set_each_floor_altimeter(vector<double> alt);
    int get_underground_floor();
    int get_ground_floor();
    vector<double> get_each_floor_altimeter();

    void set_max_velocity(double velocity);
    void set_acceleration(double acceleration);
    void set_jerk(double jerk);
    double get_max_velocity();
    double get_acceleration();
    double get_jerk();

    void set_this_elevator_energy_flag(bool flag);
    bool get_this_elevator_energy_consumption();

    void set_this_elevator_daily_energy_consumption(int sim_mode_delta);
    vector<double> get_this_elevator_daily_energy_consumption();

    void setIDLEPower(double power);
    void setStandbyPower(double power);
    void setISOReferenceCycleEnergy(double power);
    double getIDLEPower();
    double getStandbyPower();
    double getISOReferenceCycleEnergy();
 
private:
    bool calculate_this_elevator_energy_consumption = false;

    wstring building_name = L"";
    wstring device_name = L"";

    int underground_floor = 0;
    int ground_floor = 0;

    vector<double> each_floor_altimeter;

    // Weight of the elevator
    double rate_load = 1350.0; 

    double load_factor = 0.0;

    double max_velocity = 0.0;

    double acceleration = 0.0;

    double jerk = 1.0;

    double IDLE_Power = 500.0; // W
    double Standby_Power_5Min = 120.0; // W
    double ISO_Reference_Cycle_Energy = 170.0; // Wh

    int Category = 0;
    double S_factor = 0.0;

    double IDLE_time_ratio = 0.0;
    double StandBy_time_ratio = 0.0;
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
	bool isRunning = false;

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

    logger thisLogger;

    std::chrono::system_clock::time_point worldClock;
    std::chrono::system_clock::time_point someClock;

    vector<vector<double>>::iterator it;
    vector<wstring> log_list;

    elevatorAlgorithmDefault(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time);
    virtual ~elevatorAlgorithmDefault();

    virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy);
    virtual void stopThread();
	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* p);
    virtual void stop(physics* p, flags* this_flag);

    virtual void write_logs(std::vector<std::wstring> strings);
    virtual void write_log(std::wstring string);
    virtual void write_csv_header();
    virtual wstring make_csv_string(ElevatorStatusEnum en);
    virtual void write_csv_body(std::wstring str);

    virtual void writeEnergyLog();
    void appendLogToLogList(int code, ...);
    virtual void IDLELog(va_list args);
    virtual void CALLLog(va_list args);
    virtual void PRESSLog(va_list args);
    virtual void UNPRESSLog(va_list args);
    virtual void STOPLog(va_list args);
    virtual void MOVLog(va_list args);

    virtual bool checkReachability(elevatorStatus* stats, double current_altimeter, int dest_floor);

    virtual vector<int> getNewButtonInsideList(vector<int> prev, vector<int> current);
    virtual vector<int> getRemoveButtonInsideList(vector<int> prev, vector<int> current);

    virtual void set_physical_information(physics* p);
	virtual void rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* p);
	virtual void rearrangeMainTripList(elevatorStatus* stats, physics* p);

    virtual void set_elevator_Status_JSON_STRING();
 
	virtual void printThisElevatorEnergyConsumptionInfos();
	virtual void printTimeDeltaWhenSpawn();
	virtual void printTimeDeltaWhenRearrange();
    virtual int printTimeDeltaNow();
	virtual void printTimeDeltaWhenStop();
    virtual void printTimeDeltaWhenIDLE();

    virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy);
    virtual void insideLogic(elevatorStatus* status, physics* phy, vector<int> new_elem, vector<int> remove_elem);
    virtual void outsideLogic(elevatorStatus* status, physics* phy, int outside_button, bool outside_direction);

    virtual void updateMainTripList(elevatorStatus* status, physics* phy, int elem);

    virtual string wstringToString(const std::wstring& wstr) const;
    virtual wstring stringToWstring(const std::string& str) const;

    double getAltimeterDifferenceBetweenTwoFloors(vector<double> each_floor_alt, int underground, int floor1, double alt2);

    elevatorStatus* getElevatorStatus();
    flags* getElevatorFlag();

    virtual string getJSONString();

protected:
    //elevatorStatus의 값들을 UE5로 전송할 JSON STRING
    string elevatorStatus_JSON_STRING;

    //각 Loop를 돌 간격
    int RETRIEVE_interval_millisecond;

    std::mutex mtx;
};