/*
#include "elevator2.h"
#include <nlohmann/json.hpp>

Elevator2::Elevator2(Wparsed_struct parsed_struct, vector<wstring> ACP_NAMES)
{
	this->elevatorStatus = new Elevator_Status;
	this->each_notification_string = new notification_strings;

	this->sock = new socket_oneM2M(parsed_struct, ACP_NAMES);
	this->p = new physics(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    });

	string buildingName;
	string deivceName;

	buildingName.assign(parsed_struct.building_name.begin(), parsed_struct.building_name.end());
	deivceName.assign(parsed_struct.device_name.begin(), parsed_struct.device_name.end());

	this->UEsock = new socket_UnrealEngine();

	this->thisElevatorStatus->current_goTo_floor_vector_info = new vector<vector<double>>;
	this->thisElevatorStatus->current_goTo_Floor_single_info = vector<double>{};

	this->isRunning = true;
	this->firstOperation = true;

	this->RETRIEVE_interval_millisecond = 10;

	this->Elevator_opcode = 0;
	this->thisElevatorStatus->go_to_floor = 0;

	this->elevatorStatus->building_name = parsed_struct.building_name;
	this->elevatorStatus->device_name = parsed_struct.device_name;

	//std::cout << "CREATED ELEVATOR CLASS BUILDING : " << this->elevatorStatus->building_name << " Elevator  : " << this->elevatorStatus->device_name << std::endl;
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

	const wstring building_name = this->elevatorStatus->building_name;
	const wstring device_name = this->elevatorStatus->device_name;

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time;

	string buildingName;
	string deivceName;

	buildingName.assign(building_name.begin(), building_name.end());
	deivceName.assign(device_name.begin(), device_name.end());

	us->sock.UE_info = us->wrap_for_UE_socket(buildingName, deivceName, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
	us->send_data_to_UE5(this->UEsock->sock.UE_info);
	
	while(isRunning)
	{
		start_time = std::chrono::steady_clock::now();
		//CHECK MAIN TRIP LIST AND this->thisElevatorStatus->current_goTo_floor_vector_info exists
		if(sim->main_trip_list.empty() && sim->reserved_trip_list_up.empty() && sim->reserved_trip_list_down.empty())
		{
			if(firstFlag == false && IDLEFlag == false)
			{
				std::cout << "ELEVATOR " << this->elevatorStatus->device_name << " TURNS TO IDLE..." << std::endl;
				//ADD DELETE
				isRunning = true;
				IDLEFlag = true;
				firstFlag = true;
				firstOperation = true;
			}
		}

		if(isRunning == true && this->thisElevatorStatus->current_goTo_floor_vector_info->empty())
		{
			//std::cout << "ELEVATOR " << this->elevatorStatus->device_name << " IN IDLE STATE..." << std::endl;
		}
		else if(isRunning == true && !this->thisElevatorStatus->current_goTo_floor_vector_info->empty())
		{
			firstFlag = false;
			//CHECK IF main trip list is modified
			if(!sim->main_trip_list.empty() && this->thisElevatorStatus->go_to_floor != sim->main_trip_list[0][0])
			{
				this->thisElevatorStatus->go_to_floor = sim->main_trip_list[0][0];
			}

			//CHECK IF goTo Floor is SET
			if(!this->thisElevatorStatus->current_goTo_Floor_single_info.empty())
			{
				p->current_velocity = this->thisElevatorStatus->current_goTo_Floor_single_info[1];
				p->current_altimeter = this->thisElevatorStatus->current_goTo_Floor_single_info[2];

				this->elevatorStatus->velocity = p->current_velocity;
				this->elevatorStatus->altimeter = p->current_altimeter;

				//std::cout << this->elevatorStatus->device_name << " : CURRENT TRIP INFO : " << this->thisElevatorStatus->current_goTo_Floor_single_info[0] << " : VELOCITY : " << this->thisElevatorStatus->current_goTo_Floor_single_info[1] << " ALTIMETER : " << this->thisElevatorStatus->current_goTo_Floor_single_info[2] << std::endl;

				//CHECK IF THIS VECTOR IS NEAR END
				if(!(this->thisElevatorStatus->current_goTo_Floor_single_info == this->thisElevatorStatus->current_goTo_floor_vector_info->back()))
				{
					++it;
					this->thisElevatorStatus->current_goTo_Floor_single_info = *it;
				}

				//IF REACHED FLOOR
				else
				{
					if(p->lock)
					{
						p->lock = false;
						//3 Second LOCK
						std::cout << this->elevatorStatus->device_name;
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
							wstring temp = sim->main_trip_list[0][0] > 0 ? to_wstring(sim->main_trip_list[0][0]) : L"B"+to_wstring(sim->main_trip_list[0][0] * -1);
							wstring payload = L"None";
							s->socket.cin_create(L"status", payload, 3, device_name, s->Default_CNTs[2], temp);

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

								this->thisElevatorStatus->current_goTo_floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

								//SEND NEXT FLOOR TO UE5
								us->sock.UE_info = us->wrap_for_UE_socket(buildingName, deivceName, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
								us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
								us->send_data_to_UE5(us->sock.UE_info);

								this->thisElevatorStatus->go_to_floor = sim->main_trip_list[0][0];

								it = this->thisElevatorStatus->current_goTo_floor_vector_info->begin();
								this->thisElevatorStatus->current_goTo_Floor_single_info = *it;
							}
							else
							{
								this->thisElevatorStatus->current_goTo_floor_vector_info->clear();
								this->thisElevatorStatus->current_goTo_Floor_single_info.clear();
								p->clear_data();
								IDLEFlag = false;
							}
						}

						else
						{
							sim->dev_print_trip_list();
							this->thisElevatorStatus->current_goTo_floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

							us->sock.UE_info = us->wrap_for_UE_socket(buildingName, deivceName, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
							us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
							us->send_data_to_UE5(us->sock.UE_info);

							this->thisElevatorStatus->go_to_floor = sim->main_trip_list[0][0];

							it = this->thisElevatorStatus->current_goTo_floor_vector_info->begin();
							this->thisElevatorStatus->current_goTo_Floor_single_info = *it;
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

	const wstring building_name = this->elevatorStatus->building_name;
	const wstring device_name = this->elevatorStatus->device_name;

	string buildingName;
	string deivceName;

	buildingName.assign(building_name.begin(), building_name.end());
	deivceName.assign(device_name.begin(), device_name.end());

	//CHECK THIS IS FIRST OPERATION
	if(firstOperation)
	{
		firstOperation = false;
		if(!temp->button_inside.empty())
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->button_inside[0]);
			}
			sim->update_main_trip_list_via_inside_data(temp->button_inside, p->current_direction);
			*(this->elevatorStatus->button_inside) = temp->button_inside;
		}
		else if(temp->button_outside_floor != 0)
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->button_outside_floor);
			}
			sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->button_outside_floor, temp->button_outside_direction, temp->button_outside_altimeter);
			this->elevatorStatus->button_outside->push_back({temp->button_outside_floor, temp->button_outside_direction});
		}
		std::cout << "goTo Floor is Changed None to : "  << sim->main_trip_list[0][0] << std::endl;
		//sim->dev_print_trip_list();

		//CHANGE V_T Graph
		this->thisElevatorStatus->current_goTo_floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

		this->thisElevatorStatus->go_to_floor = sim->main_trip_list[0][0];

		//SEND MODIFY goTo Floor Data To Unreal Engine
		us->sock.UE_info = us->wrap_for_UE_socket(buildingName, deivceName, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
		us->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
		us->send_data_to_UE5(us->sock.UE_info);

		it = this->thisElevatorStatus->current_goTo_floor_vector_info->begin();
		this->thisElevatorStatus->current_goTo_Floor_single_info = *it;
	}
	else
	{
		if(temp->velocity != 0.0)
		{
			
		}
		if(temp->altimeter != 0.0)
		{
			
		}
		if(!temp->button_inside.empty())
		{
			sim->prev_button_inside_data2 = *(this->elevatorStatus->button_inside);
			sim->update_main_trip_list_via_inside_data(temp->button_inside, this->elevatorStatus->direction);

			*(this->elevatorStatus->button_inside) = temp->button_inside;
		}
		if(temp->goTo != 0)
		{
			
		}
		if(temp->button_outside_floor != 0)
		{
			//OUTSIDE FLOOR IS ADDED ONLY
			bool flag = true;
			//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
			for(const auto& elem : *(this->elevatorStatus->button_outside))
			{
				if(temp->button_outside_floor == elem[0])
				{
					flag = false;
					break;
				}
			}
			if(flag)
			{
				sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->button_outside_floor, temp->button_outside_direction, temp->button_outside_altimeter);
				this->elevatorStatus->button_outside->push_back({temp->button_outside_floor, temp->button_outside_direction});
			}
		}
	}
}
*/