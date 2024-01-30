#include "elevator.h"

Elevator::Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) :
sock(parsed_struct, ACP_NAMES),
p(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    }),
UEsock(parsed_struct.building_name, parsed_struct.device_name, parsed_struct.underground_floor, parsed_struct.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, 1.25, 2.5)
{
	this->isRunning = true;
	this->RETRIEVE_interval_millisecond = 50;

	this->Elevator_opcode = 0;
	this->go_To_Floor = 0;
	this->building_name = parsed_struct.building_name;
	this->device_name = parsed_struct.device_name;

	this->floor_info.push_back(parsed_struct.underground_floor);
	this->floor_info.push_back(parsed_struct.ground_floor);

	cout << "CREATED ELEVATOR CLASS BUILDING NAME : " << building_name << " DEVICE : " << device_name << endl;
  }

Elevator::~Elevator()
{
	stop_thread();
}

void Elevator::update_latest_info(int floor, int inout, simulation* s)
{
	//BUTTON INSIDE MODIFY
	if(inout)
	{
		string temp = this->p.s.int_floor_to_string(floor);
		this->latest.button_inside.erase(remove(this->latest.button_inside.begin(), this->latest.button_inside.end(), temp), this->latest.button_inside.end());
		s->prev_button_inside_data.erase(remove(s->prev_button_inside_data.begin(), s->prev_button_inside_data.end(), temp), s->prev_button_inside_data.end());
	}
	//BUTTON OUTSIDE MODIFY
	else
	{
		this->latest.button_outside.erase(
			std::remove_if
			(
                this->latest.button_outside.begin(),
                this->latest.button_outside.end(),
                [floor](const std::vector<int>& vec) { return !vec.empty() && vec[0] == floor; }
            ),
            this->latest.button_outside.end()
		);

		s->prev_button_outside_data.erase(
			std::remove_if
			(
                s->prev_button_outside_data.begin(),
                s->prev_button_outside_data.end(),
                [floor](const std::vector<int>& vec) { return !vec.empty() && vec[0] == floor; }
            ),
            s->prev_button_outside_data.end()
		);
	}
}

void Elevator::start_thread()
{
	thread temp(&Elevator::run, this);
	temp.detach();
}

void Elevator::stop_thread()
{
}

