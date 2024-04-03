#include "elevatorAlgorithmMultiple.h"

elevatorAlgorithmMultiple::elevatorAlgorithmMultiple(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time) : elevatorAlgorithmDefault(buildingName, deviceName, this_building_creation_time)
{

}

elevatorAlgorithmMultiple::~elevatorAlgorithmMultiple()
{

}

string elevatorAlgorithmMultiple::getJSONString()
{
	return elevatorAlgorithmDefault::getJSONString();
}

void elevatorAlgorithmMultiple::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::startThread(sock, ueSock, phy);

	thread temp(&elevatorAlgorithmMultiple::run, this, sock , ueSock, phy);
	temp.detach();
}

void elevatorAlgorithmMultiple::stopThread()
{
	elevatorAlgorithmDefault::stopThread();
}

void elevatorAlgorithmMultiple::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::run(sock, ueSock, phy);

	socket_oneM2M* s = sock;
	socket_UnrealEngine* us = ueSock;
	physics* p = phy;
	simulation* sim = p->s;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();

	flags* thisElevatorFlag = getElevatorFlag();

	notificationContent* notiContent = this->thisNotificationContent;

	vector<vector<int>> main_trip_list = sim->main_trip_list;
	vector<vector<int>> outside_button_list;

	const wstring building_name = this->thisElevatorStatus->get_building_name();
	const wstring device_name = this->thisElevatorStatus->get_device_name();

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;
	
	while(thisElevatorFlag->isRunning)
	{
		startTime = std::chrono::steady_clock::now();
		if(!(thisElevatorFlag->firstOperation || !thisElevatorFlag->IDLEFlag || sim->bCheckAllListEmpty()))
		{
			//ELEVATOR TURN STATE TO IDLE
			std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " TURNS TO IDLE..." << " TOTAL MOVE DISTANCE : " << p->total_move_distance << std::endl;
			elevatorAlgorithmDefault::	printTimeDeltaWhenStop();
			this->stop(p);

			thisElevatorFlag->isRunning = true;
			thisElevatorFlag->firstOperation = true;
			thisElevatorFlag->IDLEFlag = true;
		}
		else if(!this->thisElevatorStatus->current_goTo_Floor_single_info.empty())
		{
			//std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " : CURRENT TRIP INFO : " << this->thisElevatorStatus->current_goTo_Floor_single_info[0] << " : VELOCITY : " << this->thisElevatorStatus->current_goTo_Floor_single_info[1] << " ALTIMETER : " << this->thisElevatorStatus->current_goTo_Floor_single_info[2] << std::endl;

			p->total_move_distance += this->thisElevatorStatus->current_goTo_Floor_single_info[1] * ((int)(TICK)/(SECOND));
			p->current_velocity = this->thisElevatorStatus->current_goTo_Floor_single_info[1];
			thisElevatorStatus->altimeter = this->thisElevatorStatus->current_goTo_Floor_single_info[2];

			thisElevatorStatus->velocity = p->current_velocity;
			thisElevatorStatus->altimeter = thisElevatorStatus->altimeter;
			thisElevatorStatus->total_move_distance = p->total_move_distance;

			if(!sim->main_trip_list.empty() && this->thisElevatorStatus->go_to_floor != sim->main_trip_list[0][0])
			{
				std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " : IS START FROM : " << this->thisElevatorStatus->go_to_floor << " TO " << sim->main_trip_list[0][0] << std::endl;

				this->thisElevatorStatus->go_to_floor = sim->main_trip_list[0][0];
			}

			if(!(this->thisElevatorStatus->current_goTo_Floor_single_info == this->thisElevatorStatus->current_goTo_floor_vector_info->back()))
			{
				++it;
				this->thisElevatorStatus->current_goTo_Floor_single_info = *it;
			}
			//IF REACHED END OF VECTOR
			else
			{
				if(p->lock)
				{
					p->lock = false;

					//3 Second LOCK
					std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Stopped At : " << sim->main_trip_list[0][0] << std::endl;
					elevatorAlgorithmDefault::printTimeDeltaWhenStop();

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
						wstring temp = sim->main_trip_list[0][0] > 0 ? to_wstring(sim->main_trip_list[0][0]) : L"B"+to_wstring(sim->main_trip_list[0][0] * -1);
						wstring payload = L"None";
						s->socket.cin_create(L"status", payload, 3, device_name, s->Default_CNTs[2], temp);

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
							rearrangeVector(this->getElevatorStatus(), ueSock, p);
						}
						else
						{
							thisElevatorFlag->IDLEFlag = true;
							this->thisElevatorStatus->current_goTo_floor_vector_info->clear();
							this->thisElevatorStatus->current_goTo_Floor_single_info.clear();
							p->clear_data();
						}
					}
					else
					{
						sim->dev_print_trip_list();
						rearrangeVector(this->getElevatorStatus(), ueSock, p);
					}
			}
			}
		}
		endTime = std::chrono::steady_clock::now();
		long long elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

		long long sleep_time_ms = TICK - elapsed_time_ms;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
	}
}

void elevatorAlgorithmMultiple::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::rearrangeVector(stats, ueSock, phy);
}

