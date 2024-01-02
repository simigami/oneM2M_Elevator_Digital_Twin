#pragma once
#include "socket_oneM2M.h"
#include "simulation.h"

class Elevator
{
public:
    socket_oneM2M sock;
	physics p;

    string building_name;
    string device_name;

    vector<vector<long double>> current_goTo_Floor_vector_info;
    vector<vector<string>> latest_RETRIEVE_info;
    vector<int> floor_info;

    long double Elevator_current_time, Elevator_current_velocity, Elevator_current_altimeter;
    int Elevator_opcode;
    int go_To_Floor;

    bool isRunning;

    Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
    ~Elevator();

    void RETRIEVE_from_oneM2M();
    void dev_print();

    void start_thread();
    void stop_thread();
    void run();

private:
    int RETRIEVE_interval_second;

    // oneM2M retrieve를 통해 값이 들어왔을 때, 파싱 후 physics나, simulator에 역할을 수행하게끔 하기
};