void Elevator::run()
{
	vector<vector<string>> retrieved_string;
	vector<string> ret_bin;

	vector<vector<long double>> temp2;

    system_clock::time_point start;
    std::chrono::duration<double> interval;

	socket_oneM2M s = this->sock;
	socket_UnrealEngine us = this->UEsock;
	physics p = this->p;
	simulation sim = this->p.s;

	vector<vector<int>> main_trip_list = sim.main_trip_list;
	vector<vector<int>> outside_button_list;

	std::chrono::steady_clock::time_point free_time;

	bool flag;

	us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
	us.send_data_to_UE5(this->UEsock.sock.UE_info);

	while(isRunning)
	{
		start = system_clock::now();
		//GET RETRIEVED INFORMATIONS
		retrieved_string = this->RETRIEVE_from_oneM2M();

		//SET or MODIFY MAIN TRIP LIST
		flag = this->latest.empty;

		//IF FIRST ON OPERATION
		if(flag)
		{
			this->latest = this->parse_oneM2M_RETRIEVE_to_STRUCT(retrieved_string);

			if(this->latest.button_outside.empty() && this->latest.button_inside.empty())
			{
				cout << this->building_name << " -> " << this->device_name  << " : latest RETRIEVE info is EMPTY..." << endl;
				continue;
			}
			else
			{
				cout << this->building_name << " -> " << this->device_name  << " : RECEIVED FIRST DATA..." << endl;
				this->latest.empty = false;
				if(!this->latest.button_outside.empty())
				{
					//VERY FIRST REQUEST FROM DATA -> DEFINE ELEVATOR DIRECTION
					//SET INITIAL DIRECTION
					if(p.init == true)
					{
						p.init = false;
						p.set_initial_elevator_direction(this->latest.button_outside);
					}
					sim.update_main_trip_list_via_outside_data(this->latest.button_outside, p.current_direction);
				}

				if(!this->latest.button_inside.empty())
				{
					if(p.init == true)
					{
						p.init = false;
						p.set_initial_elevator_direction(this->latest.button_inside);
					}
					sim.update_main_trip_list_via_inside_data(this->latest.button_inside, p.current_direction);
				}
				cout << "goTo Floor is Changed None to : "  << sim.main_trip_list[0][0] << endl;
				sim.dev_print_trip_list();

				//SEND MODIFY goTo Floor Data To Unreal Engine
				us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
				us.sock.UE_info.goToFloor = sim.main_trip_list[0][0];
				us.send_data_to_UE5(us.sock.UE_info);

				//CHANGE V_T Graph
				current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0][0]);

				it = current_goTo_Floor_vector_info.begin();
				current_goTo_Floor_single_info = *it;
			}
			//interval = system_clock::now() - start;
			//cout << device_name <<  "FIRST OPERTAION TIME : " << interval.count()<< " seconds..." << endl;
			//start = system_clock::now();
		}

		//CHECK WITH PREVIOUS CIN RETRIEVE
		else
		{
			latest_RETRIEVE_STRUCT current = this->parse_oneM2M_RETRIEVE_to_STRUCT(retrieved_string);

			sim.check_cin_and_modify_main_trip_list_between_previous_RETRIEVE(this->latest, current, p.current_direction);
			current.empty = this->latest.empty;
			this->latest = current;

			//CHECK LATEST TRIP INFO WITH MODIFIED TRIP LIST
			if(!latest_trip_list_info.empty() && !sim.main_trip_list.empty())
			{
				//IF CLOSEST goTo Floor is Changed
				if(latest_trip_list_info[0][0] != sim.main_trip_list[0][0])
				{
					cout << "goTo Floor is Changed " << latest_trip_list_info[0][0] << " to " << sim.main_trip_list[0][0] << endl;
					sim.dev_print_trip_list();

					//SEND MODIFY goTo Floor Data To Unreal Engine
					us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
					us.sock.UE_info.goToFloor = sim.main_trip_list[0][0];
					us.send_data_to_UE5(us.sock.UE_info);

					//CHANGE V_T Graph
					current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0][0]);

					it = current_goTo_Floor_vector_info.begin();
					current_goTo_Floor_single_info = *it;
				}
				else
				{
					//cout << "goTo Floor is NOT Changed "  << endl;
				}
			}
			//IF CURRENT TRIP LIST IS ENDED -> CHANGE ELEVATOR STATUS
			if(current_goTo_Floor_single_info == current_goTo_Floor_vector_info.back())
			{
				cout << building_name << " : " << device_name << " -> REACHED " << go_To_Floor << endl;
				if(p.lock)
				{
					p.lock = false;
					//3 Second LOCK
					sim.dev_print_stopped_floor();
					free_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);
				}
				else
				{
					if(std::chrono::steady_clock::now() >= free_time)
					{
						p.lock = true;
						//IF REACHED FLOOR IS FROM INSIDE BUTTON
						if(sim.main_trip_list[0][1] == 1)
						{
							update_latest_info(sim.main_trip_list[0][0], 1, &sim);

							if(sim.main_trip_list.size() >= 2 && sim.main_trip_list[0][0] == sim.main_trip_list[1][0])
							{
								sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);
							}
							sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);

							//DELETE THIS FLOOR ON oneM2M - FOR DUBUG
							string payload;
							for(const auto& elem : sim.main_trip_list)
							{
								if(elem != sim.main_trip_list.front())
								{
									payload += " " + std::to_string(elem[0]);
									sim.prev_button_inside_data.push_back(std::to_string(elem[0]));
								}
							}
							s.socket.cin_create(s.originator_name, "button_inside", payload, 3, device_name, s.Default_CNTs[1], s.Default_INSIDE_CNTs[0]);
						}
						//IF REACHED FLOOR IS FROM OUTSIDE BUTTON
						else
						{
							//DELETE THIS FLOOR ON oneM2M - FOR DUBUG
							string temp = sim.main_trip_list[0][0] > 0 ? std::to_string(sim.main_trip_list[0][0]) : "B"+std::to_string(sim.main_trip_list[0][0] * -1);
							string payload = "None";
							s.socket.cin_create(s.originator_name, "status", payload, 3, device_name, s.Default_CNTs[2], temp);

							update_latest_info(sim.main_trip_list[0][0], 0, &sim);
							sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);
						}

						//IF MAIN TRIP LIST IS EMPTY
						if(sim.main_trip_list.empty())
						{
							sim.prev_button_inside_data.clear();
							sim.swap_trip_list();

							if(!sim.main_trip_list.empty())
							{
								p.swap_direction();

								//SEND NEXT FLOOR TO UE5
								us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
								us.sock.UE_info.goToFloor = sim.main_trip_list[0][0];
								us.send_data_to_UE5(us.sock.UE_info);

								sim.dev_print_trip_list();
								current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0][0]);

								it = current_goTo_Floor_vector_info.begin();
								current_goTo_Floor_single_info = *it;
							}
							//IF MAIN, RESERVE TRIP LIST IS EMPTY = No Button Signal From this Elevator
							else
							{
								this->latest = latest_RETRIEVE_STRUCT();
								p.clear_data();
							}
						}
						else
						{
							//SEND NEXT FLOOR TO UE5
							us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
							us.sock.UE_info.goToFloor = sim.main_trip_list[0][0];
							us.send_data_to_UE5(us.sock.UE_info);

							sim.dev_print_trip_list();
							current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0][0]);

							it = current_goTo_Floor_vector_info.begin();
							current_goTo_Floor_single_info = *it;
						}
					}
				}
			}
			else
			{
				++it;
				current_goTo_Floor_single_info = *it;
				p.current_velocity = current_goTo_Floor_single_info[1];
				p.current_altimeter = current_goTo_Floor_single_info[2];
			}
			//interval = system_clock::now() - start;
			//cout << device_name <<  "DEFAULT OPERTAION TIME : " << interval.count()<< " seconds..." << endl;
			//start = system_clock::now();
		}

		if(sim.main_trip_list.empty() && sim.reserved_trip_list_up.empty() && sim.reserved_trip_list_down.empty())
		{
			//cout << this->device_name << " : Operation Finished, Change Status To IDLE..." << endl;
		}
		else
		{
			cout << "CURRENT TRIP INFO : " << current_goTo_Floor_single_info[0] << " : VELOCITY : " << current_goTo_Floor_single_info[1] << " ALTIMETER : " << current_goTo_Floor_single_info[2] << endl;
			latest_trip_list_info = sim.main_trip_list;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(this->RETRIEVE_interval_millisecond));
		interval = system_clock::now() - start;
		//cout << device_name << " TOTAL TICK TIME : " << interval.count()<< " seconds..." << endl;
	}
}

