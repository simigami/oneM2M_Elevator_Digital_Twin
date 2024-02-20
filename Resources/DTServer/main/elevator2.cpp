#include "elevator2.h"
#include <nlohmann/json.hpp>

Elevator2::Elevator2(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES)
{
	this->elevatorStatus = new Elevator_Status;
	this->each_notification_string = new notification_strings;

	this->sock = new socket_oneM2M(parsed_struct, ACP_NAMES);
	this->p = new physics(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    });

	this->UEsock = new socket_UnrealEngine(parsed_struct.building_name, parsed_struct.device_name, parsed_struct.underground_floor, parsed_struct.ground_floor, this->p->info.altimeter_of_each_floor, this->p->info.acceleration, this->p->info.max_velocity);

	this->current_goTo_Floor_vector_info = new vector<vector<double>>;
	this->current_goTo_Floor_single_info = vector<double>{};

	this->isRunning = true;
	this->firstOperation = true;

	this->RETRIEVE_interval_millisecond = 10;

	this->Elevator_opcode = 0;
	this->go_To_Floor = 0;

	this->elevatorStatus->building_name = parsed_struct.building_name;
	this->elevatorStatus->device_name = parsed_struct.device_name;

	//cout << "CREATED ELEVATOR CLASS BUILDING : " << this->elevatorStatus->building_name << " Elevator  : " << this->elevatorStatus->device_name << endl;
  }

void Elevator2::start_thread()
{
	thread temp(&Elevator2::run, this);
	temp.detach();

	//std::future<void> test = std::async(std::launch::async, &Elevator2::run, this);
	//test.get();
}

void Elevator2::stop_thread()
{

}

