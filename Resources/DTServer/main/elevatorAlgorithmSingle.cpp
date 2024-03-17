#include "elevatorAlgorithmSingle.h"

elevatorAlgorithmSingle::elevatorAlgorithmSingle(wstring buildingName, wstring deviceName) : elevatorAlgorithmDefault(buildingName, deviceName)
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
	elevatorAlgorithmDefault::run(sock, ueSock, phy);

	socket_UnrealEngine* us = ueSock;
	physics* p = phy;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();
	flags* thisElevatorFlag = getElevatorFlag();

	vector<vector<int>> main_trip_list = p->s->main_trip_list;
	vector<vector<int>> outside_button_list;

	const wstring building_name = this->thisElevatorStatus->get_building_name();
	const wstring device_name = this->thisElevatorStatus->get_device_name();

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time;

	this->set_elevator_Status_JSON_STRING();
	us->send_data_to_UE5(this->elevatorStatus_JSON_STRING);
	
	while(thisElevatorFlag->isRunning)
	{
		start_time = std::chrono::steady_clock::now();
		//CHECK MAIN TRIP LIST AND this->thisElevatorStatus->current_goTo_floor_vector_info exists
		if ((p->s->bCheckAllListEmpty() == true) and (thisElevatorFlag->firstOperation == false) and (thisElevatorFlag->IDLEFlag == true))
		{
			std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " TURNS TO IDLE, " << " TOTAL MOVE DISTANCE : " << p->total_move_distance << std::endl;
			elevatorAlgorithmDefault::	printTimeDeltaWhenStop();
			this->stop(p);

			thisElevatorFlag->isRunning = true;
			thisElevatorFlag->firstOperation = true;
			thisElevatorFlag->IDLEFlag = true;
		}
		if(thisElevatorFlag->isRunning == true && !this->thisElevatorStatus->current_goTo_floor_vector_info->empty())
		{
			thisElevatorFlag->firstOperation = false;
			//CHECK IF main trip list is modified
			if(!p->s->main_trip_list.empty() && this->go_To_Floor != p->s->main_trip_list[0][0])
			{
				this->go_To_Floor = p->s->main_trip_list[0][0];
			}

			//CHECK IF goTo Floor is SET
			if(!this->thisElevatorStatus->current_goTo_Floor_single_info.empty())
			{
				p->total_move_distance += this->thisElevatorStatus->current_goTo_Floor_single_info[1] * ((int)(TICK)/(SECOND));
				p->current_velocity = this->thisElevatorStatus->current_goTo_Floor_single_info[1];
				p->current_altimeter = this->thisElevatorStatus->current_goTo_Floor_single_info[2];

				thisElevatorStatus->velocity = p->current_velocity;
				thisElevatorStatus->altimeter = p->current_altimeter;
				thisElevatorStatus->total_move_distance = p->total_move_distance;
				thisElevatorStatus->wake_up_time += 0.1;

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
						thisElevatorFlag->IDLEFlag = false;

						//4 Second LOCK
						std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Stopped At : " << this->thisElevatorStatus->go_to_floor << std::endl;
						elevatorAlgorithmDefault::printTimeDeltaWhenStop();

						free_time = std::chrono::steady_clock::now() + std::chrono::seconds(4);

						if(p->s->main_trip_list[0][1] == 1)
						{
							int erasing_inside_floor = p->s->main_trip_list[0][0];
							if(p->s->main_trip_list.size() >= 2 && p->s->main_trip_list[0][0] == p->s->main_trip_list[1][0])
							{
								p->s->main_trip_list = p->s->pop_floor_of_trip_list(p->s->main_trip_list);
							}
							p->s->main_trip_list = p->s->pop_floor_of_trip_list(p->s->main_trip_list);

							thisElevatorStatus->button_inside->erase(remove(thisElevatorStatus->button_inside->begin(), thisElevatorStatus->button_inside->end(), erasing_inside_floor), thisElevatorStatus->button_inside->end());
						}

						//IF REACHED FLOOR IS FROM OUTSIDE BUTTON
						else
						{
							int erasing_outside_floor = p->s->main_trip_list[0][0];
							//DELETE THIS FLOOR ON oneM2M
							wstring temp = p->s->main_trip_list[0][0] > 0 ? to_wstring(p->s->main_trip_list[0][0]) : L"B"+to_wstring(p->s->main_trip_list[0][0] * -1);
							wstring payload = L"None";
							sock->socket.cin_create(L"status", payload, 3, device_name, sock->Default_CNTs[2], temp);

							p->s->main_trip_list = p->s->pop_floor_of_trip_list(p->s->main_trip_list);
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
						if(p->s->main_trip_list.empty())
						{
							p->s->prev_button_inside_data.clear();
							p->s->swap_trip_list();

							if(!p->s->main_trip_list.empty())
							{
								p->s->dev_print_trip_list();

								rearrangeVector(this->getElevatorStatus(), ueSock, p);
							}
							else
							{
								this->thisElevatorStatus->current_goTo_floor_vector_info->clear();
								this->thisElevatorStatus->current_goTo_Floor_single_info.clear();
								p->clear_data();
								thisElevatorFlag->IDLEFlag = true;
							}
						}

						else
						{
							p->s->dev_print_trip_list();
							rearrangeVector(this->getElevatorStatus(), ueSock, p);
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

void elevatorAlgorithmSingle::stop(physics* p)
{
	elevatorAlgorithmDefault::stop(p);
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

	if (thisElevatorStatus->get_building_name() == L"EV1") {
		std::wcout << "IN " << this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " goTo Floor is Changed None to : " << temp->added_button_outside_floor << std::endl;
	}

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
			p->s->update_main_trip_list_via_inside_data(temp->button_inside_list, p->current_direction);
			p->current_direction = p->set_direction(p->s->main_trip_list[0][0]);
			*(thisElevatorStatus->button_inside) = temp->button_inside_list;
		}
		else if(temp->added_button_outside_floor != 0)
		{
			if(p->init == true)
			{
				p->init = false;
				p->set_initial_elevator_direction(temp->added_button_outside_floor);
			}
			p->s->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
			p->current_direction = p->set_direction(p->s->main_trip_list[0][0]);

			thisElevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});
		}
		std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " goTo Floor is Changed None to : " << p->s->main_trip_list[0][0] << std::endl;

		//CHANGE V_T Graph
		rearrangeVector(this->getElevatorStatus(), ueSock, p);
	}
	else
	{
		if(temp->current_velocity != 0.0)
		{
			
		}
		if(temp->current_altimeter != 0.0)
		{
			
		}
		if(!temp->button_inside_list.empty())
		{
			p->s->prev_button_inside_data2 = *(thisElevatorStatus->button_inside);
			p->s->update_main_trip_list_via_inside_data(temp->button_inside_list, thisElevatorStatus->direction);
			p->current_direction = p->set_direction(p->s->main_trip_list[0][0]);

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
				int currentDestFloor = p->s->main_trip_list[0][0];
				p->s->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, temp->added_button_outside_floor, temp->added_button_outside_direction, temp->added_button_outside_altimeter);
				p->current_direction = p->set_direction(p->s->main_trip_list[0][0]);

				thisElevatorStatus->button_outside->push_back({temp->added_button_outside_floor, temp->added_button_outside_direction});

				if(currentDestFloor != p->s->main_trip_list[0][0])
				{
					rearrangeVector(this->getElevatorStatus(), ueSock, p);
					thisElevatorFlag->IDLEFlag = false;
				}

			}
		}
	}
}

void elevatorAlgorithmSingle::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::rearrangeVector(stats, ueSock, phy);
}