void elevatorAlgorithmMultiple::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	//CHECK notification_strings
	notificationContent* notiContent = this->thisNotificationContent;
	physics* p = phy;
	simulation* sim = p->s;
	socket_UnrealEngine* us = ueSock;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();
	flags* thisElevatorFlag = getElevatorFlag();

	//CHECK THIS IS FIRST OPERATION
	if(thisElevatorFlag->firstOperation)
	{
		if(!notiContent->button_inside.empty())
		{
			thisElevatorFlag->firstOperation = false;
			p->set_initial_elevator_direction(notiContent->button_inside[0]);
			sim->update_main_trip_list_via_inside_data(notiContent->button_inside, thisElevatorStatus->direction);
			*(thisElevatorStatus->button_inside) = notiContent->button_inside;
		}
		else if(notiContent->button_outside_floor != 0)
		{
			thisElevatorFlag->firstOperation = false;
			p->set_initial_elevator_direction(notiContent->button_outside_floor);
			sim->update_main_trip_list_via_outside_data2(thisElevatorStatus->direction, thisElevatorStatus->altimeter, notiContent->button_outside_floor, notiContent->button_outside_direction, notiContent->button_outside_altimeter);
			thisElevatorStatus->button_outside->push_back({notiContent->button_outside_floor, notiContent->button_outside_direction});
		}

		std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << "goTo Floor is Changed None -> " << sim->main_trip_list[0][0] << std::endl;
		rearrangeVector(this->getElevatorStatus(), ueSock, p);
	}

	//IF NOTIFICATION ACCURRED WHILE ELEVATOR OPERATING
	else
	{
		if(notiContent->current_velocity != 0.0)
		{
			
		}
		if(notiContent->current_altimeter != 0.0)
		{
			
		}
		if(!notiContent->button_inside.empty())
		{
			const int currentDestFloor = sim->main_trip_list.empty()? 0 : sim->main_trip_list[0][0];

			sim->prev_button_inside_data2 = *(thisElevatorStatus->button_inside);
			sim->update_main_trip_list_via_inside_data(notiContent->button_inside, thisElevatorStatus->direction);

			*(thisElevatorStatus->button_inside) = notiContent->button_inside;

			if(currentDestFloor != 0 && currentDestFloor != sim->main_trip_list[0][0] && p->lock)
			{
				rearrangeVector(this->getElevatorStatus(), ueSock, p);
			}
		}
		if(notiContent->button_outside_floor != 0)
		{
			//OUTSIDE FLOOR IS ADDED ONLY
			bool flag = true;
			//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
			for(const auto& elem : *(thisElevatorStatus->button_outside))
			{
				if(notiContent->button_outside_floor == elem[0])
				{
					flag = false;
					break;
				}
			}

			//IF NEW CALLED FLOOR IS OCCURRED
			//CHECK IF NEAREST-N Has CALLED
			if(flag)
			{
				const int currentDestFloor = sim->main_trip_list.empty()? 0 : sim->main_trip_list[0][0];
				double currentDestFloorAltimeter = p->floorToAltimeter(currentDestFloor, p->info.altimeter_of_each_floor);

				if(this->bFirstMoveFlag)
				{
					double currentAltimeterToFloorRatio = p->altimeterToFloorRatio(thisElevatorStatus->altimeter, p->info.altimeter_of_each_floor);

					//this->bFirstMoveFlag = false;
					sim->update_main_trip_list_via_outside_data_Nearest_N(
						thisElevatorStatus->direction, thisElevatorStatus->altimeter, 
						notiContent->button_outside_floor, 
						notiContent->button_outside_direction, 
						notiContent->button_outside_altimeter,
						currentDestFloorAltimeter,
						currentAltimeterToFloorRatio,
						this->ReverseIntervalMovingDeltaMaximum);

					thisElevatorStatus->button_outside->push_back({notiContent->button_outside_floor, notiContent->button_outside_direction});
				}
				else
				{
					sim->update_main_trip_list_via_outside_data2(thisElevatorStatus->direction, thisElevatorStatus->altimeter, notiContent->button_outside_floor, notiContent->button_outside_direction, notiContent->button_outside_altimeter);
					thisElevatorStatus->button_outside->push_back({notiContent->button_outside_floor, notiContent->button_outside_direction});
				}

				if(currentDestFloor != 0 && currentDestFloor != sim->main_trip_list[0][0] && p->lock)
				{
					rearrangeVector(this->getElevatorStatus(), ueSock, p);
				}
			}
		}
	}
}

void elevatorAlgorithmMultiple::appendLogToLogList(int code, ...)
{
	elevatorAlgorithmDefault::appendLogToLogList(code);
}

void elevatorAlgorithmMultiple::writeLog()
{
	elevatorAlgorithmDefault::writeLog();
}

int elevatorAlgorithmMultiple::printTimeDeltaNow()
{
	return elevatorAlgorithmDefault::printTimeDeltaNow();
}

void elevatorAlgorithmMultiple::moveToDeterminedFloor(socket_UnrealEngine* ueSock, physics* phy)
{
	if(!this->thisFlags->IDLEFlag)
	{
		std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Error : moveToDeterminedFloor is called when not IDLE" << std::endl;
		return;
	}


}
