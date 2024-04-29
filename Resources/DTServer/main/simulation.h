#pragma once
#include "parse_json.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

struct default_building_info
{
	int underground_floor;
	int ground_floor;

	float acceleration = 1.25;
	float max_velocity = 2.5;

	vector<double> altimeter_of_each_floor;
};

class simulation
{

public:
	void clear_data();

	bool bCheckAllListEmpty();

	bool add_floor_to_main_trip_list(int floor, bool direction, bool inout);
	bool add_floor_to_reserve_trip_list(int floor, bool direction, bool inout);

	bool modify_trip_list(vector<string> button_inside, bool direction);
	bool modify_trip_list(vector<vector<int>> button_outside, bool direction);

	bool is_main_trip_list_empty();
	bool check_reachability();

	const void dev_print_trip_list();
	const void dev_print_stopped_floor();

	int string_floor_to_int(const string& floor);
	string int_floor_to_string(const int& floor);

	vector<vector<int>> main_trip_list;
	vector<vector<int>> reserved_trip_list_up;
	vector<vector<int>> reserved_trip_list_down;

	vector<double> each_floor_altimeter;
};

class physics
{
public:
	physics(Wparsed_struct parsed_struct);

	simulation* s;
	default_building_info info;

	bool current_direction;
	double current_velocity; 
	double current_altimeter;

	double t_to_max_velocity; 
	double t_constant_speed;
	double t_max_to_zero_deceleration;

	bool set_initial_elevator_direction(vector<string> button_inside);
	bool set_initial_elevator_direction(vector<vector<int>> button_outside);

	bool set_initial_elevator_direction(int floor);
	bool set_direction(int floor);

	long double timeToVelocity(long double initial_velocity, long double final_velocity, long double acceleration);

	double floorToAltimeter(int floor, vector<double> eachFloorAltimeter) const;

	vector<vector<double>>* draw_vt_on_single_floor(int floor);

	vector<vector<double>> getElevatorTrajectory(
		double current_altimeter, double current_velocity,
		double dest_altimeter,
		double acceleration, double maximum_velocity, bool direction
	);

private:
};