#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

using namespace std;

struct latest_RETRIEVE_STRUCT
{
    bool empty = true;

    double velocity;
    double altimeter;

	int und;
	int gnd;
	vector<double> each_floor_altimeter;

    vector<string> button_inside;
    vector<vector<int>> button_outside;
};

class simulation
{

public:
	void check_cin_and_modify_main_trip_list_between_previous_RETRIEVE(latest_RETRIEVE_STRUCT previous, latest_RETRIEVE_STRUCT current, bool direction);
	void swap_trip_list();
	const void clear_data();

	bool bCheckAllListEmpty();

	bool bigger(vector<int>& v1, vector<int>& v2);
	bool smaller(vector<int>& v1, vector<int>& v2);

	bool add_floor_to_main_trip_list(int floor, bool direction, bool inout);
	bool add_floor_to_reserve_trip_list(int floor, bool direction, bool inout);

	vector<vector<int>> pop_floor_of_trip_list(vector<vector<int>> trip_list);
	bool erase_floor_of_trip_list(vector<vector<int>> trip_list, int floor);

	bool update_main_trip_list_via_inside_data(vector<string> button_inside, bool direction);
	bool update_main_trip_list_via_inside_data2(vector<int> button_inside, bool direction);
;
	void check_and_set_trip_list(int req_floor, bool direction, bool req_direction, double current_altimeter, double req_altimeter);
	void check_and_set_trip_list_Nearest_N(
		int req_floor, 
		bool direction, bool req_direction, 
		double current_altimeter, double req_altimeter, double currentDestFloorAltimeter,
		double currentAltRatio, double intervalMAX);
	
	bool update_main_trip_list_via_outside_data(vector<vector<int>> button_outside, bool direction, double current_altimeter, vector<double> each_floor_altimeter, int und, int gnd);
	bool update_main_trip_list_via_outside_data2(bool current_direction, long double current_altimeter, int outside_floor, bool outside_direction, double outside_altimeter);
	bool update_main_trip_list_via_outside_data_Nearest_N(
		bool current_direction, long double current_altimeter, 
		int outside_floor, bool outside_direction, double outside_altimeter,
		double currentDestFloorAltimeter,
		double currentAltRatio, double intervalMAX);

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

	vector<string> prev_button_inside_data;
	vector<int> prev_button_inside_data2;
	vector<vector<int>> prev_button_outside_data;

	vector<double> each_floor_altimeter;
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

	simulation* s;
	default_building_info info;

	bool init =true;
	bool lock = true;
	bool current_direction;
	long double current_velocity; 
	long double current_altimeter;

	double t_to_max_velocity; 
	double t_constant_speed;
	double t_max_to_zero_deceleration;

	const void swap_direction();
	const void clear_data();

	bool set_initial_elevator_direction(vector<string> button_inside);
	bool set_initial_elevator_direction(vector<vector<int>> button_outside);
	bool set_initial_elevator_direction(int floor);
	bool set_direction(int floor);

	long double timeToVelocity(long double initial_velocity, long double final_velocity, long double acceleration);
	long double distanceDuringAcceleration(long double initial_velocity, long double final_velocity, long double acceleration);

	double altimeterToFloorRatio(double altimeter, vector<double> eachFloorAltimeter);
	double floorToAltimeter(int floor, vector<double> eachFloorAltimeter) const;

	vector<vector<double>>* draw_vt_on_single_floor(int floor);
	vector<vector<double>> getElevatorTrajectory(double current_altimeter, double current_velocity,
	                                             double dest_altimeter,
	                                             double acceleration, double maximum_velocity, bool direction);

private:
};