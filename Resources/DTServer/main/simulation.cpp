#include "simulation.h"
//#include "matplotlibcpp.h"
#include <algorithm>

using namespace std;
//using namespace matplotlibcpp;

bool simulation::add_floor_to_main_trip_list(int floor, bool direction)
{
	try
	{
		if(direction)
		{
			this->main_trip_list.push_back(floor);
			sort(this->main_trip_list.begin(), this->main_trip_list.end());
		}
		else
		{
			this->main_trip_list.push_back(floor);
			sort(this->main_trip_list.begin(), this->main_trip_list.end(), greater<int>());
		}
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

// THIS DIRECTION IS DIRECTION OF CALLED FLOOR, NOT ELEVATOR ITSELF
bool simulation::add_floor_to_reserve_trip_list(int floor, bool direction)
{
	try
	{
		if(direction)
		{
			this->reserved_trip_list_up.push_back(floor);
			sort(this->reserved_trip_list_up.begin(), this->reserved_trip_list_up.end());
		}
		else
		{
			this->reserved_trip_list_down.push_back(floor);
			sort(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end(), greater<int>());
		}
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

bool simulation::pop_floor_of_trip_list(vector<int> trip_list)
{
	try
	{
		trip_list.erase(trip_list.begin());
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

vector<int> simulation::erase_floor_of_trip_list(vector<int> trip_list, int floor)
{
	try
	{
		trip_list.erase(remove(trip_list.begin(), trip_list.end(), floor), trip_list.end());
		return trip_list;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		exit(0);
	}
}

bool simulation::update_main_trip_list_via_inside_data(vector<string> button_inside, bool direction)
{
	try
	{
		int floor;
		bool flag;

		vector<string> prev = this->prev_button_inside_data;
		vector<string> Currently_Erased(button_inside.size() + prev.size());
		vector<string> Newly_Added(button_inside.size() + prev.size());

		vector<string>::iterator iter_prev;
		vector<string>::iterator iter_next;

		sort(button_inside.begin(), button_inside.end());
		sort(prev.begin(), prev.end());

		iter_prev = set_difference(prev.begin(), prev.end(), button_inside.begin(), button_inside.end(), Currently_Erased.begin());
		iter_next = set_difference(button_inside.begin(), button_inside.end(), prev.begin(), prev.end(), Newly_Added.begin());

		Currently_Erased.resize(iter_prev-Currently_Erased.begin());
		Newly_Added.resize(iter_next-Newly_Added.begin());

		cout << endl << "Newly Added Floor" << endl;
		for(const auto& elem : Newly_Added)
		{
			floor = this->string_floor_to_int(elem);
			flag = check_reachability();
			if(flag)
			{
				add_floor_to_main_trip_list(floor, direction);
				cout << "INSIDE FLOOR : " << floor << " IS REACHABLE, ADDING ON MAIN LIST..." << endl;
			}
			//else
			//{
			//	cout << "FLOOR : " << floor << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << endl;
			//	add_floor_to_reserve_trip_list(floor, direction);
			//}
		}

		cout << endl << "Currently Erased Floor" << endl;
		for(const auto& elem : Currently_Erased)
		{
			cout << " " << elem;
			floor = this->string_floor_to_int(elem);
			this->main_trip_list = erase_floor_of_trip_list(this->main_trip_list, floor);
			cout << "INSIDE FLOOR : " << floor << " IS DELETED, ERASE ON MAIN LIST..." << endl;
		}
		this->prev_button_inside_data = button_inside;

		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

bool simulation::update_main_trip_list_via_outside_data(vector<vector<int>> button_outside, bool direction)
{
	try
	{
		vector<vector<int>> prev = this->prev_button_outside_data;
		vector<vector<int>> Currently_Erased(button_outside.size() + prev.size());
		vector<vector<int>> Newly_Added(button_outside.size() + prev.size());

		vector<vector<int>>::iterator iter_prev;
		vector<vector<int>>::iterator iter_next;

		int req_floor;
		bool req_direction;
		bool flag;

		sort(button_outside.begin(), button_outside.end());
		sort(prev.begin(), prev.end());

		iter_prev = set_difference(prev.begin(), prev.end(), button_outside.begin(), button_outside.end(), Currently_Erased.begin());
		iter_next = set_difference(button_outside.begin(), button_outside.end(), prev.begin(), prev.end(), Newly_Added.begin());

		Currently_Erased.resize(iter_prev-Currently_Erased.begin());
		Newly_Added.resize(iter_next-Newly_Added.begin());

		cout << endl << "Newly Added Floor : ";
		for(const auto& elem : Newly_Added)
		{
			req_floor = elem[0];
			req_direction = elem[1];
			flag = check_reachability();
			if(flag)
			{
				if(direction == req_direction)
				{
					add_floor_to_main_trip_list(req_floor, direction);
					cout << "OUTSIDE FLOOR : " << req_floor << " IS REACHABLE WITH SAME DIRECTION, ADDING ON MAIN LIST..." << endl;
				}
				else
				{
					add_floor_to_reserve_trip_list(req_floor, req_direction);
					cout << "OUTSIDE FLOOR : " << req_floor << " DIFFERENT DIRECTION, ADDING ON RESERVED LIST..." << endl;
				}
			}
			else
			{
				cout << "OUTSIDE FLOOR : " << req_floor << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << endl;
				add_floor_to_reserve_trip_list(req_floor, req_direction);
			}
			cout << endl;
		}
		// ASSUME THAT OUTSIDE FLOOR BUTTON CANNOT BE CANCELED
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

// direction = Elevator's Current Moving Direction -> True : Up, False : Down
bool simulation::modify_trip_list(vector<string> button_inside, bool direction)
{
	bool flag;
	int size = button_inside.size();
	vector<int> button_inside_int;

	for(const auto& elem : button_inside)
	{
		button_inside_int.push_back(string_floor_to_int(elem));
	}
	try
	{
		if(direction)
		{
			for(int index = 0; index < size; index++)
			{
				if(this->main_trip_list[index] != button_inside_int[index])
				{
					//CHECK IF NEW TRIP IS REACHABLE
					flag = check_reachability();
					if(flag)
					{
						add_floor_to_main_trip_list(button_inside_int[index], direction);
						cout << "FLOOR : " << button_inside_int[index] << " IS MISSING, ADDING ON MAIN LIST..." << endl;

						return true;
					}
					else
					{
						cout << "FLOOR : " << button_inside_int[index] << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << endl;
						add_floor_to_reserve_trip_list(button_inside_int[index], direction);
					}
				}
			}
			cout << "FLOOR LIST IS SAME..."<< endl;
			return true;
		}
		else
		{
			for(int index = 0; index < size; index++)
			{
				if(this->main_trip_list[size-1-index] != button_inside_int[index])
				{
					//CHECK IF NEW TRIP IS REACHABLE
					flag = check_reachability();
					if(flag)
					{
						add_floor_to_main_trip_list(button_inside_int[index], direction);
						cout << "FLOOR : " << button_inside_int[index] << " IS MISSING, ADDING ON MAIN LIST..." << endl;

						return true;
					}
					else
					{
						cout << "FLOOR : " << button_inside_int[index] << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << endl;
						add_floor_to_reserve_trip_list(button_inside_int[index], direction);
					}
				}
			}
		}
		cout << "FLOOR LIST IS SAME..."<< endl;
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

bool simulation::modify_trip_list(vector<vector<int>> button_outside, bool direction)
{
	//    button_detected_elevator_outside = [[8, True], [12, False], ...]
	for(const auto& elem : button_outside)
	{
		
	}
	return true;
}

bool simulation::is_main_trip_list_empty()
{
	return this->main_trip_list.empty();
}

bool simulation::check_reachability()
{
	if(true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

const void simulation::dev_print_trip_list()
{
	cout << endl << "MAIN TRIP LIST IS : ";
	for(auto elem : this->main_trip_list)
	{
		cout << " " << elem;
	}
	cout << endl;
	cout << "RESERVE TRIP UP LIST IS : ";
	for(auto elem : this->reserved_trip_list_up)
	{
		cout << " " << elem;
	}
	cout << endl;
	cout << "RESERVE TRIP DOWN LIST IS : ";
	for(auto elem : this->reserved_trip_list_down)
	{
		cout << " " << elem;
	}
	cout << endl;
}

int simulation::string_floor_to_int(const string& floor)
{
	size_t pos = floor.find_first_of("B");

	if(pos != string::npos)
	{
		string remaining = floor.substr(pos+1);

		return -stoi(remaining);
	}
	else
	{
		return stoi(floor);
	}
}

physics::physics(int underground_floor, int ground_floor, vector<double> altimeter_of_each_floor)
{
	this->info.underground_floor = underground_floor;
	this->info.ground_floor = ground_floor;
	this->info.altimeter_of_each_floor = altimeter_of_each_floor;

	this->current_velocity = 0.0;
	this->current_altimeter = -55;

	this->current_direction = NULL;
}

bool physics::set_initial_elevator_direction(vector<string> button_inside)
{
	if(button_inside.empty())
	{
		cout << "ERROR ON CLASS PHYSICS : " << " button_inside IS EMPTY..." << endl;
		exit(0);
	}
	else
	{
		const int called_floor = this->s.string_floor_to_int(button_inside[0]);
		const int called_floor_index = called_floor > 0 ? called_floor+(this->info.underground_floor-1) : called_floor+(this->info.underground_floor);
		const int called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];
		if(this->current_altimeter < called_floor_altimeter)
		{
			cout << "INITIAL FLOOR : " << called_floor << " is ABOVE FROM ELEVATOR, SETTING DIRECTION TO True..." << endl;
			return true;
		}
		else
		{
			cout << "INITIAL FLOOR : " << called_floor << " is BELOW FROM ELEVATOR, SETTING DIRECTION TO False..." << endl;
			return false;
		}
	}
}

bool physics::set_initial_elevator_direction(vector<vector<int>> button_outside)
{
	if(button_outside.empty())
	{
		cout << "ERROR ON CLASS PHYSICS : " << " button_outside IS EMPTY..." << endl;
		exit(0);
	}
	else
	{
		const int called_floor = button_outside[0][0];
		const int called_floor_index = called_floor > 0 ? called_floor+(this->info.underground_floor-1) : called_floor+(this->info.underground_floor);
		const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];
		if(this->current_altimeter < called_floor_altimeter)
		{
			cout << "INITIAL FLOOR : " << called_floor << " is ABOVE FROM ELEVATOR, SETTING DIRECTION TO True..." << endl;
			this->current_direction = true;
			return true;
		}
		else
		{
			cout << "INITIAL FLOOR : " << called_floor << " is BELOW FROM ELEVATOR, SETTING DIRECTION TO False..." << endl;
			this->current_direction = false;
			return false;
		}
	}
}

long double physics::timeToVelocity(long double initial_velocity, long double final_velocity, long double acceleration)
{
	return abs((final_velocity - initial_velocity) / acceleration);
}

long double physics::distanceDuringAcceleration(long double initial_velocity, long double final_velocity, long double acceleration)
{
	const long double t_to_final_velocity = timeToVelocity(initial_velocity, final_velocity, acceleration);

	const long double distance_during_acceleration = (initial_velocity * t_to_final_velocity) +
                                          (0.5 * (final_velocity >= initial_velocity ? 1 : -1) * acceleration * std::pow(t_to_final_velocity, 2));

    return distance_during_acceleration;
}

vector<vector<long double>> physics::draw_vt_on_sigle_floor(int floor)
{
	vector<long double> time, velocity, distance;
	vector<vector<long double>> ret;

	long double current_altimeter = this->current_altimeter;

	long double max_velocity = this->info.max_velocity;
	long double current_velocity = this->current_velocity;
	long double zero_velocity = 0.0;

	long double acceleration = this->info.acceleration;
	
	const long double ttr = this->info.ttr;

	long double t_total = ttr * 2;

	const long double t_to_max_velocity = timeToVelocity(current_velocity, max_velocity, acceleration);
	const long double d_deceleration = distanceDuringAcceleration(current_velocity, zero_velocity, acceleration);
	const long double d_current_to_max_acceleration = distanceDuringAcceleration(current_velocity, max_velocity, acceleration);
	const long double d_max_to_zero_acceleration = distanceDuringAcceleration(max_velocity, zero_velocity, acceleration);

	const int called_floor_index = floor > 0 ? floor+(this->info.underground_floor-1) : floor+(this->info.underground_floor);
	const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];

	if(d_deceleration >= abs(called_floor_altimeter - current_altimeter))
	{
		// IF ELEVATOR REACHES DESTINATION WITHOUT ACCELERATE TO MAXIMUM SPEED
	}
	else
	{
		// IF ELEVATOR REACHES DESTINATION WITH ACCELERATE TO MAXIMUM SPEED
		long double accumulated_distance = 0.0;
		const long double tick = 0.01;

		long double t_constant_speed = (abs(called_floor_altimeter - current_altimeter) - d_current_to_max_acceleration - d_max_to_zero_acceleration) / max_velocity;
		long double t_max_to_zero_deceleration = timeToVelocity(max_velocity, zero_velocity, acceleration);

		t_constant_speed = round((t_constant_speed * 100)) / 100;
		t_max_to_zero_deceleration = round((t_max_to_zero_deceleration * 100)) / 100;

		cout << "t_to_max_velocity : " << t_to_max_velocity << " t_constant_speed : " << t_constant_speed << " t_max_to_zero_deceleration : " << t_max_to_zero_deceleration << endl;
        // Acceleration phase
        for (long double t = 0.0; t < t_to_max_velocity; t += tick) {
			accumulated_distance = current_velocity * tick + 0.5 * acceleration * std::pow(t, 2);

            time.push_back(t);
            velocity.push_back(current_velocity + acceleration * t);
            distance.push_back(current_altimeter + accumulated_distance);

			//cout << "TIME : " << t << " VELOCITY : " << current_velocity + acceleration * t << " ALT : " << current_altimeter + accumulated_distance << endl;
        }

        // Constant speed phase
        for (long double t = t_to_max_velocity; t < t_to_max_velocity + t_constant_speed; t += tick) {
			accumulated_distance += max_velocity * tick;

            time.push_back(t);
            velocity.push_back(max_velocity);
            distance.push_back(current_altimeter + accumulated_distance);

			//cout << "TIME : " << t << " VELOCITY : " << max_velocity << " ALT : " << current_altimeter + accumulated_distance << endl;
        }

        // Deceleration phase
        for (long double t = t_to_max_velocity + t_constant_speed + tick; t < t_to_max_velocity + t_constant_speed + t_max_to_zero_deceleration; t += tick) {
			const double delta = t - (t_to_max_velocity + t_constant_speed);

			accumulated_distance += (max_velocity - (delta * acceleration)) * tick + (-0.5 * acceleration * std::pow(tick, 2));

            time.push_back(t);

			if(abs(max_velocity - acceleration * (t - (t_to_max_velocity + t_constant_speed))) < 1e-10)
			{
				velocity.push_back(0.0);
				distance.push_back(called_floor_altimeter);
			}
			else
			{
				velocity.push_back(max_velocity - acceleration * (t - (t_to_max_velocity + t_constant_speed)));
				distance.push_back(current_altimeter + accumulated_distance);
			}

			//cout << "TIME : " << t << " VELOCITY : " << max_velocity - acceleration * (t - (t_to_max_velocity + t_constant_speed)) << " ALT : " << current_altimeter + accumulated_distance << endl;
        }
	}

	ret.push_back(time);
	ret.push_back(velocity);
	ret.push_back(distance);

	return ret;
}