UE_Info Elevator::wrap_for_UE_socket(string building_name, string device_name, int underground_floor, int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity)
{
	UE_Info temp;
	temp.building_name = building_name;
	temp.device_name = device_name;
	temp.underground_floor = underground_floor;
	temp.ground_floor = ground_floor;
	temp.each_floor_altimeter = each_floor_altimeter;
	temp.acceleration = acceleration;
	temp.max_velocity = max_velocity;

	return temp;
}

vector<vector<string>> Elevator::RETRIEVE_from_oneM2M()
{
	//RETRIEVE from oneM2M Resource Tree Based on its building_name and device_name
	return this->sock.retrieve_oneM2M_cins(this->floor_info);
}

latest_RETRIEVE_STRUCT Elevator::parse_oneM2M_RETRIEVE_to_STRUCT(vector<vector<string>> ret)
{
	string floor, direction;
	int floor_int, direction_int;

	latest_RETRIEVE_STRUCT temp;

	temp.velocity = stod(ret[0][0]);
	temp.altimeter = stod(ret[1][0]);

	temp.button_inside = ret[2];

	for(const string& elem : ret[3])
	{
		stringstream ss(elem);

		getline(ss, floor, ':');
		getline(ss >> ws, direction);

		floor_int = this->p.s.string_floor_to_int(floor);
		direction_int = direction._Equal("Up") ? 1 : 0;

		temp.button_outside.push_back({floor_int, direction_int});
	}

	return temp;
}

void Elevator::dev_print()
{
	cout << "ELEVATOR : " << this << " DEV PRINT" << endl;
	cout << "SOCK : " << this->sock.socket_name << endl;
	cout << "PHYSICS : " << this->p.current_velocity << endl;
	cout << "BUILDING NAME : " << this->building_name << endl;
	cout << "DEVICE NAME : " << this->device_name << endl;
}

/*
 * 			string first_goTo_floor = this->latest_RETRIEVE_info[2].front();
			int goTo_floor_index = this->p.s.string_floor_to_int(first_goTo_floor);

			//DRAW V_T TABLE FOR FIRST GOTO FLOOR
			this->current_goTo_Floor_vector_info = this->p.draw_vt_on_sigle_floor(goTo_floor_index);
 */