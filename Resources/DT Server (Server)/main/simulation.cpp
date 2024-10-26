#include "simulation.h"
#include <algorithm>
#include <unordered_set>

using namespace std;

void simulation::clear_data()
{
	main_trip_list.clear();
	reserved_trip_list_up.clear();
	reserved_trip_list_down.clear();
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

	this->current_velocity = 0.0;

	if (parsed_struct.init_floor != 0)
	{
		this->current_altimeter = floorToAltimeter(parsed_struct.init_floor, parsed_struct.each_floor_altimeter);
	}
	else
	{
		this->current_altimeter = parsed_struct.each_floor_altimeter[0];
	}

	this->current_direction = NULL;
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

long double physics::timeToVelocity(long double initial_velocity, long double final_velocity, long double acceleration)
{
	return abs((final_velocity - initial_velocity) / acceleration);
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

            double this_time_altimeter = current_altimeter;
            double this_time_distance;
            double t_p = 0.0;
			double t = 0.0;

            trajectory.push_back({0.0, 0.0, current_altimeter});

			for (t ; t <= timeToAccelerate; t) 
            {
				const double this_time_velocity = 0 + t * acceleration;
				t += dt;
                this_time_distance = this_time_velocity * dt + (0.5 * acceleration * (t- t_p) * (t- t_p));

                if(timeToAccelerate < t && t < timeToAccelerate + dt)
                {
					double t_p_to_max_distance = (0.5 * acceleration * (timeToAccelerate - t_p)* (timeToAccelerate - t_p));
					double max_to_t_distance = (0.5 * acceleration * (t - timeToAccelerate)* (t - timeToAccelerate));

					this_time_altimeter += multiplyFactor * (t_p_to_max_distance + max_to_t_distance);
					this_time_altimeter = floor(this_time_altimeter * 10000) / 10000.0f;
	                trajectory.push_back({t, reachableVelocity, this_time_altimeter});
                }
                else
                {
	                this_time_altimeter += multiplyFactor * this_time_distance;
	                trajectory.push_back({t, current_velocity + t * acceleration, this_time_altimeter });
                }
				t_p = t;
            }

			t -= timeToAccelerate;
			t_p = t;

			// 2. 목표 속도부터 0으로 감속
			for (t ; t <= timeToDecelerate - dt; t)
            {
				const double this_time_velocity = reachableVelocity - t * acceleration;
				t += dt;
				this_time_altimeter += multiplyFactor * (this_time_velocity * dt + fabs((0.5 * acceleration * (t - t_p) * (t - t_p))));
				this_time_altimeter = floor(this_time_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccelerate, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccelerate, reachableVelocity - t * acceleration, this_time_altimeter});
                }
				t_p = t;
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