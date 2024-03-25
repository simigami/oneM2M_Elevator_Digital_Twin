/*
#pragma once
#include "send_UnrealEngine.h"
#include "simulation.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"

struct Elevator_Status
{
	wstring building_name = L"";
    wstring device_name = L"";

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

struct notification_strings
{
	double velocity = 0.0;
	double altimeter = 0.0;

	vector<int> button_inside;

    int goTo = 0;

	bool button_outside_direction;
    int button_outside_floor = 0;
    double button_outside_altimeter = 0.0;

    int erased_button_outside_floor = 0;
};

class Elevator2
{
public:
    Elevator_Status* elevatorStatus;
    notification_strings* each_notification_string;

    socket_oneM2M* sock;
	physics* p;
    socket_UnrealEngine* UEsock;

    vector<vector<double>>* current_goTo_floor_vector_info;
    vector<double> current_goTo_Floor_single_info;
    vector<vector<double>>::iterator it;

    string notification_body;

    long double Elevator_current_time, Elevator_current_velocity, Elevator_current_altimeter;
    int Elevator_opcode;
    int go_To_Floor;

    bool isRunning;
    bool firstOperation;
    bool IDLEFlag;

    Elevator2(Wparsed_struct parsed_struct, vector<wstring> ACP_NAMES);

    void start_thread();
    void stop_thread();

	void run();
    void updateElevatorTick();

private:
    int RETRIEVE_interval_millisecond;
};*/