void Elevator2::run()
{
	bool firstFlag = true;
	IDLEFlag = true;

	socket_oneM2M* s = this->sock;
	socket_UnrealEngine* us = this->UEsock;
	physics* p = this->p;
	simulation* sim = this->p->s;

	vector<vector<int>> main_trip_list = sim->main_trip_list;
	vector<vector<int>> outside_button_list;

	const string building_name = this->elevatorStatus->building_name;
	const string device_name = this->elevatorStatus->device_name;

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time;

	us->sock.UE_info = wrap_for_UE_socket(building_name, device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
	us->send_data_to_UE5(this->UEsock->sock.UE_info);
	
	while(isRunning)
	{
		start_time = std::chrono::steady_clock::now();
		//CHECK MAIN TRIP LIST AND current_goTo_Floor_vector_info exists
		if(sim->main_trip_list.empty() && sim->reserved_trip_list_up.empty() && sim->reserved_trip_list_down.empty())
		{
			if(firstFlag == false && IDLEFlag == false)
			{
				cout << "ELEVATOR " << this->elevatorStatus->device_name << " TURNS TO IDLE..." << endl;
				//ADD DELETE
				isRunning = true;
				IDLEFlag = true;
				firstFlag = true;
				firstOperation = true;
			}
		}

		if(isRunning == true && this->current_goTo_Floor_vector_info->empty())
		{
			//cout << "ELEVATOR " << this->elevatorStatus->device_name << " IN IDLE STATE..." << endl;
		}
		else if(isRunning == true && !this->current_goTo_Floor_vector_info->empty())
		{
			firstFlag = false;
			//CHECK IF main trip list is modified
			if(!sim->main_trip_list.empty() && this->go_To_Floor != sim->main_trip_list[0][0])
			{
				this->go_To_Floor = sim->main_trip_list[0][0];
			}

			//CHECK IF goTo Floor is SET
			if(!current_goTo_Floor_single_info.empty())
			{
				p->current_velocity = current_goTo_Floor_single_info[1];
				p->current_altimeter = current_goTo_Floor_single_info[2];

				this->elevatorStatus->velocity = p->current_velocity;
				this->elevatorStatus->altimeter = p->current_altimeter;

				//cout << this->elevatorStatus->device_name << " : CURRENT TRIP INFO : " << current_goTo_Floor_single_info[0] << " : VELOCITY : " << current_goTo_Floor_single_info[1] << " ALTIMETER : " << current_goTo_Floor_single_info[2] << endl;

				//CHECK IF THIS VECTOR IS NEAR END
				if(!(current_goTo_Floor_single_info == current_goTo_Floor_vector_info->back()))
				{
					++it;
					current_goTo_Floor_single_info = *it;
				}

				//IF REACHED FLOOR
				else
				{
					if(p->lock)
					{
						p->lock = false;
						//3 Second LOCK
						cout << this->elevatorStatus->device_name;
						sim->dev_print_stopped_floor();
						free_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);

						if(sim->main_trip_list[0][1] == 1)
						{
							int erasing_inside_floor = sim->main_trip_list[0][0];
							if(sim->main_trip_list.size() >= 2 && sim->main_trip_list[0][0] == sim->main_trip_list[1][0])
							{
								sim->main_trip_list = sim->pop_floor_of_trip_list(sim->main_trip_list);
							}
							sim->main_trip_list = sim->pop_floor_of_trip_list(sim->main_trip_list);

							this->elevatorStatus->button_inside->erase(remove(this->elevatorStatus->button_inside->begin(), this->elevatorStatus->button_inside->end(), erasing_inside_floor), this->elevatorStatus->button_inside->end());
						}

						//IF REACHED FLOOR IS FROM OUTSIDE BUTTON
						else
						{
							int erasing_outside_floor = sim->main_trip_list[0][0];
							//DELETE THIS FLOOR ON oneM2M
							string temp = sim->main_trip_list[0][0] > 0 ? std::to_string(sim->main_trip_list[0][0]) : "B"+std::to_string(sim->main_trip_list[0][0] * -1);
							string payload = "None";
							s->socket.cin_create(s->originator_name, "status", payload, 3, device_name, s->Default_CNTs[2], temp);

							sim->main_trip_list = sim->pop_floor_of_trip_list(sim->main_trip_list);
							elevatorStatus->button_outside->erase(std::remove_if(elevatorStatus->button_outside->begin(), 
	                                                    elevatorStatus->button_outside->end(), 
	                                                    [erasing_outside_floor](const vector<int>& innerVec) {
	                                                        return !innerVec.empty() && innerVec[0] == erasing_outside_floor;
	                                                    }), 
	                                      elevatorStatus->button_outside->end());
						}
					}

					else if(std::chrono::steady_clock::now() >= free_time)
					{
						p->lock = true;
						if(sim->main_trip_list.empty())
						{
							sim->prev_button_inside_data.clear();
							sim->swap_trip_list();

							if(!sim->main_trip_list.empty())
							{
								sim->dev_print_trip_list();

								current_goTo_Floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

								//SEND NEXT FLOOR TO UE5
								us->sock.UE_info = wrap_for_UE_socket(this->elevatorStatus->building_name, this->elevatorStatus->device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
								us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
								us->send_data_to_UE5(us->sock.UE_info);

								this->go_To_Floor = sim->main_trip_list[0][0];

								it = current_goTo_Floor_vector_info->begin();
								current_goTo_Floor_single_info = *it;
							}
							else
							{
								this->current_goTo_Floor_vector_info->clear();
								this->current_goTo_Floor_single_info.clear();
								p->clear_data();
								IDLEFlag = false;
							}
						}

						else
						{
							sim->dev_print_trip_list();
							current_goTo_Floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

							us->sock.UE_info = wrap_for_UE_socket(this->elevatorStatus->building_name, this->elevatorStatus->device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
							us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
							us->send_data_to_UE5(us->sock.UE_info);

							this->go_To_Floor = sim->main_trip_list[0][0];

							it = current_goTo_Floor_vector_info->begin();
							current_goTo_Floor_single_info = *it;
						}
					}
				}
			}
		}
		end_time = std::chrono::steady_clock::now();
		auto elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

		auto sleep_time_ms = 100 - elapsed_time_ms;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
	}
}

void Elevator2::updateElevatorTick()
{
	//CHECK notification_strings
	notification_strings* temp = this->each_notification_string;
	physics* p = this->p;
	simulation* sim = this->p->s;
	socket_UnrealEngine* us = this->UEsock;

	//CHECK THIS IS FIRST OPERATION
	if(firstOperation)
	{
		firstOperation = false;
		if(!temp->button_inside_list.empty())
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->button_inside_list[0]);
			}
			sim->update_main_trip_list_via_inside_data2(temp->button_inside_list, p->current_direction);
			*(this->elevatorStatus->button_inside) = temp->button_inside_list;
		}
		else if(temp->added_button_outside_floor != 0)
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->added_button_outside_floor);
			}
			sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
			this->elevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});
		}
		cout << "goTo Floor is Changed None to : "  << sim->main_trip_list[0][0] << endl;
		//sim->dev_print_trip_list();

		//CHANGE V_T Graph
		current_goTo_Floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

		this->go_To_Floor = sim->main_trip_list[0][0];

		//SEND MODIFY goTo Floor Data To Unreal Engine
		us->sock.UE_info = wrap_for_UE_socket(this->elevatorStatus->building_name, this->elevatorStatus->device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
		us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
		us->send_data_to_UE5(us->sock.UE_info);

		it = current_goTo_Floor_vector_info->begin();
		current_goTo_Floor_single_info = *it;
	}
	else
	{ /*
		latest_RETRIEVE_STRUCT current = this->parse_oneM2M_RETRIEVE_to_STRUCT(retrieved_string);
		//FOR DEBUGGING
		current.altimeter = current_goTo_Floor_single_info[2];

		sim->check_cin_and_modify_main_trip_list_between_previous_RETRIEVE(this->latest, current, p.current_direction);
		current.empty = this->latest.empty;
		this->latest = current;

		//CHECK LATEST TRIP INFO WITH MODIFIED TRIP LIST
		if(!latest_trip_list_info.empty() && !sim->main_trip_list.empty())
		{
			//IF CLOSEST goTo Floor is Changed
			if(latest_trip_list_info[0][0] != sim->main_trip_list[0][0])
			{
				cout << "goTo Floor is Changed" << endl; //<< latest_trip_list_info[0][0] << " to " << sim->main_trip_list[0][0] << endl;
				sim->dev_print_trip_list();

				//CHANGE V_T Graph
				current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim->main_trip_list[0][0]);

				//SEND MODIFY goTo Floor Data To Unreal Engine
				us.sock.UE_info = wrap_for_UE_socket(this->building_name, this->device_name, p.info.underground_floor, p.info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p.info.acceleration, p.info.max_velocity);
				us.set_goTo_Floor(sim->main_trip_list[0][0], p.t_to_max_velocity, p.t_constant_speed, p.t_max_to_zero_deceleration);
				us.send_data_to_UE5(us.sock.UE_info);

				it = current_goTo_Floor_vector_info.begin();
				current_goTo_Floor_single_info = *it;
			}
			else
			{
				//cout << "goTo Floor is NOT Changed "  << endl;
			}
		}
		//IF CURRENT TRIP LIST IS ENDED -> CHANGE ELEVATOR STATUS
		*/

		if(temp->velocity != 0.0)
		{
			
		}
		if(temp->altimeter != 0.0)
		{
			
		}
		if(!temp->button_inside_list.empty())
		{
			sim->prev_button_inside_data2 = *(this->elevatorStatus->button_inside);
			sim->update_main_trip_list_via_inside_data2(temp->button_inside_list, this->elevatorStatus->direction);

			*(this->elevatorStatus->button_inside) = temp->button_inside_list;
		}
		if(temp->goTo != 0)
		{
			
		}
		if(temp->added_button_outside_floor != 0)
		{
			//OUTSIDE FLOOR IS ADDED ONLY
			bool flag = true;
			//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
			for(const auto& elem : *(this->elevatorStatus->button_outside))
			{
				if(temp->added_button_outside_floor == elem[0])
				{
					flag = false;
					break;
				}
			}
			if(flag)
			{
				sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
				this->elevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});
			}
		}
	}
}

UE_Info Elevator2::wrap_for_UE_socket(string building_name, string device_name, int underground_floor, int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity)
{
	UE_Info temp;
	temp.building_name = building_name;
	temp.device_name = device_name;
	temp.underground_floor = underground_floor;
	temp.ground_floor = ground_floor;

	if(each_floor_altimeter[0] < 0)
	{
		for(auto elem : each_floor_altimeter)
		{
			temp.each_floor_altimeter.push_back(elem+abs(each_floor_altimeter[0]));
		}
	}
	else if(each_floor_altimeter[0] > 0)
	{
		for(auto elem : each_floor_altimeter)
		{
			temp.each_floor_altimeter.push_back(elem-abs(each_floor_altimeter[0]));
		}
	}
	temp.acceleration = acceleration;
	temp.max_velocity = max_velocity;

	return temp;
}