#pragma once
#include <boost/asio.hpp>
#include <ShlObj.h>
#include <commdlg.h>
#include "socket_oneM2M.h"
#include "parse_json.h"

using namespace std;

struct UE5Transaction {
    int goTo_floor;
    int timestamp;

    double tta;
    double ttm;
    double ttd;

    wstring owner;
};

struct transaction {
    int start_floor = 0;
    vector<int>* destination_floors = new vector<int>();
    wstring transaction_owner = L"";
    double timestamp = 0.0;
    double end_timestamp = 0.0;
};

class simElevator {

public:
    simElevator();

    wstring elevatorName;

    double current_velocity;
    double current_altimeter;
    double move_distance;

    vector<transaction> current_transaction;
    vector<transaction> previous_transactions;
};

class simBuilding {
public:
    simBuilding();

    vector<simElevator>* elevators = new vector<simElevator>{};
    vector<transaction>* transactions = new vector<transaction>{};
    vector<UE5Transaction>* timestamp_for_each_floor = new vector<UE5Transaction>();

    wstring buildingName;
    string json_string;
    
    int undergroundFloor;
    int abovegroundFloor;

    int default_start_floor;

    vector<double> each_floor_altimeter = vector<double>();

    double default_elevator_max_velocity;
    double default_elevator_max_acceleration;

    double default_elevator_stop_time;
};

class dt_simulation
{
public:

    OPENFILENAME ofn;
    socket_oneM2M* oneM2MSocket;
    wchar_t* fileLocation;

    // 1st dimension will become each Building Name, 2nd will be each Elevator Name
    //vector<vector<wstring>> thisTestCaseResourceInfo;
    vector<wstring> TCNames;
    map<wstring, map<wstring, int>> thisTestCaseResourceInfo;

    vector<simBuilding>* buildings = new vector<simBuilding>{};

    dt_simulation();

    void setSimulatedFileLocation();
    wchar_t getSimulatedFileLocation() const;

    void run();
    std::unordered_map<std::string, std::string> findLogStart(const std::string& fileAddress, std::unordered_map<std::string, std::string>& elevatorMap);
    std::vector<simBuilding>* createBuildings(const std::unordered_map<std::string, std::string>& elevatorMap);
    
    simBuilding getBuilding(const std::wstring& buildingName);
    transaction getPreviousTransaction(simBuilding this_building);

    void ReadAndAddAllTransactions();
    void PrintAllTransactions();
    void WriteAllTransactionsToFile();
    void AddTransactionOfThisElevator(simBuilding this_building, wifstream* file);
    void SimulationLogCALL(simBuilding this_building, std::wstring line);
    void SimulationLogMOV(simBuilding this_building, std::wstring line);
    void SimulationLogSTOP(simBuilding this_building, std::wstring line);
    void SimulationLogPRESS(simBuilding this_building, std::wstring line);
    void SimulationLogIDLE(simBuilding this_building, std::wstring line);
    void sortTransactions(simBuilding this_building);

    void runningthread();
    void send_data(std::string json_string);
    string set_elevator_Status_JSON_STRING(simBuilding this_building, UE5Transaction each_timestamp);
    void sendAllBuildingTransactions(simBuilding each_building);
    void giveAllBuildingTransactions();
    void giveElevatorTransaction(simBuilding this_building);
    void reallocateAllElevatorOfThisBuilding(simBuilding this_building, int timestamp);
    double getTimeBetweenTwoFloors(simBuilding this_building, int start_floor, int end_floor);
    simElevator* findIDLEElevator(vector<simElevator>* this_building_elevators, transaction this_transaction);
    simElevator* findNearestElevator(simBuilding this_building, vector<simElevator>* this_building_elevators, transaction this_transaction);

    void run2(); // PREVIOUSLY USED

    void readFileAndCreateoneM2MResource(wstring TCName);
    void putSimulatedStringTooneM2M(Wparsed_struct parsedStruct);
};