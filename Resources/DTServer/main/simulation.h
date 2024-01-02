#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

using namespace std;

class simulation
{

public:
	bool add_floor_to_main_trip_list(int floor, bool direction);
	bool add_floor_to_reserve_trip_list(int floor, bool direction);
	bool pop_floor_of_trip_list(vector<int> trip_list);
	vector<int> erase_floor_of_trip_list(vector<int> trip_list, int floor);

	bool update_main_trip_list_via_inside_data(vector<string> button_inside, bool direction);
	bool update_main_trip_list_via_outside_data(vector<vector<int>> button_outside, bool direction);

	bool modify_trip_list(vector<string> button_inside, bool direction);
	bool modify_trip_list(vector<vector<int>> button_outside, bool direction);

	bool is_main_trip_list_empty();
	bool check_reachability();

	const void dev_print_trip_list();

	int string_floor_to_int(const string& floor);

	vector<int> main_trip_list;
	vector<int> reserved_trip_list_up;
	vector<int> reserved_trip_list_down;

	vector<string> prev_button_inside_data;
	vector<vector<int>> prev_button_outside_data;
};

struct default_building_info
{
	int underground_floor;
	int ground_floor;

	float acceleration = 1.25;
	float max_velocity = 2.5;
	float ttr = 2.0;

	vector<double> altimeter_of_each_floor;
};

class physics
{
public:
	physics(int underground_floor, int ground_floor, vector<double> altimeter_of_each_floor);

	simulation s;
	default_building_info info;

	bool current_direction;
	long double current_velocity; 
	long double current_altimeter;

	bool set_initial_elevator_direction(vector<string> button_inside);
	bool set_initial_elevator_direction(vector<vector<int>> button_outside);

	long double timeToVelocity(long double initial_velocity, long double final_velocity, long double acceleration);
	long double distanceDuringAcceleration(long double initial_velocity, long double final_velocity, long double acceleration);

	vector<vector<long double>> draw_vt_on_sigle_floor(int floor);
};