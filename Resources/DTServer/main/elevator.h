#pragma once
#include "send_UnrealEngine.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

class Elevator
{
public:
    socket_oneM2M sock;
	physics p;
    socket_UnrealEngine UEsock;

    latest_RETRIEVE_STRUCT latest;

    string building_name;
    string device_name;

    vector<vector<long double>> current_goTo_Floor_vector_info;
    vector<long double> current_goTo_Floor_single_info;
    vector<vector<long double>>::iterator it;

    vector<vector<int>> latest_trip_list_info;
    vector<int> floor_info;

    long double Elevator_current_time, Elevator_current_velocity, Elevator_current_altimeter;
    int Elevator_opcode;
    int go_To_Floor;

    bool isRunning;

    Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
    ~Elevator();

    void dev_print();

    void update_latest_info(int floor, int inout, simulation* s);
    void start_thread();
    void stop_thread();
    void run();

private:
    int RETRIEVE_interval_millisecond;

    UE_Info wrap_for_UE_socket(string building_name, string device_name, int underground_floor, int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity);
    vector<vector<string>> RETRIEVE_from_oneM2M();
    latest_RETRIEVE_STRUCT parse_oneM2M_RETRIEVE_to_STRUCT(vector<vector<string>> ret);

    // oneM2M retrieve를 통해 값이 들어왔을 때, 파싱 후 physics나, simulator에 역할을 수행하게끔 하기
};