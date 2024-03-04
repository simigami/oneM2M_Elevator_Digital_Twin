#include "elevatorAlgorithmSingle.h"

#define SECOND 1000.0
#define TICK 100.0

elevatorAlgorithmSingle::elevatorAlgorithmSingle(string buildingName, string deviceName) : elevatorAlgorithmDefault(buildingName, deviceName)
{

}

elevatorAlgorithmSingle::~elevatorAlgorithmSingle()
{

}

void elevatorAlgorithmSingle::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::startThread(sock , ueSock, phy);

	thread temp(&elevatorAlgorithmSingle::run, this, sock , ueSock, phy);
	temp.detach();
}

void elevatorAlgorithmSingle::stopThread()
{
	elevatorAlgorithmDefault::stopThread();


}

void elevatorAlgorithmSingle::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	socket_oneM2M* s = sock;
	socket_UnrealEngine* us = ueSock;
	physics* p = phy;
	simulation* sim = p->s;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();
	flags* thisElevatorFlag = getElevatorFlag();

	vector<vector<int>> main_trip_list = sim->main_trip_list;
	vector<vector<int>> outside_button_list;

	const string building_name = this->building_name;
	const string device_name = this->device_name;

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time;

	us->sock.UE_info = us->wrap_for_UE_socket(building_name, device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
	us->send_data_to_UE5(us->sock.UE_info);
	
	while(thisElevatorFlag->isRunning)
	{
		start_time = std::chrono::steady_clock::now();
		//CHECK MAIN TRIP LIST AND current_goTo_Floor_vector_info exists
		if(sim->main_trip_list.empty() && sim->reserved_trip_list_up.empty() && sim->reserved_trip_list_down.empty())
		{
			if(!(thisElevatorFlag->firstOperation || !thisElevatorFlag->IDLEFlag))
			{
				cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << " TURNS TO IDLE, " << " TOTAL MOVE DISTANCE : " << p->totalMoveDistance << endl;
				thisElevatorFlag->isRunning = true;
				thisElevatorFlag->firstOperation = true;
				thisElevatorFlag->IDLEFlag = true;
			}
		}

		if(thisElevatorFlag->isRunning == true && this->current_goTo_Floor_vector_info->empty())
		{
			//cout << "ELEVATOR " << thisElevatorStatus->device_name << " IN IDLE STATE..." << endl;
		}
		else if(thisElevatorFlag->isRunning == true && !this->current_goTo_Floor_vector_info->empty())
		{
			thisElevatorFlag->firstOperation = false;
			//CHECK IF main trip list is modified
			if(!sim->main_trip_list.empty() && this->go_To_Floor != sim->main_trip_list[0][0])
			{
				this->go_To_Floor = sim->main_trip_list[0][0];
			}

			//CHECK IF goTo Floor is SET
			if(!current_goTo_Floor_single_info.empty())
			{
				p->totalMoveDistance += current_goTo_Floor_single_info[1] * ((int)(TICK)/(SECOND));
				p->current_velocity = current_goTo_Floor_single_info[1];
				p->current_altimeter = current_goTo_Floor_single_info[2];

				thisElevatorStatus->velocity = p->current_velocity;
				thisElevatorStatus->altimeter = p->current_altimeter;
				thisElevatorStatus->totalMoveDistance = p->totalMoveDistance;

				//cout << "IN " <<  this->building_name << " -> " << this->device_name << " : CURRENT TRIP INFO : " << current_goTo_Floor_single_info[0] << " : VELOCITY : " << current_goTo_Floor_single_info[1] << " ALTIMETER : " << current_goTo_Floor_single_info[2] << endl;

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

						//4 Second LOCK
						cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << " Stopped At : " << sim->main_trip_list[0][0] << endl;
						free_time = std::chrono::steady_clock::now() + std::chrono::seconds(4);

						if(sim->main_trip_list[0][1] == 1)
						{
							int erasing_inside_floor = sim->main_trip_list[0][0];
							if(sim->main_trip_list.size() >= 2 && sim->main_trip_list[0][0] == sim->main_trip_list[1][0])
							{
								sim->main_trip_list = sim->pop_floor_of_trip_list(sim->main_trip_list);
							}
							sim->main_trip_list = sim->pop_floor_of_trip_list(sim->main_trip_list);

							thisElevatorStatus->button_inside->erase(remove(thisElevatorStatus->button_inside->begin(), thisElevatorStatus->button_inside->end(), erasing_inside_floor), thisElevatorStatus->button_inside->end());
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
							thisElevatorStatus->button_outside->erase(std::remove_if(thisElevatorStatus->button_outside->begin(), 
	                                                    thisElevatorStatus->button_outside->end(), 
	                                                    [erasing_outside_floor](const vector<int>& innerVec) {
	                                                        return !innerVec.empty() && innerVec[0] == erasing_outside_floor;
	                                                    }), 
	                                      thisElevatorStatus->button_outside->end());
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

								rearrangeVector(ueSock, p);
								thisElevatorFlag->IDLEFlag = true;
							}
							else
							{
								this->current_goTo_Floor_vector_info->clear();
								this->current_goTo_Floor_single_info.clear();
								p->clear_data();
								thisElevatorFlag->IDLEFlag = true;
							}
						}

						else
						{
							sim->dev_print_trip_list();
							rearrangeVector(ueSock, p);
							thisElevatorFlag->IDLEFlag = false;
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

void elevatorAlgorithmSingle::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	//CHECK notification_strings
	notificationContent* temp = this->thisNotificationContent;
	physics* p = phy;
	simulation* sim = p->s;
	socket_UnrealEngine* us = ueSock;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();
	flags* thisElevatorFlag = getElevatorFlag();

	//CHECK THIS IS FIRST OPERATION
	if(thisElevatorFlag->firstOperation)
	{
		thisElevatorFlag->firstOperation = false;
		if(!temp->button_inside_list.empty())
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->button_inside_list[0]);
			}
			sim->update_main_trip_list_via_inside_data2(temp->button_inside_list, p->current_direction);
			*(thisElevatorStatus->button_inside) = temp->button_inside_list;
		}
		else if(temp->added_button_outside_floor != 0)
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->added_button_outside_floor);
			}
			sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
			thisElevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});
		}
		cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << " goTo Floor is Changed None to : " << sim->main_trip_list[0][0] << endl;

		//CHANGE V_T Graph
		rearrangeVector(ueSock, p);
		thisElevatorFlag->IDLEFlag = false;
	}
	else
	{
		if(temp->velocity != 0.0)
		{
			
		}
		if(temp->altimeter != 0.0)
		{
			
		}
		if(!temp->button_inside_list.empty())
		{
			sim->prev_button_inside_data2 = *(thisElevatorStatus->button_inside);
			sim->update_main_trip_list_via_inside_data2(temp->button_inside_list, thisElevatorStatus->direction);

			*(thisElevatorStatus->button_inside) = temp->button_inside_list;
		}
		if(temp->goTo != 0)
		{
			
		}
		if(temp->added_button_outside_floor != 0)
		{
			//OUTSIDE FLOOR IS ADDED ONLY
			bool flag = true;
			//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
			for(const auto& elem : *(thisElevatorStatus->button_outside))
			{
				if(temp->added_button_outside_floor == elem[0])
				{
					flag = false;
					break;
				}
			}
			if(flag)
			{
				int currentDestFloor = sim->main_trip_list[0][0];
				sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
				thisElevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});

				if(currentDestFloor != sim->main_trip_list[0][0])
				{
					rearrangeVector(ueSock, p);
					thisElevatorFlag->IDLEFlag = false;
				}
			}
		}
	}
}