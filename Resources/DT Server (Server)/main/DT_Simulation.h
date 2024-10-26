#pragma once
#include "FileSystem.h"
#include "socket_oneM2M.h"
#include "elevatorAlgorithmDefault.h"
#include "parse_json.h"
#include <boost/asio.hpp>
#include <set>

using namespace std;
class logger;

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
    set<int>* inside_floors;
    wstring transaction_owner = L"";
    double timestamp = 0.0;
    double end_timestamp = 0.0;

    bool* closed = new bool();
    bool* idle_with_zero_trip = new bool();
};

class simElevator {

public:
    simElevator(wstring buliding_name, wstring device_name);
    elevatorAlgorithmDefault* this_elevator_algorithm;

    wstring elevatorName;

	bool init = true;

    int undergroundFloor;
    int abovegroundFloor;

    double current_velocity;
    double current_altimeter;
    double move_distance;

    vector<vector<double>> energy_consumption_vector = {};

    int latest_floor = 0;
    int will_reach_floor = 0;

    int init_floor = -5;

    vector<double> each_floor_altimeter = vector<double>();
    vector<int> current_called_floors;

    vector<transaction> current_transaction;
    vector<transaction> previous_transactions;
};

class simBuilding{
public:
    simBuilding();

    logger* thisLogger = new logger;

    vector<simElevator>* elevators = new vector<simElevator>{};
    vector<transaction>* transactions = new vector<transaction>{};
    vector<UE5Transaction>* timestamp_for_each_floor = new vector<UE5Transaction>();

    wstring buildingName;
    string json_string;

    int default_start_floor;

    double default_elevator_max_velocity;
    double default_elevator_max_acceleration;

    double default_elevator_stop_time;
};

class dt_simulation
{
public:

    FileSystem fileSystem;

    socket_oneM2M* oneM2MSocket;
    wchar_t* fileLocation;

    // 1st dimension will become each Building Name, 2nd will be each Elevator Name
    //vector<vector<wstring>> thisTestCaseResourceInfo;
    vector<wstring> TCNames;
    map<wstring, map<wstring, int>> thisTestCaseResourceInfo;

    vector<simBuilding>* buildings = new vector<simBuilding>{};

    dt_simulation();

    void run();
    
    simBuilding getBuilding(const std::wstring& buildingName);

    void ReadAndAddAllTransactions();
    void PrintAllTransactions();
    void WriteAllTransactionsToFile();
    std::vector<simBuilding>* makeInstance(const std::string& fileAddress);
    void AddTransactionOfThisElevator(simBuilding this_building, wifstream* file);
    void SimulationLogCALL(simBuilding this_building, std::wstring line);
    void SimulationLogMOV(simBuilding this_building, std::wstring line);
    void SimulationLogSTOP(simBuilding this_building, std::wstring line);
    void SimulationLogPRESS(simBuilding this_building, std::wstring line);
    void SimulationLogIDLE(simBuilding this_building, std::wstring line);
    void sortTransactions(simBuilding this_building);

    void runSimulation();
    void calculateEnergyConsumption(simBuilding* thisBuilding, UE5Transaction each_transaction);
    void send_data(std::string json_string);
    string set_elevator_Status_JSON_STRING(simBuilding this_building, simElevator ThisElevator, UE5Transaction each_timestamp);
    void sendAllBuildingTransactions(simBuilding* each_building, std::mutex* this_mutex, int dilation, int visualtion_mode);
    void giveAllBuildingTransactions();
    void giveElevatorTransaction(simBuilding this_building);

    void reallocateAllElevatorOfThisBuilding(simBuilding this_building, int timestamp);
    const int setSimulationAlgorithm(simBuilding this_building);
    void SimulationWithAlgorithm(const int alg_num, simBuilding* this_building, transaction tran);
    void SimulationAlgorithmDefault(simBuilding* this_building, transaction tran);
    void SimulationAlgorithmRandom(simBuilding* this_building, transaction tran);
    void SimulationAlgorithmShortestTransactionFirst(simBuilding* this_building, transaction tran);

    double getTimeBetweenTwoFloors(simBuilding this_building, simElevator ThisElevator, int start_floor, int end_floor);
    double getDisatanceBetweenTwoFloors(simBuilding this_building, simElevator ThisElevator, int start_floor, int end_floor);
    simElevator* findIDLEElevator(vector<simElevator>* this_building_elevators, transaction this_transaction);
    simElevator* findNearestElevator(simElevator ThisElevator, vector<simElevator>* this_building_elevators, transaction this_transaction);

    wstring MakeCSVStringByGatherElevatorInfo(simElevator ThisElevator, const int Timestamp);

private:
    string log_file_path;
};