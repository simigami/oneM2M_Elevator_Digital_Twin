#include "elevatorAlgorithmMultiple.h"

#define SECOND 1000.0
#define TICK 100.0

elevatorAlgorithmMultiple::elevatorAlgorithmMultiple(string buildingName, string deviceName) : elevatorAlgorithmDefault(buildingName, deviceName)
{

}

elevatorAlgorithmMultiple::~elevatorAlgorithmMultiple()
{

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

	const string building_name = this->building_name;
	const string device_name = this->device_name;

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point endTime;
	
	while(thisElevatorFlag->isRunning)
	{
		startTime = std::chrono::steady_clock::now();
		if(!(thisElevatorFlag->firstOperation || !thisElevatorFlag->IDLEFlag || sim->bCheckAllListEmpty()))
		{
			//ELEVATOR TURN STATE TO IDLE
			cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << " TURNS TO IDLE..." << " TOTAL MOVE DISTANCE : " << p->totalMoveDistance << endl;

			thisElevatorFlag->isRunning = true;
			thisElevatorFlag->firstOperation = true;
			thisElevatorFlag->IDLEFlag = true;
		}
		else if(!current_goTo_Floor_single_info.empty())
		{
			//cout << "IN " <<  this->building_name << " -> " << this->device_name << " : CURRENT TRIP INFO : " << current_goTo_Floor_single_info[0] << " : VELOCITY : " << current_goTo_Floor_single_info[1] << " ALTIMETER : " << current_goTo_Floor_single_info[2] << endl;

			p->totalMoveDistance += current_goTo_Floor_single_info[1] * ((int)(TICK)/(SECOND));
			p->current_velocity = current_goTo_Floor_single_info[1];
			p->current_altimeter = current_goTo_Floor_single_info[2];

			thisElevatorStatus->velocity = p->current_velocity;
			thisElevatorStatus->altimeter = p->current_altimeter;
			thisElevatorStatus->totalMoveDistance = p->totalMoveDistance;

			if(!sim->main_trip_list.empty() && this->go_To_Floor != sim->main_trip_list[0][0])
			{
				cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << " : IS START FROM : " << this->go_To_Floor << " TO " << sim->main_trip_list[0][0] << endl;

				this->go_To_Floor = sim->main_trip_list[0][0];
			}

			if(!(current_goTo_Floor_single_info == current_goTo_Floor_vector_info->back()))
			{
				++it;
				current_goTo_Floor_single_info = *it;
			}
			//IF REACHED END OF VECTOR
			else
			{
				if(p->lock)
				{
					p->lock = false;

					//3 Second LOCK
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
						}
						else
						{
							thisElevatorFlag->IDLEFlag = true;
							this->current_goTo_Floor_vector_info->clear();
							this->current_goTo_Floor_single_info.clear();
							p->clear_data();
						}
					}
					else
					{
						sim->dev_print_trip_list();
						rearrangeVector(ueSock, p);
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

void elevatorAlgorithmMultiple::rearrangeVector(socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::rearrangeVector(ueSock, phy);
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
		if(!notiContent->button_inside_list.empty())
		{
			thisElevatorFlag->firstOperation = false;
			p->set_initial_elevator_direction(notiContent->button_inside_list[0]);
			sim->update_main_trip_list_via_inside_data2(notiContent->button_inside_list, p->current_direction);
			*(thisElevatorStatus->button_inside) = notiContent->button_inside_list;
		}
		else if(notiContent->added_button_outside_floor != 0)
		{
			thisElevatorFlag->firstOperation = false;
			p->set_initial_elevator_direction(notiContent->added_button_outside_floor);
			sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, notiContent->added_button_outside_floor, notiContent->added_button_outside_direction, notiContent->added_button_outside_altimeter);
			thisElevatorStatus->button_outside->push_back({notiContent->added_button_outside_floor, notiContent->added_button_outside_direction});
		}

		cout << endl << "IN " <<  this->building_name << " -> " << this->device_name << "goTo Floor is Changed None -> " << sim->main_trip_list[0][0] << endl;
		rearrangeVector(ueSock, p);
		thisElevatorFlag->IDLEFlag = false;
	}

	//IF NOTIFICATION ACCURRED WHILE ELEVATOR OPERATING
	else
	{
		if(notiContent->velocity != 0.0)
		{
			
		}
		if(notiContent->altimeter != 0.0)
		{
			
		}
		if(!notiContent->button_inside_list.empty())
		{
			const int currentDestFloor = sim->main_trip_list.empty()? 0 : sim->main_trip_list[0][0];

			sim->prev_button_inside_data2 = *(thisElevatorStatus->button_inside);
			sim->update_main_trip_list_via_inside_data2(notiContent->button_inside_list, thisElevatorStatus->direction);

			*(thisElevatorStatus->button_inside) = notiContent->button_inside_list;

			if(currentDestFloor != 0 && currentDestFloor != sim->main_trip_list[0][0] && p->lock)
			{
				rearrangeVector(ueSock, p);
				thisElevatorFlag->IDLEFlag = false;
			}
		}
		if(notiContent->goTo != 0)
		{
			
		}
		if(notiContent->added_button_outside_floor != 0)
		{
			//OUTSIDE FLOOR IS ADDED ONLY
			bool flag = true;
			//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
			for(const auto& elem : *(thisElevatorStatus->button_outside))
			{
				if(notiContent->added_button_outside_floor == elem[0])
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
					double currentAltimeterToFloorRatio = p->altimeterToFloorRatio(p->current_altimeter, p->info.altimeter_of_each_floor);

					//this->bFirstMoveFlag = false;
					sim->update_main_trip_list_via_outside_data_Nearest_N(
						p->current_direction, p->current_altimeter, 
						notiContent->added_button_outside_floor, 
						notiContent->added_button_outside_direction, 
						notiContent->added_button_outside_altimeter,
						currentDestFloorAltimeter,
						currentAltimeterToFloorRatio,
						this->ReverseIntervalMovingDeltaMaximum);

					thisElevatorStatus->button_outside->push_back({notiContent->added_button_outside_floor, notiContent->added_button_outside_direction});
				}
				else
				{
					sim->update_main_trip_list_via_outside_data2(p->current_direction, p->current_altimeter, notiContent->added_button_outside_floor, notiContent->added_button_outside_direction, notiContent->added_button_outside_altimeter);
					thisElevatorStatus->button_outside->push_back({notiContent->added_button_outside_floor, notiContent->added_button_outside_direction});
				}

				if(currentDestFloor != 0 && currentDestFloor != sim->main_trip_list[0][0] && p->lock)
				{
					rearrangeVector(ueSock, p);
					thisElevatorFlag->IDLEFlag = false;
				}
			}
		}
	}
}