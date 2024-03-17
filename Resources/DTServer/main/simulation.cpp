#include "simulation.h"
#include "elevator2.h"
#include <algorithm>
#include <unordered_set>

using namespace std;

void simulation::swap_trip_list()
{
	if(this->main_trip_list.empty())
	{
		if(this->reserved_trip_list_up.size() > this->reserved_trip_list_down.size())
		{
			//std::cout << "UP LIST IS BIGGER THAN DOWN LIST, SWAP MAIN LIST TO RESERVED UP..." << std::endl;
			this->main_trip_list = vector<vector<int>>(this->reserved_trip_list_up.begin(), this->reserved_trip_list_up.end());
			this->reserved_trip_list_up.clear();
		}
		else if(this->reserved_trip_list_down.size() > this->reserved_trip_list_up.size())
		{
			//std::cout << "DOWN LIST IS BIGGER THAN UP LIST, SWAP MAIN LIST TO RESERVED DOWN..." << std::endl;
			this->main_trip_list = vector<vector<int>>(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end());
			this->reserved_trip_list_down.clear();
		}
		else if(this->reserved_trip_list_down.size() == this->reserved_trip_list_up.size() && !this->reserved_trip_list_down.empty())
		{
			//std::cout << "BOTH RESERVED HAS SAME SIZE, SWAP MAIN LIST TO RESERVED DOWN..." << std::endl;
			this->main_trip_list = vector<vector<int>>(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end());
			
			this->reserved_trip_list_down.clear();
		}
		else if(this->reserved_trip_list_down.empty() && this->reserved_trip_list_up.empty())
		{
			return;
		}
	}
}

const void simulation::clear_data()
{
	main_trip_list.clear();
	reserved_trip_list_up.clear();
	reserved_trip_list_down.clear();

	prev_button_inside_data.clear();
	prev_button_outside_data.clear();
}

bool simulation::bCheckAllListEmpty()
{
	if (this->main_trip_list.empty() && this->reserved_trip_list_up.empty() && this->reserved_trip_list_down.empty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool simulation::bigger(vector<int>& v1, vector<int>& v2)
{
	return v1[0] < v2[0];
}

bool simulation::smaller(vector<int>& v1, vector<int>& v2)
{
	return v1[0] > v2[0];
}

bool simulation::add_floor_to_main_trip_list(int floor, bool direction, bool inout) // inout 1 = inside, 0 = outside
{
	auto greaterComparator = [](const std::vector<int>& vec1, const std::vector<int>& vec2) {
        return vec1[0] < vec2[0];
    };

	auto lesserComparator = [](const std::vector<int>& vec1, const std::vector<int>& vec2) {
        return vec1[0] > vec2[0];
    };

	try
	{
		if(direction)
		{
			this->main_trip_list.push_back({floor, inout});
			sort(this->main_trip_list.begin(), this->main_trip_list.end(), greaterComparator);
		}
		else
		{
			this->main_trip_list.push_back({floor, inout});
			sort(this->main_trip_list.begin(), this->main_trip_list.end(), lesserComparator);
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
bool simulation::add_floor_to_reserve_trip_list(int floor, bool direction, bool inout)
{
	auto greaterComparator = [](const std::vector<int>& vec1, const std::vector<int>& vec2) {
        return vec1[0] < vec2[0];
    };

	auto lesserComparator = [](const std::vector<int>& vec1, const std::vector<int>& vec2) {
        return vec1[0] > vec2[0];
    };

	try
	{
		if(direction)
		{
			this->reserved_trip_list_up.push_back({floor, inout});
			sort(this->reserved_trip_list_up.begin(), this->reserved_trip_list_up.end(), greaterComparator);
		}
		else
		{
			this->reserved_trip_list_down.push_back({floor, inout});
			sort(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end(), lesserComparator);
		}
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

vector<vector<int>> simulation::pop_floor_of_trip_list(vector<vector<int>> trip_list)
{
	try
	{
		trip_list.erase(trip_list.begin());
		return trip_list;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
	}
}

bool simulation::erase_floor_of_trip_list(vector<vector<int>> trip_list, int floor)
{
	try
	{
		//trip_list.erase(remove(trip_list.begin(), trip_list.end(), floor), trip_list.end());
		trip_list.erase(
			remove_if(
				trip_list.begin(), trip_list.end(),
				[floor](const vector<int>& vec)
				{
					return !vec.empty() && vec[0] == floor;
				}
			),
			trip_list.end()
		);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		exit(0);
	}
}

void simulation::update_main_trip_list_via_inside_data(vector<int> button_inside, bool direction)
{
	vector<int> prev_button_inside = this->prev_button_inside_data2;

	std::vector<int> new_elements;
    std::vector<int> missing_elements;

    for (const auto& element : button_inside) 
	{
        if (std::find(prev_button_inside.begin(), prev_button_inside.end(), element) == prev_button_inside.end()) 
		{
            new_elements.push_back(element);
        }
    }
    for (const int& element : prev_button_inside) 
	{
        if (std::find(button_inside.begin(), button_inside.end(), element) == button_inside.end()) 
		{
            missing_elements.push_back(element);
        }
    }

    for (const int& floor : new_elements)
	{
		bool flag = check_reachability();
		if(flag)
		{
			add_floor_to_main_trip_list(floor, direction, 1);
		}
		else
		{
			add_floor_to_reserve_trip_list(floor, direction, 1);
		}
    }
    for (const auto& floor : missing_elements) 
	{
		auto it = find_if(this->main_trip_list.begin(), this->main_trip_list.end(),[floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
		if(it != this->main_trip_list.end())
		{
			this->main_trip_list.erase(it);
		}
		else
		{
			it =  find_if(this->reserved_trip_list_up.begin(), this->reserved_trip_list_up.end(), [floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
			if(it != this->reserved_trip_list_up.end())
			{
				this->reserved_trip_list_up.erase(it);
			}
			else
			{
				it =  find_if(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end(), [floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
				if(it != this->reserved_trip_list_down.end())
				{
					this->reserved_trip_list_down.erase(it);
				}
				else
				{
					std::cout << "ERROR OCCURRED ON simulation::update_main_trip_list_via_inside_data : del floor doesn't exist in trip lists..." << std::endl;
					exit(0);
				}
			}
		}
    }
}

void simulation::check_and_set_trip_list(int req_floor, bool direction, bool req_direction, double current_altimeter, double req_altimeter)
{
	if(direction)
	{
		if(req_direction == direction)
		{
			if(current_altimeter < req_altimeter)
			{
				add_floor_to_main_trip_list(req_floor, direction, 0);
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " IS REACHABLE WITH SAME DIRECTION, ADDING ON MAIN LIST..." << std::endl;
			}
			else
			{
				add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " SAME DIRECTION, BUT LOWER ALTIMETER. ADDING ON RESERVED LIST..." << std::endl;
			}
		}
		else
		{
			if(this->main_trip_list.empty())
			{
				add_floor_to_main_trip_list(req_floor, direction, 0);
			}
			else
			{
				add_floor_to_reserve_trip_list(req_floor, req_direction, 0);	
			}
			//std::cout << "OUTSIDE FLOOR : " << req_floor << " DIFFERENT DIRECTION. ADDING ON RESERVED LIST..." << std::endl;
		}
	}
	else
	{
		if(req_direction == direction)
		{
			if(current_altimeter > req_altimeter)
			{
				add_floor_to_main_trip_list(req_floor, direction, 0);
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " IS REACHABLE WITH SAME DIRECTION, ADDING ON MAIN LIST..." << std::endl;
			}
			else
			{
				add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " SAME DIRECTION, BUT LOWER ALTIMETER. ADDING ON RESERVED LIST..." << std::endl;
			}
		}
		else
		{
			if(this->main_trip_list.empty())
			{
				add_floor_to_main_trip_list(req_floor, direction, 0);
			}
			else
			{
				add_floor_to_reserve_trip_list(req_floor, req_direction, 0);	
			}
			//std::cout << "OUTSIDE FLOOR : " << req_floor << " DIFFERENT DIRECTION. ADDING ON RESERVED LIST..." << std::endl;
		}
	}
}

void simulation::check_and_set_trip_list_Nearest_N(
	int req_floor, 
	bool direction, bool req_direction,
	double current_altimeter, double req_altimeter,
	double currentDestFloorAltimeter,
	double currentAltRatio, double intervalMAX)
{
	try
	{
		if (1.0 <= currentAltRatio && currentAltRatio <= 2.0)
		{
			currentAltRatio -= 1;
		}

		if (direction)
		{
			if (req_direction == direction)
			{
				if (current_altimeter < req_altimeter)
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else if (static_cast<double>(req_floor) >= currentAltRatio - intervalMAX)
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else
				{
					add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
				}
			}
			else
			{
				if (this->main_trip_list.empty())
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				// 방향이 다르지만, 호출한 방향으로의 최악으로 많이 간 경우가, 현재 이동하는 Trip Dest보다 작을 때는 이득임
				else if (abs(each_floor_altimeter.front() - current_altimeter) < abs(currentDestFloorAltimeter - current_altimeter))
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else
				{
					add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
				}
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " DIFFERENT DIRECTION. ADDING ON RESERVED LIST..." << std::endl;
			}
		}
		else
		{
			if (req_direction == direction)
			{
				if (current_altimeter > req_altimeter)
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
					//std::cout << "OUTSIDE FLOOR : " << req_floor << " IS REACHABLE WITH SAME DIRECTION, ADDING ON MAIN LIST..." << std::endl;
				}
				else if (static_cast<double>(req_floor) <= currentAltRatio + intervalMAX)
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else
				{
					add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
					//std::cout << "OUTSIDE FLOOR : " << req_floor << " SAME DIRECTION, BUT LOWER ALTIMETER. ADDING ON RESERVED LIST..." << std::endl;
				}
			}
			else
			{
				if (this->main_trip_list.empty())
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else if (abs(each_floor_altimeter.back() - current_altimeter) < abs(currentDestFloorAltimeter - current_altimeter))
				{
					add_floor_to_main_trip_list(req_floor, direction, 0);
				}
				else
				{
					add_floor_to_reserve_trip_list(req_floor, req_direction, 0);
				}
				//std::cout << "OUTSIDE FLOOR : " << req_floor << " DIFFERENT DIRECTION. ADDING ON RESERVED LIST..." << std::endl;
			}
		}

	}
	catch (const std::exception&)
	{
		std::cout << "Exception Caught on simulation.cpp : " << "check_and_set_trip_list_Nearest_N" << std::endl;
	}
}

bool simulation::update_main_trip_list_via_outside_data(vector<vector<int>> button_outside, bool direction, double current_altimeter, vector<double> each_floor_altimeter, int und, int gnd)
{
	try
	{
		vector<vector<int>> prev = this->prev_button_outside_data;
		vector<vector<int>> Currently_Erased(button_outside.size() + prev.size());
		vector<vector<int>> Newly_Added(button_outside.size() + prev.size());

		vector<vector<int>>::iterator iter_prev;
		vector<vector<int>>::iterator iter_next;

		int req_floor;
		double req_altimeter;
		bool req_direction;
		//bool flag;

		sort(button_outside.begin(), button_outside.end());
		sort(prev.begin(), prev.end());

		iter_prev = set_difference(prev.begin(), prev.end(), button_outside.begin(), button_outside.end(), Currently_Erased.begin());
		iter_next = set_difference(button_outside.begin(), button_outside.end(), prev.begin(), prev.end(), Newly_Added.begin());

		Currently_Erased.resize(iter_prev-Currently_Erased.begin());
		Newly_Added.resize(iter_next-Newly_Added.begin());

		//std::cout << "Newly Added Floor : ";
		for(const auto& elem : Newly_Added)
		{
			req_floor = elem[0];

			if(req_floor < 0)
			{
				req_altimeter = each_floor_altimeter[req_floor+und];
			}
			else
			{
				req_altimeter = each_floor_altimeter[und+req_floor-1];
			}

			req_direction = elem[1];
			check_and_set_trip_list(req_floor, direction, req_direction, current_altimeter, req_altimeter);
		}

		for(const vector<int>& elem : Currently_Erased)
		{
			int floor = elem[0];
			auto it = find_if(this->main_trip_list.begin(), this->main_trip_list.end(),[floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
			if(it != this->main_trip_list.end())
			{
				this->main_trip_list.erase(it);
			}
			else
			{
				it = find_if(this->reserved_trip_list_up.begin(), this->reserved_trip_list_up.end(),[floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
				if(it != this->reserved_trip_list_up.end())
				{
					this->reserved_trip_list_up.erase(it);
				}
				else
				{
					it = find_if(this->reserved_trip_list_down.begin(), this->reserved_trip_list_down.end(),[floor](const vector<int>& vec){return !vec.empty() && vec[0] == floor;});
					if(it != this->reserved_trip_list_down.end())
					{
						this->reserved_trip_list_down.erase(it);
					}
					else
					{
						std::cout << "ERROR OCCURRED ON simulation::update_main_trip_list_via_inside_data : del floor doesn't exist in trip lists..." << std::endl;
						exit(0);
					}
				}
			}
			std::cout << "OUTSIDE FLOOR : " << floor << " IS DELETED, ERASE ON MAIN LIST..." << std::endl;
		}
		this->prev_button_outside_data = button_outside;

		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Caught on simulation.cpp : " << e.what();
		return false;
	}
}

bool simulation::update_main_trip_list_via_outside_data2(bool current_direction, long double current_altimeter, int outside_floor, bool outside_direction, double outside_altimeter)
{
	int req_floor = outside_floor;
	double req_altimeter = outside_altimeter;
	bool req_direction = outside_direction;

	check_and_set_trip_list(req_floor, current_direction, req_direction, current_altimeter, req_altimeter);

	return true;
}

bool simulation::update_main_trip_list_via_outside_data_Nearest_N(
	bool current_direction, long double current_altimeter,
	int outside_floor, bool outside_direction, double outside_altimeter,
	double currentDestFloorAltimeter,
	double currentAltRatio, 
	double intervalMAX)
{
	int req_floor = outside_floor;
	double req_altimeter = outside_altimeter;
	bool req_direction = outside_direction;

	check_and_set_trip_list_Nearest_N(req_floor, current_direction, req_direction, current_altimeter, req_altimeter, currentDestFloorAltimeter, currentAltRatio, intervalMAX);

	return true;
}

// direction = Elevator's Current Moving Direction -> True : Up, False : Down
bool simulation::modify_trip_list(vector<string> button_inside, bool direction)
{
	bool flag;
	size_t size = button_inside.size();
	vector<int> button_inside_int;

	for(const auto& elem : button_inside)
	{
		button_inside_int.push_back(string_floor_to_int(elem));
	}
	try
	{
		if(direction)
		{
			for(auto index = 0; index < size; index++)
			{
				if(this->main_trip_list[index][0] != button_inside_int[index])
				{
					//CHECK IF NEW TRIP IS REACHABLE
					flag = check_reachability();
					if(flag)
					{
						add_floor_to_main_trip_list(button_inside_int[index], direction, 1);
						//std::cout << "FLOOR : " << button_inside_int[index] << " IS MISSING, ADDING ON MAIN LIST..." << std::endl;

						return true;
					}
					else
					{
						//std::cout << "FLOOR : " << button_inside_int[index] << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << std::endl;
						add_floor_to_reserve_trip_list(button_inside_int[index], direction, 1);
					}
				}
			}
			std::cout << "FLOOR LIST IS SAME..."<< std::endl;
			return true;
		}
		else
		{
			for(auto index = 0; index < size; index++)
			{
				if(this->main_trip_list[size-1-index][0] != button_inside_int[index])
				{
					//CHECK IF NEW TRIP IS REACHABLE
					flag = check_reachability();
					if(flag)
					{
						add_floor_to_main_trip_list(button_inside_int[index], direction, 1);
						//std::cout << "FLOOR : " << button_inside_int[index] << " IS MISSING, ADDING ON MAIN LIST..." << std::endl;

						return true;
					}
					else
					{
						//std::cout << "FLOOR : " << button_inside_int[index] << " IS UNREACHABLE, ADDING ON RESERVED LIST..." << std::endl;
						add_floor_to_reserve_trip_list(button_inside_int[index], direction, 1);
					}
				}
			}
		}
		std::cout << "FLOOR LIST IS SAME..."<< std::endl;
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
	std::cout << "MAIN TRIP :";
	for(auto elem : this->main_trip_list)
	{
		string temp = elem[1]==1 ? "INSIDE" : "OUTSIDE";
		std::cout << " " << elem[0] << " " << temp << " , ";

		if (elem[0] == 2024) {
			std::cout << "2024 IS IN MAIN TRIP LIST...";
		}
	}
	std::cout << std::endl;

	std::cout << "RESERVE TRIP UP :";
	for(auto elem : this->reserved_trip_list_up)
	{
		string temp = elem[1]==1 ? "INSIDE" : "OUTSIDE";
		std::cout << " " << elem[0] << " " << temp << " , ";
	}
	std::cout << std::endl;

	std::cout << "RESERVE TRIP DOWN :";
	for(auto elem : this->reserved_trip_list_down)
	{
		string temp = elem[1]==1 ? "INSIDE" : "OUTSIDE";
		std::cout << " " << elem[0] << " " << temp << " , ";
	}
	std::cout << std::endl;
}

const void simulation::dev_print_stopped_floor()
{
	std::cout << " Stopped At : " << this->main_trip_list[0][0] << std::endl;
	return void();
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

string simulation::int_floor_to_string(const int& floor)
{
	return floor > 0 ? std::to_string(floor) : "B"+std::to_string(floor*-1);
}

physics::physics(Wparsed_struct parsed_struct)
{
	this->s = new simulation();

	this->info.underground_floor = parsed_struct.underground_floor;
	this->info.ground_floor = parsed_struct.ground_floor;

	this->info.altimeter_of_each_floor = parsed_struct.each_floor_altimeter;
	this->s->each_floor_altimeter = parsed_struct.each_floor_altimeter;

	this->total_move_distance = 0.0;
	this->current_velocity = 0.0;
	this->current_altimeter = -55;

	this->current_direction = NULL;
}

bool physics::set_initial_elevator_direction(vector<string> button_inside)
{
	if(button_inside.empty())
	{
		//std::cout << "ERROR ON CLASS PHYSICS : " << " button_inside IS EMPTY..." << std::endl;
		exit(0);
	}
	else
	{
		const int called_floor = this->s->string_floor_to_int(button_inside[0]);
		const int called_floor_index = called_floor > 0 ? called_floor+(this->info.underground_floor-1) : called_floor+(this->info.underground_floor);
		const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];
		if(this->current_altimeter < called_floor_altimeter)
		{
			//std::cout << "INITIAL FLOOR : " << called_floor << " is ABOVE FROM ELEVATOR, SETTING DIRECTION TO True..." << std::endl;
			this->current_direction = true;
			return true;
		}
		else
		{
			//std::cout << "INITIAL FLOOR : " << called_floor << " is BELOW FROM ELEVATOR, SETTING DIRECTION TO False..." << std::endl;
			this->current_direction = false;
			return false;
		}
	}
}

bool physics::set_initial_elevator_direction(int floor)
{
	const int called_floor_index = floor > 0 ? floor+(this->info.underground_floor-1) : floor+(this->info.underground_floor);
	const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];

	if(this->current_altimeter < called_floor_altimeter)
	{
		this->current_direction = true;
		return true;
	}
	else
	{
		this->current_direction = false;
		return false;
	}
}

bool physics::set_initial_elevator_direction(vector<vector<int>> button_outside)
{
	if(button_outside.empty())
	{
		std::cout << "ERROR ON CLASS PHYSICS : " << " button_outside IS EMPTY..." << std::endl;
		exit(0);
	}
	else
	{
		const int called_floor = button_outside[0][0];
		const int called_floor_index = called_floor > 0 ? called_floor+(this->info.underground_floor-1) : called_floor+(this->info.underground_floor);
		const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];
		if(this->current_altimeter < called_floor_altimeter)
		{
			//std::cout << "INITIAL FLOOR : " << called_floor << " is ABOVE FROM ELEVATOR, SETTING DIRECTION TO True..." << std::endl;
			this->current_direction = true;
			return true;
		}
		else
		{
			//std::cout << "INITIAL FLOOR : " << called_floor << " is BELOW FROM ELEVATOR, SETTING DIRECTION TO False..." << std::endl;
			this->current_direction = false;
			return false;
		}
	}
}

const void physics::clear_data()
{
	physics* p = this;

	p->s->clear_data();

	p->init =true;
	p->lock = true;
	p->current_direction = NULL;
	p->current_velocity = 0.0; 
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

double physics::altimeterToFloorRatio(double altimeter, vector<double> eachFloorAltimeter)
{
    size_t i = 0;

	// 예외 처리: eachFloorAltimeter가 비어있는 경우 또는 altimeter가 범위를 벗어난 경우
    if (eachFloorAltimeter.empty() || altimeter < eachFloorAltimeter.front() || altimeter > eachFloorAltimeter.back()) {
        std::cerr << "Error: Altitude out of range or eachFloorAltimeter is empty." << std::endl;
        exit(0);
    }

    while (i < eachFloorAltimeter.size() - 1 && altimeter > eachFloorAltimeter[i + 1]) {
        i++;
    }

    // 해당 위치의 인덱스와 해당 위치에서의 비율 계산
    double ratio = (altimeter - eachFloorAltimeter[i]) / (eachFloorAltimeter[i + 1] - eachFloorAltimeter[i]);
	ratio = round(ratio * 10) / 10;

	if(i <= this->info.underground_floor-1)
	{
		i = i + (-1)  * this->info.underground_floor;
	}
	else
	{
		i -= (this->info.underground_floor-1);
	}

    return i + ratio; // 인덱스와 비율을 더하여 반환
}

double physics::floorToAltimeter(int floor, vector<double> eachFloorAltimeter) const
{
	return floor > 0 ? eachFloorAltimeter[this->info.underground_floor-1+floor] : eachFloorAltimeter[this->info.underground_floor+floor];
}

bool physics::set_direction(int floor)
{
	const int called_floor = floor;
	const int called_floor_index = called_floor > 0 ? called_floor+(this->info.underground_floor-1) : called_floor+(this->info.underground_floor);
	const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];
	if(this->current_altimeter < called_floor_altimeter)
	{
		this->current_direction = true;
		return true;
	}
	else
	{
		this->current_direction = false;
		return false;
	}
}

const void physics::swap_direction()
{
	this->current_direction = !this->current_direction;
	return void();
}

vector<vector<double>>* physics::draw_vt_on_single_floor(int floor)
{
	vector<double> each_tick_time_velocity_altimeter;
	vector<vector<double>>* ret;
	ret = new std::vector<std::vector<double>>;

	double current_velocity = this->current_velocity;
	double current_altimeter = this->current_altimeter;
	const int called_floor_index = floor > 0 ? floor+(this->info.underground_floor-1) : floor+(this->info.underground_floor);
	const double called_floor_altimeter = this->info.altimeter_of_each_floor[called_floor_index];

	double max_velocity = this->info.max_velocity;
	double acceleration = this->info.acceleration;

	// SET DIRECTION
	this->current_direction = set_direction(floor);

	*ret = getElevatorTrajectory(current_altimeter, current_velocity, called_floor_altimeter, acceleration, max_velocity, this->current_direction);

	return ret;
}

vector<vector<double>> physics::getElevatorTrajectory(double current_altimeter, double current_velocity, double dest_altimeter, double acceleration, double maximum_velocity, bool direction)
{
	// 0.1초 단위 시간 간격
	const double dt = 0.1;

	// 엘리베이터가 정지 상태인지 확인
	bool isStationary = (fabs(current_velocity) < 1e-6);

    // 현재 속도에서 최대 속도까지 가속할 때 가는 거리
    double distance_acceleration = ((maximum_velocity*maximum_velocity)-(current_velocity*current_velocity))/(2*acceleration);

    // maximum_velocity에서 0m/s까지 감속할 때 가는 거리
    double distance_deceleration = 0.5 * maximum_velocity * maximum_velocity / acceleration;

	// 목표 고도까지의 거리 계산
	double distance = fabs(dest_altimeter - current_altimeter);

	// 최대 속도 도달 여부 판단
	bool reachesMaxVelocity = distance > (distance_acceleration + distance_deceleration);

	vector<vector<double>> trajectory;

    int multiplyFactor = 0;

    if(direction)
    {
	    multiplyFactor = 1;
    }
    else
    {
	    multiplyFactor = -1;
    }

	if (isStationary) {

        //엘리베이터가 정지한 경우
	    if (reachesMaxVelocity) {
	    	// 최대 속도 도달 시 3단계 운동
            double distance_max_velocity = distance - distance_acceleration - distance_deceleration;
			double timeToAccVelocity = (maximum_velocity - current_velocity) / acceleration;
			double timeAtMaxVelocity = distance_max_velocity / maximum_velocity;
			double timeToDccVelocity = maximum_velocity / acceleration;

			t_to_max_velocity = timeToAccVelocity;
	    	t_constant_speed = timeAtMaxVelocity;
	    	t_max_to_zero_deceleration = timeToDccVelocity;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

            // 1. 최대 속도까지 가속
	    	for (double t = 0; t < timeToAccVelocity; t)
            {
                t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));
                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
	    	}

            // 2. 최대 속도 유지
			for (double t = 0; t < timeAtMaxVelocity - dt; t)
            {
				t += dt;
				temp_altimeter += multiplyFactor * maximum_velocity * dt;
                trajectory.push_back({t + timeToAccVelocity, maximum_velocity, temp_altimeter});

            }

            // 3. 최대 속도부터 0으로 감속
			double timeToDecelerate = maximum_velocity / acceleration;
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, maximum_velocity - t * acceleration, temp_altimeter});
                }
            }
        }

        else
        {
	        // 1. 목표 속력까지 가속
			double timeToAccelerate = floor(sqrt(distance/acceleration) * 1000) / 1000.0f;
			double timeToDecelerate = timeToAccelerate;
            double reachableVelocity = acceleration * timeToAccelerate;

			t_to_max_velocity = timeToAccelerate;
	    	t_constant_speed = 0.0;
	    	t_max_to_zero_deceleration = timeToAccelerate;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

			for (double t = 0; t <= timeToAccelerate; t) 
            {
				t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));

                if(timeToAccelerate < t && t < timeToAccelerate + dt)
                {
                    temp_altimeter += multiplyFactor * 2 * ((0.5 * acceleration * timeToAccelerate * timeToAccelerate) - previous_temp);
                    temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;
	                trajectory.push_back({t, reachableVelocity, temp_altimeter});
                }
                else
                {
	                temp_altimeter += multiplyFactor * (0.5 * acceleration * t * t) - previous_temp;
	                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
                }
            }

			// 2. 목표 속도부터 0으로 감속
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccelerate, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccelerate, reachableVelocity - t * acceleration, temp_altimeter});
                }
            }
        }
	}
    else
    {
	    if (reachesMaxVelocity) {
	    	// 최대 속도 도달 시 3단계 운동
            double distance_max_velocity = distance - distance_acceleration - distance_deceleration;
			double timeToAccVelocity = (maximum_velocity - current_velocity) / acceleration <= 0 ?  0 : (maximum_velocity - current_velocity) / acceleration;
			double timeAtMaxVelocity = distance_max_velocity == dt ? 0 : distance_max_velocity / maximum_velocity;
			double timeToDccVelocity = maximum_velocity / acceleration;

			t_to_max_velocity = timeToAccVelocity;
	    	t_constant_speed = timeAtMaxVelocity;
	    	t_max_to_zero_deceleration = timeToDccVelocity;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, current_velocity, current_altimeter});

            // 1. 최대 속도까지 가속
	    	for (double t = 0; t < timeToAccVelocity; t)
            {
                t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));
                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                trajectory.push_back({t, current_velocity + t * acceleration <= maximum_velocity ? current_velocity + t * acceleration : maximum_velocity, temp_altimeter});
	    	}

            // 2. 최대 속도 유지
			for (double t = 0; t < timeAtMaxVelocity - dt; t)
            {
				t += dt;
				temp_altimeter += multiplyFactor * maximum_velocity * dt;
                trajectory.push_back({t + timeToAccVelocity, maximum_velocity, temp_altimeter});
            }

            // 3. 최대 속도부터 0으로 감속
			double timeToDecelerate = maximum_velocity / acceleration;
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 1000) / 1000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, maximum_velocity - t * acceleration, temp_altimeter});
                }
            }
        }

        else
        {
            double DistancezeroToCurrentVelocity = 0.5 * (current_velocity/acceleration) * (current_velocity/acceleration) * acceleration;
        	// 1. 목표 속력까지 가속
			double timeToAccelerate = floor(sqrt((distance+DistancezeroToCurrentVelocity)/acceleration) * 1000) / 1000.0f;
			double timeToDecelerate = timeToAccelerate;

            timeToAccelerate -= (current_velocity/acceleration);
            timeToAccelerate = floor(timeToAccelerate * 1000) / 1000.0f;

			t_to_max_velocity = timeToAccelerate;
	    	t_constant_speed = 0.0;
	    	t_max_to_zero_deceleration = timeToDecelerate;

            double reachableVelocity = acceleration * timeToAccelerate;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

			for (double t = 0; t <= timeToAccelerate; t) 
            {
				t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));

                if(timeToAccelerate < t && t < timeToAccelerate + dt)
                {
                    temp_altimeter += multiplyFactor * (2 * ((0.5 * acceleration * timeToAccelerate * timeToAccelerate) - previous_temp));
                    temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;
	                trajectory.push_back({t, reachableVelocity, temp_altimeter});
                }
                else
                {
	                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
	                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
                }
            }

			// 2. 목표 속도부터 0으로 감속
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccelerate, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccelerate, reachableVelocity - t * acceleration, temp_altimeter});
                }
            }
        }
    }
	return trajectory;
}