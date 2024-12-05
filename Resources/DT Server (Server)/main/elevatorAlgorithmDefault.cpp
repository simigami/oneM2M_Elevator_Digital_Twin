#include <nlohmann/json.hpp>
#include "elevatorAlgorithmDefault.h"
#include <mutex>

elevatorAlgorithmDefault::elevatorAlgorithmDefault(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time)
{
	thisFlags = new flags;

	thisNotificationContent = new notificationContent;

	thisElevatorStatus = new elevatorStatus;
	thisElevatorStatus->set_names(buildingName, deviceName);

	thisElevatorStatus->start_time = std::chrono::system_clock::now();
	worldClock = this_building_creation_time;
	RETRIEVE_interval_millisecond = 10;

	thisLogger.log_file_name = thisLogger.get_file_name_as_timestamp() + L"_" + buildingName + L"_" + deviceName + L".txt";
	thisLogger.csv_file_name = thisLogger.get_file_name_as_timestamp() + L"_" + buildingName + L".csv";

	// Check if csv_file_name is already exist
	const wstring csv_file_path = thisLogger.log_directory + L"\\" + thisLogger.csv_file_name;
	std::wifstream csv_file(csv_file_path);
	if (!csv_file.is_open())
	{
		thisLogger.write_rts_csv_header();
	}
	else
	{
		csv_file.close();
	}
}

elevatorAlgorithmDefault::~elevatorAlgorithmDefault()
{

}

void elevatorAlgorithmDefault::set_physical_information(physics* p)
{
	thisElevatorStatus->set_physical_information(p);
}

void elevatorAlgorithmDefault::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
}

void elevatorAlgorithmDefault::stopThread()
{
}

void elevatorAlgorithmDefault::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	socket_UnrealEngine* us = ueSock;
	physics* p = phy;

	elevatorStatus* thisElevatorStatus = getElevatorStatus();
	flags* thisElevatorFlag = getElevatorFlag();

	vector<vector<int>> main_trip_list = p->s->main_trip_list;

	const wstring building_name = this->thisElevatorStatus->get_building_name();
	const wstring device_name = this->thisElevatorStatus->get_device_name();

	std::chrono::steady_clock::time_point free_time;
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time;

	try
	{
		while (true)
		{
			start_time = std::chrono::steady_clock::now();

			if ((thisElevatorFlag->isRunning == false) && (thisElevatorFlag->IDLEFlag == true) && (p->s->bCheckAllListEmpty() == true))
			{
				continue;
			}

			// CHECK IF THIS ELEVATOR TURNS TO IDLE STATE
			else if ((p->s->bCheckAllListEmpty() == true) and (thisElevatorFlag->firstOperation == false) and (thisElevatorFlag->IDLEFlag == true))
			{
				//std::wcout << "IN " << building_name << " -> " << device_name << " TURNS TO IDLE, " << " TOTAL MOVE DISTANCE : " << p->total_move_distance << std::endl;
				const int delta_second = elevatorAlgorithmDefault::printTimeDeltaNow();
				appendLogToLogList(IDLE, 3, building_name, device_name, delta_second);
				writeEnergyLog();
				
				getElevatorStatus()->set_this_elevator_daily_energy_consumption(0);
				printTimeDeltaWhenIDLE();

				stop(p, thisElevatorFlag);
			}

			else
			{
				auto this_elevator_goto_floor_vector = this->thisElevatorStatus->current_goTo_floor_vector_info;

				// CHECK MAIN TRIP LIST IS EMPTY
				if (this_elevator_goto_floor_vector->empty())
				{
					continue;
				}
				else
				{
					// GET ITERATOR FROM ALGORITHM DEFAULT
					auto iter = this->it;
					vector<double> this_elevator_goto_floor_single = *it;

					this->thisElevatorStatus->current_goTo_Floor_single_info = this_elevator_goto_floor_single;

					// THIS MEANS THAT ELEVATOR IS MOVING, so move the this_elevator_goto_floor_vector to next vector
					p->current_velocity = this->thisElevatorStatus->current_goTo_Floor_single_info[1];
					p->current_altimeter = this->thisElevatorStatus->current_goTo_Floor_single_info[2];

					thisElevatorStatus->total_move_distance += this->thisElevatorStatus->current_goTo_Floor_single_info[1] * ((int)(TICK) / (SECOND));
					thisElevatorStatus->velocity = p->current_velocity;
					thisElevatorStatus->altimeter = p->current_altimeter;

					// CHECK IF this_elevator_goto_floor_vector IS END OF current_goTo_Floor_vector_info
					if (this_elevator_goto_floor_single == this->thisElevatorStatus->current_goTo_floor_vector_info->back())
					{
						rearrangeMainTripList(thisElevatorStatus, p);

						std::wcout << "IN " << building_name << " -> " << device_name << " STOPPED AT FLOOR : " << this->thisElevatorStatus->go_to_floor << std::endl;
						getElevatorStatus()->CurrentFloor = this->thisElevatorStatus->go_to_floor;

						getElevatorStatus()->number_of_trips += 1;
						getElevatorStatus()->set_this_elevator_daily_energy_consumption(0);
						//printThisElevatorEnergyConsumptionInfos();

						wstring log_string = L"IN " + building_name + L" -> " + device_name + L" STOPPED AT FLOOR : " + std::to_wstring(this->thisElevatorStatus->go_to_floor);
						thisLogger.write_log(log_string);

						elevatorAlgorithmDefault::printTimeDeltaWhenStop();

						const int delta_second = elevatorAlgorithmDefault::printTimeDeltaNow();
						appendLogToLogList(STOP, 4, building_name, device_name, this->thisElevatorStatus->go_to_floor, delta_second);

						getElevatorStatus()->isElevatorStopped = true;
						std::this_thread::sleep_for(std::chrono::milliseconds((int)thisElevatorStatus->door_open_time * 1000));
						getElevatorStatus()->isElevatorStopped = false;

						// CHECK IF current_goTo_floor_vector_info is empty
						if (p->s->bCheckAllListEmpty() == true)
						{
							// THIS MEANS THAT ELEVATOR HAS STOPPED AT THE LAST FLOOR, so clear the main_trip_list
							p->s->main_trip_list.clear();
							thisElevatorFlag->IDLEFlag = true;
						}
						else
						{
							// THIS MEANS THAT ELEVATOR HAS STOPPED AT THE FLOOR, so erase the this_elevator_goto_floor_vector
							// CHECK IF MAIN TRIP LIST IS EMPTY
							if (p->s->main_trip_list.empty())
							{
								// CHECK RESERVE UP TRIP LIST
								if (p->s->reserved_trip_list_up.empty())
								{
									p->s->main_trip_list = p->s->reserved_trip_list_down;
									p->s->reserved_trip_list_down.clear();
								}
								else
								{
									// move reserved_trip_list_up to main_trip_list
									p->s->main_trip_list = p->s->reserved_trip_list_up;
									p->s->reserved_trip_list_up.clear();
								}
							}
							rearrangeVector(this->getElevatorStatus(), ueSock, p);

							const int delta_second = elevatorAlgorithmDefault::printTimeDeltaNow();
							appendLogToLogList(MOV, 4, building_name, device_name, p->s->main_trip_list[0][0], delta_second);

							if (us->VisualizationMod == 1)
							{
								this->set_elevator_Status_JSON_STRING();
								us->send_data_to_UE5(this->elevatorStatus_JSON_STRING);
							}
						}
					}
					else
					{
						this->it++;
					}
				}

				thisElevatorStatus->wake_up_time += 0.1;
			}
			// Sleep 0.1 - elapsed_seconds
			end_time = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed_seconds = end_time - start_time;
			std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed_seconds);
		}
	}
	catch (const std::exception& e)
	{
		std::wcout << "IN " << building_name << " -> " << device_name << " ERROR OCCURR AT : elevatorAlgorithmDefault::run " << e.what() << std::endl;
	}
}

void elevatorAlgorithmDefault::stop(physics* p, flags* this_flag)
{
	this->thisNotificationContent = new notificationContent;
	
	this_flag->isRunning = false;
	this_flag->firstOperation = true;
	this_flag->IDLEFlag = true;

	this->log_list = vector<wstring>{};

	this->thisElevatorStatus->button_inside->clear();
	this->thisElevatorStatus->button_outside->clear();

	this->thisElevatorStatus->current_goTo_floor_vector_info = new vector<vector<double>>{};
	this->thisElevatorStatus->current_goTo_Floor_single_info = vector<double>{};

	p->s = new simulation();
}

void elevatorAlgorithmDefault::write_logs(std::vector<std::wstring> strings)
{
	thisLogger.write_logs(strings);
}

void elevatorAlgorithmDefault::write_log(std::wstring string)
{
	thisLogger.write_log(string);
}

void elevatorAlgorithmDefault::write_csv_header()
{
	thisLogger.write_rts_csv_header();
}

wstring elevatorAlgorithmDefault::make_csv_string(ElevatorStatusEnum en)
{
	wstring RetVal = L"";
	wstring cmd = L"";

	switch (en)
	{	
	case RTS_SPAWN:
		cmd = L"SPAWN";
		break;
	case RTS_GO_TO_CHANGE:
		cmd = L"GO_TO_CHANGE";
		break;
	case RTS_STOP:
		cmd = L"STOP";
		break;
	case RTS_REARRANGE:
		cmd = L"REARRANGE";
		break;
	case RTS_IDLE:
		cmd = L"IDLE";
		break;
	default:
		break;
	}

	const auto status = this->getElevatorStatus();

	// Get Current DateTime using localtime_s
	auto ExecuteTime = std::chrono::system_clock::now();
	const time_t ExecuteTime_t = std::chrono::system_clock::to_time_t(ExecuteTime);
	auto DiffSeconds = printTimeDeltaNow();

	// Get Date info from ExecuteTime
	struct tm tstruct;
	localtime_s(&tstruct, &ExecuteTime_t);

	const int ExecuteYear = tstruct.tm_year + 1900;
	const int ExecuteMonth = tstruct.tm_mon + 1;
	const int ExecuteDay = tstruct.tm_mday;
	const int ExecuteHour = tstruct.tm_hour;
	const int ExecuteMinute = tstruct.tm_min;
	const int ExecuteSecond = tstruct.tm_sec;

	const int ExecuteDilation = DiffSeconds;

	const int CurrentFloor = status->CurrentFloor;

	const double Velocity = status->velocity;
	const double Acceleration = status->get_acceleration();
	const double Altimeter = status->altimeter;
	const double TotalMoveDistance = status->total_move_distance;

	const double Erd = status->erd;
	const double Esd = status->esd;
	const double Ed = status->ed;

	const wstring label = L"None";

	RetVal.append(getElevatorStatus()->get_building_name() + L",");
	RetVal.append(getElevatorStatus()->get_device_name() + L",");
	RetVal.append(std::to_wstring(ExecuteTime_t) + L",");
	RetVal.append(std::to_wstring(ExecuteYear) + L",");
	RetVal.append(std::to_wstring(ExecuteMonth) + L",");
	RetVal.append(std::to_wstring(ExecuteDay) + L",");
	RetVal.append(std::to_wstring(ExecuteHour) + L",");
	RetVal.append(std::to_wstring(ExecuteMinute) + L",");
	RetVal.append(std::to_wstring(ExecuteSecond) + L",");
	RetVal.append(std::to_wstring(ExecuteDilation) + L",");
	RetVal.append(cmd + L",");
	RetVal.append(std::to_wstring(CurrentFloor) + L",");
	RetVal.append(std::to_wstring(Velocity) + L",");
	RetVal.append(std::to_wstring(Altimeter) + L",");
	RetVal.append(std::to_wstring(TotalMoveDistance) + L",");
	RetVal.append(std::to_wstring(Erd) + L",");
	RetVal.append(std::to_wstring(Esd) + L",");
	RetVal.append(std::to_wstring(Ed) + L",");
	RetVal.append(label + L"\n");

	return RetVal;
}

void elevatorAlgorithmDefault::write_csv_body(std::wstring str)
{
	thisLogger.write_csv(str);
}

void elevatorAlgorithmDefault::appendLogToLogList(int code, ...)
{
	va_list args;
	va_start(args, code);

	switch (code)
	{
	case IDLE:
		this->IDLELog(args);
		break;
	case CALL:
		this->CALLLog(args);
		break;
	case PRESS:
		this->PRESSLog(args);
		break;
	case UNPRESS:
		this->UNPRESSLog(args);
		break;
	case STOP:
		this->STOPLog(args);
		break;
	case MOV:
		this->MOVLog(args);
		break;
	default:
		wcerr << L"LOG CODE ERROR" << endl;
		break;
	}
}

void elevatorAlgorithmDefault::IDLELog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Timestamp
	this->log_list.push_back(thisLogger.IDLELog(args));
}

void elevatorAlgorithmDefault::CALLLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Called Floor
	// arg[4] = Called Floor Direction
	// arg[5] = Timestamp
	this->log_list.push_back(thisLogger.CALLLog(args));
}

void elevatorAlgorithmDefault::PRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Pressed Floor List
	// arg[4] = Timestamp
	this->log_list.push_back(thisLogger.PRESSLog(args));
}

void elevatorAlgorithmDefault::UNPRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Unpressed Floor List
	// arg[4] = Timestamp
	this->log_list.push_back(thisLogger.UNPRESSLog(args));
}

void elevatorAlgorithmDefault::STOPLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Stopped Floor
	// arg[4] = Timestamp
	this->log_list.push_back(thisLogger.STOPLog(args));
}

void elevatorAlgorithmDefault::MOVLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Moving Floor
	// arg[4] = Timestamp
	this->log_list.push_back(thisLogger.MOVLog(args));
}

bool elevatorAlgorithmDefault::checkReachability(elevatorStatus* stats, double current_altimeter, int dest_floor)
{
	// THIS FUNCTION IS FOR CHECKING REACHABILITY OF DESTINATION FLOOR
	// FIRST GET A CURRENT VELOCITY OF ELEVATOR
	// SECOND GET A CURRENT ACCELERATION OF ELEVATOR
	// THIRD GET A ALTITUDE OF DESTINATION FLOOR
	// FOURTH CALCULATE WHETHER ALTIMETER WILL EXCEED DESTINATION ALTITUDE IF IT DECELERATE TO ZERO VELOCITY SINCE NOW
	// IF IT EXCEEDS, RETURN FALSE, ELSE RETURN TRUE

	// GET CURRENT VELOCITY
	double current_velocity = stats->velocity;

	// GET CURRENT ACCELERATION
	double current_acceleration = stats->get_acceleration();

	// GET DESTINATION ALTITUDE
	double dest_altitude = dest_floor > 0 ? stats->get_each_floor_altimeter()[stats->get_underground_floor() + dest_floor - 1] : stats->get_each_floor_altimeter()[stats->get_underground_floor() + dest_floor];

	// CALCULATE WHETHER ALTIMETER WILL EXCEED DESTINATION ALTITUDE IF IT DECELERATE TO ZERO VELOCITY SINCE NOW
	// THIS WILL USE ABS
	double time_to_zero_velocity = current_velocity / current_acceleration;
	double distance = ((current_velocity + 0) / 2) * time_to_zero_velocity;

	if (stats->direction)
	{
		if (current_altimeter > dest_altitude)
		{
			return false;
		}
		else
		{
			if (abs(dest_altitude - current_altimeter) > distance)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		if(current_altimeter < dest_altitude)
		{
			return false;
		}
		else
		{
			if (abs(dest_altitude - current_altimeter) > distance)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	// IF DESTINATION ALTITUDE IS HIGHER THAN CURRENT ALTITUDE + DISTANCE, RETURN TRUE
}

vector<int> elevatorAlgorithmDefault::getNewButtonInsideList(vector<int> prev, vector<int> current)
{
	// return new elem list that is not in prev
	// if prev is empty, return current
	// if current is empty, return empty vector
	// if prev is not empty and current is not empty, return new elem list that is not in prev
	if (prev.empty())
	{
		return current;
	}
	else if (current.empty())
	{
		return vector<int>();
	}
	else
	{
		vector<int> temp;
		for (const auto& elem : current)
		{
			if (find(prev.begin(), prev.end(), elem) == prev.end())
			{
				temp.push_back(elem);
			}
		}
		return temp;
	}

	return vector<int>();
}

vector<int> elevatorAlgorithmDefault::getRemoveButtonInsideList(vector<int> prev, vector<int> current)
{
	// return remove elem list that is in prev but not in current
	// if prev is empty, return empty vector
	// if current is empty, return prev
	// if prev is not empty and current is not empty, return remove elem list that is in prev but not in current
	if (prev.empty())
	{
		return vector<int>();
	}
	else if (current.empty())
	{
		return prev;
	}
	else
	{
		vector<int> temp;
		for (const auto& elem : prev)
		{
			if (find(current.begin(), current.end(), elem) == current.end())
			{
				temp.push_back(elem);
			}
		}
		return temp;
	}

	return vector<int>();
}

void elevatorAlgorithmDefault::writeEnergyLog()
{
	const auto logs = this->log_list;
	const auto logSize = logs.size();
	const auto wirtePath = thisLogger.log_directory_StateCode + L"\\" + thisLogger.log_file_name_for_building_logs;
	const auto temp = this->getElevatorStatus();
	const wstring buildingName = temp->get_building_name();
	const wstring deviceName = temp->get_device_name();

	std::mutex logMutex;

	std::wifstream logFile(wirtePath, ios::app);
	if (logFile) {
		std::wstring fileContent;
		std::getline(logFile, fileContent);

		size_t lastLogEndPos = fileContent.find(L"LOG END " + buildingName + L" " + deviceName);
		logFile.close();

		std::wofstream logFile(wirtePath, ios::app);
		if (lastLogEndPos == std::string::npos) {
			const auto underground_floor = temp->get_underground_floor();
			const auto ground_floor = temp->get_ground_floor();

			const auto max_velocity = temp->get_max_velocity();
			const auto acceleration = temp->get_acceleration();
			const auto jerk = temp->get_jerk();
			const auto door_open_time = temp->door_open_time;

			const auto each_floor_altimeter = temp->get_each_floor_altimeter();

			logFile << L"LOG START " << buildingName << L" " << deviceName << L" " << max_velocity << L" " << acceleration << L" " << jerk << L" " << door_open_time << L" " << underground_floor << L" " << ground_floor << L" [";
			for (int i = 0; i < each_floor_altimeter.size(); i++) {
				logFile << each_floor_altimeter[i];

				if (i != each_floor_altimeter.size() - 1) {
					logFile << L",";
				}
			}
			logFile << L"]";

			// Write this Elevator uses energy calculation
			const auto this_elevator_energy_flag = temp->get_this_elevator_energy_consumption();
			logFile << " " << this_elevator_energy_flag;

			if (this_elevator_energy_flag)
			{
				// Put Additional Information
				const auto IDLE_Power = temp->getIDLEPower();
				const auto Standby_Power = temp->getStandbyPower();
				const auto ISO_Reference_Cycle_Energy = temp->getISOReferenceCycleEnergy();

				logFile << " " << IDLE_Power << " " << Standby_Power << " " << ISO_Reference_Cycle_Energy;
			}

			logFile << std::endl;

			for (const auto& log : logs) {
				logFile << log << std::endl;
			}

			logFile << L"LOG END " << buildingName << L" " << deviceName << std::endl;

			logFile.close();
		}
		else {
			fileContent.insert(lastLogEndPos + 8, L"\n");
			for (const auto& log : logs) {
				fileContent.insert(lastLogEndPos + 9, log + L"\n");
			}
			logFile.seekp(0, std::ios::beg);
			logFile << fileContent;
			logFile.close();
		}
	}
}

void elevatorAlgorithmDefault::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* p)
{
	simulation* sim = p->s;

	//CHANGE V_T Graph
	stats->current_goTo_floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);
	
	stats->direction = p->current_direction;
	stats->tta = p->t_to_max_velocity;
	stats->ttm = p->t_constant_speed;
	stats->ttd = p->t_max_to_zero_deceleration;
	stats->go_to_floor = sim->main_trip_list[0][0];

	it = stats->current_goTo_floor_vector_info->begin();
	stats->current_goTo_Floor_single_info = *it;

	printTimeDeltaWhenRearrange();
	//SEND MODIFY goTo Floor Data To Unreal Engine

	if (ueSock->VisualizationMod == 1)
	{
		this->set_elevator_Status_JSON_STRING();
		ueSock->send_data_to_UE5(this->elevatorStatus_JSON_STRING);
	}

	this->thisFlags->IDLEFlag = false;
}

void elevatorAlgorithmDefault::rearrangeMainTripList(elevatorStatus* stats, physics* p)
{
	int current_floor = p->s->main_trip_list.front()[0];

	//	CHECK IF THIE MAIN TRIP IS FROM BUTTON INSIDE OR OUTSIDE
	if (p->s->main_trip_list.front()[1] == 1)
	{
		// THIS MEANS THAT MAIN TRIP IS FROM BUTTON INSIDE
		auto temp = this->thisElevatorStatus->button_inside;
		temp->erase(remove(temp->begin(), temp->end(), p->s->main_trip_list.front()[0]), temp->end());
	}
	else
	{
		const int target = p->s->main_trip_list.front()[0];
		auto temp = this->thisElevatorStatus->button_outside;
		temp->erase(remove_if(temp->begin(), temp->end(),
			[target](vector<int>& arr) { return arr[0] == target; }),
			temp->end());
	}

	// REMOVE CURRENT TRIP FROM main_trip_list, and set current_goTo_floor_vector_info to empty
	p->s->main_trip_list.erase(p->s->main_trip_list.begin());

	// CHECK IF MAIN TRIP LIST IS EMPTY
	if (!p->s->main_trip_list.empty())
	{
		// IF FRONT FLOOR IS SAME AS CURRENT FLOOR, DELETE THIS TRIP
		if (p->s->main_trip_list.front()[0] == current_floor)
		{
			p->s->main_trip_list.erase(p->s->main_trip_list.begin());
		}
	}

	this->thisElevatorStatus->current_goTo_floor_vector_info->clear();
	this->thisElevatorStatus->current_goTo_Floor_single_info.clear();
}

void elevatorAlgorithmDefault::set_elevator_Status_JSON_STRING()
{
	nlohmann::json jsonObj;
	try
	{
		jsonObj["building_name"] = wstringToString(thisElevatorStatus->get_building_name());
		jsonObj["device_name"] = wstringToString(thisElevatorStatus->get_device_name());

		jsonObj["dilation"] = thisElevatorStatus->dilation;

		jsonObj["acceleration"] = thisElevatorStatus->get_acceleration();
		jsonObj["max_velocity"] = thisElevatorStatus->get_max_velocity();

		jsonObj["underground_floor"] = thisElevatorStatus->get_underground_floor();
		jsonObj["ground_floor"] = thisElevatorStatus->get_ground_floor();

		//set energy consumption to jsonObj
		jsonObj["calculate_this_elevator_energy_consumption"] = thisElevatorStatus->get_this_elevator_energy_consumption();

		if (thisElevatorStatus->get_this_elevator_energy_consumption())
		{
			// Put Energy Consumption Information
			jsonObj["IDLE_Power"] = thisElevatorStatus->getIDLEPower();
			jsonObj["Standby_Power"] = thisElevatorStatus->getStandbyPower();
			jsonObj["ISO_Reference_Cycle_Energy"] = thisElevatorStatus->getISOReferenceCycleEnergy();

			jsonObj["Erd"] = thisElevatorStatus->erd;
			jsonObj["Esd"] = thisElevatorStatus->esd;
			jsonObj["Ed"] = thisElevatorStatus->ed;
		}

		auto revised_altimeter = thisElevatorStatus->get_each_floor_altimeter();
		double lowest_altimeter = abs(revised_altimeter[0]);

		if (revised_altimeter[0] > 0)
		{
			for (auto& elem : revised_altimeter)
			{
				elem -= lowest_altimeter;
			}
		}
		else
		{
			for (auto& elem : revised_altimeter)
			{
				elem += lowest_altimeter;
			}
		}

		jsonObj["each_floor_altimeter"] = revised_altimeter;
		jsonObj["goToFloor"] = thisElevatorStatus->go_to_floor;

		jsonObj["tta"] = thisElevatorStatus->tta;
		jsonObj["ttm"] = thisElevatorStatus->ttm;
		jsonObj["ttd"] = thisElevatorStatus->ttd;

		this->elevatorStatus_JSON_STRING = jsonObj.dump();
	}
	catch (const std::exception& e)
	{
		std::cerr << "JSON Parsing Error" << e.what() << std::endl;
	}
}

void elevatorAlgorithmDefault::printThisElevatorEnergyConsumptionInfos()
{
	// Get this elevator status
	auto temp = this->getElevatorStatus();

	// print erd
	std::wcout << L"Estimated Daily Running Energy Consumption : " << temp->erd << std::endl;

	// print esd
	std::wcout << L"Estimated Standby Energy Consumption : " << temp->esd << std::endl;

	// print ed
	std::wcout << L"Estimated Daily Energy Consumption : " << temp->ed << std::endl;
}

void elevatorAlgorithmDefault::printTimeDeltaWhenSpawn()
{
	char timeString[128];
	time_t timeNow = std::chrono::system_clock::to_time_t(this->worldClock);

	struct tm tstruct;
	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
	
	sprintf_s(timeString, "IN %ls -> %ls Spawned At TIME : %04d/%02d/%02d - %02d:%02d:%02d.%03lld\n",
		this->thisElevatorStatus->get_building_name().c_str(),
		this->thisElevatorStatus->get_device_name().c_str(),
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count()
	);
	//std::puts(timeString);

	std::wstring txt_log_string(timeString, timeString + strlen(timeString));
	std::wstring csv_log_string = make_csv_string(RTS_SPAWN);
	write_log(txt_log_string);
	write_csv_body(csv_log_string);
}

void elevatorAlgorithmDefault::printTimeDeltaWhenRearrange()
{
	char timeString[128];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::duration<double> delta = now - this->worldClock;

	time_t timeNow = std::chrono::system_clock::to_time_t(now);
	auto diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();

	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration)%1000;

	sprintf_s(timeString, "IN %ls -> %ls Rearranged At TIME : %04d/%02d/%02d - %02d:%02d:%02d.%03lld, Time Delta is %lld Seconds\n", 
		this->thisElevatorStatus->get_building_name().c_str(),
		this->thisElevatorStatus->get_device_name().c_str(),
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), diffSeconds
		);
	//puts(timeString);

	// change timeString to wstring
	std::wstring txt_log_string(timeString, timeString + strlen(timeString));
	std::wstring csv_log_string = make_csv_string(RTS_REARRANGE);
	write_log(txt_log_string);
	write_csv_body(csv_log_string);
}

int elevatorAlgorithmDefault::printTimeDeltaNow()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	int diffSeconds = (int)chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();

	return diffSeconds;
}

void elevatorAlgorithmDefault::printTimeDeltaWhenStop()
{
	char timeString[128];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::duration<double> delta = now - this->worldClock;

	time_t timeNow = std::chrono::system_clock::to_time_t(now);
	time_t timeDelta = std::chrono::duration_cast<std::chrono::seconds>(delta).count();

	auto diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();
	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration)%1000;

	//std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Stopped At TIME : ";
	sprintf_s(timeString, "IN %ls -> %ls STOPPED AT TIME : %04d/%02d/%02d - %02d:%02d:%02d.%03lld, Time Delta is %lld Seconds\n",
		this->thisElevatorStatus->get_building_name().c_str(),
		this->thisElevatorStatus->get_device_name().c_str(),
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), diffSeconds
	);
	//puts(timeString);

	std::wstring txt_log_string(timeString, timeString + strlen(timeString));
	std::wstring csv_log_string = make_csv_string(RTS_STOP);
	write_log(txt_log_string);
	write_csv_body(csv_log_string);
}

void elevatorAlgorithmDefault::printTimeDeltaWhenIDLE()
{
	char timeString[1024];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::duration<double> delta = now - this->worldClock;

	time_t timeNow = std::chrono::system_clock::to_time_t(now);
	time_t timeDelta = std::chrono::duration_cast<std::chrono::seconds>(delta).count();

	auto diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();
	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;

	//std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Stopped At TIME : ";
	sprintf_s(timeString, "IN %ls -> %ls IDLE AT TIME : %04d/%02d/%02d - %02d:%02d:%02d.%03lld, TOTAL MOVE DISTANCE : %f Time Delta is %lld Seconds\n",
		this->thisElevatorStatus->get_building_name().c_str(),
		this->thisElevatorStatus->get_device_name().c_str(),
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), 
		this->thisElevatorStatus->total_move_distance,
		diffSeconds
	);
	puts(timeString);

	std::wstring txt_log_string(timeString, timeString + strlen(timeString));
	std::wstring csv_log_string = make_csv_string(RTS_IDLE);
	write_log(txt_log_string);
	write_csv_body(csv_log_string);
}

void elevatorAlgorithmDefault::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	//notification Content Will hold Very New information about Elevator 
	notificationContent* nofi_content = this->thisNotificationContent;
	//elevatorStatus Will hold latest information before updating
	elevatorStatus* thisElevatorStatus = getElevatorStatus();

	flags* thisElevatorFlag = getElevatorFlag();

	physics* p = phy;
	simulation* sim = p->s;
	socket_UnrealEngine* us = ueSock;

	const wstring building_name = this->thisElevatorStatus->get_building_name();
	const wstring device_name = this->thisElevatorStatus->get_device_name();

	// check new button inside or outside information has been changed
	vector<int> new_inside = getNewButtonInsideList(*(thisElevatorStatus->button_inside), nofi_content->button_inside);
	vector<int> remove_inside = getRemoveButtonInsideList(*(thisElevatorStatus->button_inside), nofi_content->button_inside);

	try
	{
		// lock mutex using mtx variable
		std::lock_guard<std::mutex> guard(mtx);

		if (thisElevatorFlag->firstOperation)
		{
			if (!nofi_content->button_inside.empty())
			{
				p->set_initial_elevator_direction(nofi_content->button_inside[0]);
				insideLogic(thisElevatorStatus, p, new_inside, remove_inside);
			}
			else if (nofi_content->button_outside_floor != 0)
			{
				p->set_initial_elevator_direction(nofi_content->button_outside_floor);
				outsideLogic(thisElevatorStatus, p, nofi_content->button_outside_floor, nofi_content->button_outside_direction);
			}

			//std::wcout << "IN " << building_name << " -> " << device_name << " goTo Floor is Changed None to : " << p->s->main_trip_list[0][0] << std::endl;
			wstring log_string = L"IN " + building_name + L" -> " + device_name + L" goTo Floor is Changed None to : " + std::to_wstring(p->s->main_trip_list[0][0]);
			thisLogger.write_log(log_string);

			//CHANGE V_T Graph
			rearrangeVector(this->getElevatorStatus(), ueSock, p);

			if (us->VisualizationMod == 1)
			{
				this->set_elevator_Status_JSON_STRING();
				us->send_data_to_UE5(this->elevatorStatus_JSON_STRING);
			}

			const int delta_second = elevatorAlgorithmDefault::printTimeDeltaNow();
			appendLogToLogList(MOV, 4, building_name, device_name, p->s->main_trip_list[0][0], delta_second);

			//Logging
			thisElevatorFlag->firstOperation = false;
			thisElevatorFlag->isRunning = true;
			thisElevatorFlag->IDLEFlag = false;
		}

		else if (p->s->main_trip_list.empty())
		{

			if (!nofi_content->button_inside.empty())
			{
				p->set_initial_elevator_direction(nofi_content->button_inside[0]);
				insideLogic(thisElevatorStatus, p, new_inside, remove_inside);
			}
			else if (nofi_content->button_outside_floor != 0)
			{
				p->set_initial_elevator_direction(nofi_content->button_outside_floor);
				outsideLogic(thisElevatorStatus, p, nofi_content->button_outside_floor, nofi_content->button_outside_direction);
			}

			if (!thisElevatorStatus->isElevatorStopped)
			{
				rearrangeVector(this->getElevatorStatus(), ueSock, p);
			}
		}

		else
		{
			if (!nofi_content->button_inside.empty())
			{
				int nofi_content_floor = (thisElevatorStatus->button_inside)->empty() ? INT_MAX : p->current_direction ? (thisElevatorStatus->button_inside)->front() : (thisElevatorStatus->button_inside)->back();

				insideLogic(thisElevatorStatus, p, new_inside, remove_inside);

				if (nofi_content_floor != (thisElevatorStatus->button_inside)->front()) {
					if (!thisElevatorStatus->isElevatorStopped)
					{
						rearrangeVector(this->getElevatorStatus(), ueSock, p);
					}
				}
			}
			if (nofi_content->button_outside_floor != 0)
			{
				//OUTSIDE FLOOR IS ADDED ONLY
				bool flag = true;

				//CHECK THIS CALLED FLOOR IS ALREADY EXISTS IN STATUS
				for (const auto& elem : *(thisElevatorStatus->button_outside))
				{
					if (nofi_content->button_outside_floor == elem[0] && nofi_content->button_outside_direction == elem[1])
					{
						flag = false;
						break;
					}
				}
				if (flag)
				{
					int currentDestFloor;
					// ERROR OCCURRS HERE
					if (p->s->main_trip_list.empty())
					{
						currentDestFloor = thisElevatorStatus->go_to_floor;
					}
					else
					{
						currentDestFloor = p->s->main_trip_list[0][0];
					}

					outsideLogic(thisElevatorStatus, p, nofi_content->button_outside_floor, nofi_content->button_outside_direction);

					if (!p->s->main_trip_list.empty() && currentDestFloor != p->s->main_trip_list[0][0])
					{
						if (!thisElevatorStatus->isElevatorStopped)
						{
							rearrangeVector(this->getElevatorStatus(), ueSock, p);
						}
						thisElevatorFlag->IDLEFlag = false;
					}
				}
			}
		}

		thisElevatorFlag->isRunning = true;
		thisElevatorFlag->IDLEFlag = false;
	}
	//CHECK THIS IS FIRST OPERATION
	catch (const std::exception& e)
	{
		std::wcout << "IN " << building_name << " -> " << device_name << " ERROR OCCURR AT : elevatorAlgorithmSingle::updateElevatorTick " << e.what() << std::endl;
	}
}

void elevatorAlgorithmDefault::insideLogic(elevatorStatus* status, physics* phy, vector<int> new_floors, vector<int> remove_floors)
{
	vector<int> change_inside_list;
	// every elem in new_elem must be updated in main_trip_list
	for (const auto elem : new_floors)
	{
		updateMainTripList(status, phy, elem);
		status->button_inside->push_back(elem);
	}

	// if main_trip_list is not empty, set direction
	if (!phy->s->main_trip_list.empty())
	{
		phy->current_direction = phy->set_direction(phy->s->main_trip_list[0][0]);
	}

	// every elem in remove_elem must be removed if and only if 
	// current altimeter is far enough away from its remove elem floor altimeter
	for (auto elem : remove_floors)
	{
		// check if this elem is close enough to current altimeter
		if (getAltimeterDifferenceBetweenTwoFloors(status->get_each_floor_altimeter(), status->get_underground_floor(), elem, status->altimeter) > 0.5 * status->get_acceleration() * pow(status->get_max_velocity() / status->get_acceleration(), 2))
		{
			updateMainTripList(status, phy, elem);

			// remove this elem from button_inside
			status->button_inside->erase(remove(status->button_inside->begin(), status->button_inside->end(), elem), status->button_inside->end());
		}
	}

	const int delta_second = elevatorAlgorithmDefault::printTimeDeltaNow();
	appendLogToLogList(PRESS, 4, status->get_building_name(), status->get_device_name(), *(thisElevatorStatus->button_inside), delta_second);
}

void elevatorAlgorithmDefault::outsideLogic(elevatorStatus* status, physics* phy, int outside_button, bool outside_direction)
{
	// CHECK IF main_trip_list IS EMPTY, IF IT IS EMPTY THEN ADD OUTSIDE BUTTON TO main_trip_list
	if (phy->s->main_trip_list.empty())
	{
		phy->s->add_floor_to_main_trip_list(outside_button, outside_direction, 0);
		status->button_outside->push_back({ outside_button, outside_direction });
		return;
	}

	// CHECK OUTSIDE BUTTON DIRECTION AND STATUS DIRECTION
	if (outside_direction == phy->current_direction)
	{
		// THERE IS A CASE WHEN outside_button FLOOR IS ALREADY IN main_trip_list
		// IN THIS CASE, IT COUNTS BOTH INSIDE AND OUTSIDE BUTTONS BUT NEVER MAKE MAIN TRIP LIST
		// BECAUSE IT IS ALREADY IN main_trip_list AND IT WILL BE REMOVED WHEN ELEVATOR ARRIVES AT THAT FLOOR
		// THIS CASE ONLY HAPPENS WHEN ELEVATOR DIRECTION IS SAME AS OUTSIDE BUTTON DIRECTION
		// IF DIRECTION IS OPPOSITE IT SHOULD BE IN RESERVE TRIP LIST
		// THEN MAIN TRIP LIST WILL BE UPDATED WHEN ELEVATOR STOPS A LAST FLOOR OF CURRENT CALL
		bool flag = false;

		for (const auto elem : phy->s->main_trip_list)
		{
			if (elem[0] == outside_button && elem[1] == outside_direction)
			{
				flag = true;
				break;
			}
		}

		if (flag)
		{
			phy->s->add_floor_to_main_trip_list(outside_button, outside_direction, 0);
		}

		else
		{
			// IN THIS CASE ELEVATOR HAS TO ANALYZE WHETHER IT SHOULD BE IN main_trip_list OR reserved_trip_list
			// IT SHOULD BE IN RESERVED TRIP LIST IF THIS FLOOR CANNOT BE REACHED EVEN IF ELEVATOR DECELERATES TO 0 SINCE NOW
			// OTHER CASES, IT SHOULD BE IN main_trip_list
			bool flag2 = checkReachability(status, status->altimeter, outside_button);

			if (flag2)
			{
				phy->s->add_floor_to_main_trip_list(outside_button, outside_direction, 0);
			}
			else
			{
				if (outside_direction == true)
				{
					phy->s->reserved_trip_list_up.push_back({ outside_button, outside_direction });
				}
				else
				{
					phy->s->reserved_trip_list_down.push_back({ outside_button, outside_direction });
				}
			}
		}
	}
	else
	{
		if (outside_direction == true)
		{
			phy->s->reserved_trip_list_up.push_back({ outside_button, outside_direction });
		}
		else
		{
			phy->s->reserved_trip_list_down.push_back({ outside_button, outside_direction });
		}
	}
	// set p->direction
	phy->current_direction = phy->set_direction(phy->s->main_trip_list[0][0]);

	status->button_outside->push_back({ outside_button, outside_direction });
}

void elevatorAlgorithmDefault::updateMainTripList(elevatorStatus* status, physics* phy, int floor)
{
	try
	{
		// check elem is already in main_trip_list
		bool flag = false;
		for (auto e : phy->s->main_trip_list)
		{
			if (e[0] == floor)
			{
				flag = true;
				break;
			}
		}

		// if flag, then remove elem from main_trip_list
		if (flag)
		{
			// remove elem from main_trip_list
			phy->s->main_trip_list.erase(remove_if(phy->s->main_trip_list.begin(), phy->s->main_trip_list.end(),
				[floor](vector<int>& arr) { return arr[0] == floor; }),
				phy->s->main_trip_list.end());
		}

		else
		{
			phy->s->add_floor_to_main_trip_list(floor, phy->current_direction, 1);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error Occurrs at elevatorAlgorithmDefault::updateMainTripList : " << e.what() << std::endl;
	}
}

string elevatorAlgorithmDefault::wstringToString(const std::wstring& wstr) const
{
	std::string s(wstr.begin(), wstr.end());
	return s;
}

wstring elevatorAlgorithmDefault::stringToWstring(const std::string& str) const
{
	std::wstring s(str.begin(), str.end());
	return s;
}

double elevatorAlgorithmDefault::getAltimeterDifferenceBetweenTwoFloors(vector<double> each_floor_alt, int underground, int dest_floor, double current_altimeter)
{
	double dest_altimeter;
	if (dest_floor > 0)
	{
		dest_altimeter = each_floor_alt[underground + dest_floor - 1];
	}
	else
	{
		dest_altimeter = each_floor_alt[underground + dest_floor];
	}

	return abs(dest_altimeter - current_altimeter);
}

elevatorStatus* elevatorAlgorithmDefault::getElevatorStatus()
{
	return this->thisElevatorStatus;
}

flags* elevatorAlgorithmDefault::getElevatorFlag()
{
	return this->thisFlags;
}

string elevatorAlgorithmDefault::getJSONString()
{
	return this->elevatorStatus_JSON_STRING;
}

void elevatorStatus::set_names(wstring buildingName, wstring deviceName)
{
	this->building_name = buildingName;
	this->device_name = deviceName;
}

void elevatorStatus::set_physical_information(physics* p)
{
	this->tta = p->t_to_max_velocity;
	this->ttm = p->t_constant_speed;
	this->ttd = p->t_max_to_zero_deceleration;

	this->underground_floor = p->info.underground_floor;
	this->ground_floor = p->info.ground_floor;

	this->each_floor_altimeter = p->info.altimeter_of_each_floor;
	this->max_velocity = p->info.max_velocity;	
	this->acceleration = p->info.acceleration;

	this->current_goTo_floor_vector_info = new vector<vector<double>>{};
	this->current_goTo_Floor_single_info = vector<double>{};
}
void elevatorStatus::set_building_name(wstring buildingName)
{
	this->building_name = buildingName;
}
void elevatorStatus::set_device_name(wstring deviceName)
{
	this->device_name = deviceName;
}
wstring elevatorStatus::get_building_name()
{
	return building_name;
}
wstring elevatorStatus::get_device_name()
{
	return device_name;
}

void elevatorStatus::set_underground_floor(int floor)
{
	underground_floor = floor;
}
void elevatorStatus::set_ground_floor(int floor)
{
	ground_floor = floor;
}
void elevatorStatus::set_each_floor_altimeter(vector<double> alt)
{
	each_floor_altimeter = alt;
}
int elevatorStatus::get_underground_floor()
{
	return underground_floor;
}
int elevatorStatus::get_ground_floor()
{
	return ground_floor;
}
vector<double> elevatorStatus::get_each_floor_altimeter()
{
	return each_floor_altimeter;
}

void elevatorStatus::set_max_velocity(double velocity)
{
	this->max_velocity = velocity;
}
void elevatorStatus::set_acceleration(double acceleration)
{
	this->acceleration = acceleration;
}
void elevatorStatus::set_jerk(double jerk)
{
	this->jerk = jerk;
}
double elevatorStatus::get_max_velocity()
{
	return max_velocity;
}
double elevatorStatus::get_acceleration()
{
	return acceleration;
}
double elevatorStatus::get_jerk()
{
	return this->jerk;
}

void elevatorStatus::set_this_elevator_energy_flag(bool flag)
{
	calculate_this_elevator_energy_consumption = flag;
}

bool elevatorStatus::get_this_elevator_energy_consumption()
{
	return calculate_this_elevator_energy_consumption;
}
void elevatorStatus::set_this_elevator_daily_energy_consumption(int sim_mode_delta)
{
	const double night_time_shorten_ratio = 0.66;
	const double one_day = 86400.0;

	measure_time = chrono::system_clock::now();

	// Get time diff in seconds
	auto diff = chrono::duration_cast<chrono::seconds>(measure_time - start_time).count();

	if (sim_mode_delta != 0) 
	{
		diff = sim_mode_delta;
	}

	// Get ratio of diif seconds to 1 day
	double ratio = one_day / diff;

	// Round to .0
	ratio = round(ratio * 100) / 100;

	// Get Estimated Daily Trip Count
	int estimated_daily_trip_count = floor(ratio * number_of_trips * (0.5 *(1 + night_time_shorten_ratio)));

	if (estimated_daily_trip_count < 50) 
	{
		Category = 1;
		IDLE_time_ratio = 0.13;
		StandBy_time_ratio = 0.87;
	}
	else if (estimated_daily_trip_count < 125)
	{
		Category = 2;
		IDLE_time_ratio = 0.23;
		StandBy_time_ratio = 0.77;
	}
	else if (estimated_daily_trip_count < 300)
	{
		Category = 3;
		IDLE_time_ratio = 0.36;
		StandBy_time_ratio = 0.64;
	}
	else if (estimated_daily_trip_count < 750)
	{
		Category = 4;
		IDLE_time_ratio = 0.45;
		StandBy_time_ratio = 0.55;
	}
	else
	{
		Category = 5;
		IDLE_time_ratio = 0.42;
		StandBy_time_ratio = 0.58;
	}

	// Calculate %S constant
	if (Category < 5)
	{
		if (ground_floor + underground_floor == 2)
		{
			S_factor = 1.0;
		}
		else if (ground_floor + underground_floor == 3) 
		{
			S_factor = 0.67;
		}
		else
		{
			S_factor = 0.44;
		}
	}
	else
	{
		if (ground_floor + underground_floor == 2)
		{
			S_factor = 1.0;
		}
		else if (ground_floor + underground_floor == 3)
		{
			S_factor = 0.67;
		}
		else
		{
			S_factor = 0.33;
		}
	}

	// Calulate Load Factor
	double Q_Factor = 0.0;
	switch (Category)
	{
		case 4:
			Q_Factor = 9.0;
			break;
		case 5 :
			Q_Factor = 13.0;
			break;
		default:
			Q_Factor = 7.5;
			break;
	}

	switch (this_elevator_type_info.this_elevator_type)
	{
		case NoCounterBalance:
			load_factor = 1 + (Q_Factor * 0.0071);
			break;

		case CounterBalance:
			if (this_elevator_type_info.type_load_rate <= 0.4) {
				load_factor = 1 - (Q_Factor * 0.0164);
			}
			else
			{
				load_factor = 1 - (Q_Factor * 0.0192);
			}
			break;

		case Hydraulic:
			load_factor = 1 + (Q_Factor * 0.0071);
			break;

		case CounterBalanceWithHydraulic:
			if (this_elevator_type_info.type_load_rate <= 0.35) {
				load_factor = 1 + (Q_Factor * 0.01);
			}
			else
			{
				load_factor = 1 - (Q_Factor * 0.0187);
			}
			break;

		default:
			break;
	}

	const double estimated_daily_running_energy_consumption =
		(estimated_daily_trip_count * S_factor * load_factor * ISO_Reference_Cycle_Energy) / 2.0;

	const double average_travel_distance_per_trip = abs(this->each_floor_altimeter.front() - this->each_floor_altimeter.back()) * S_factor;

	const double average_time_per_trip = 
		average_travel_distance_per_trip / max_velocity +
		max_velocity / acceleration +
		acceleration / jerk +
		door_open_time;

	const double estimated_daily_standing_energy_consumption = 
		(24 - ((estimated_daily_trip_count * average_time_per_trip)/3600)) * 
		(IDLE_Power * IDLE_time_ratio + Standby_Power_5Min * StandBy_time_ratio);

	if (estimated_daily_standing_energy_consumption < 0)
	{
		//cout << "Error : Estimated Daily Standing Energy Consumption is Negative" << endl;
		//cout << "Estimated estimated_daily_trip_count : " << estimated_daily_trip_count << endl;
		//cout << "Estimated average_travel_distance_per_trip : " << average_travel_distance_per_trip << endl;
	}

	const double estimated_daily_energy_consumption = 
		estimated_daily_running_energy_consumption + estimated_daily_standing_energy_consumption;

	erd = estimated_daily_running_energy_consumption;
	esd = estimated_daily_standing_energy_consumption;
	ed = estimated_daily_energy_consumption;
}

vector<double> elevatorStatus::get_this_elevator_daily_energy_consumption()
{
	if (calculate_this_elevator_energy_consumption)
	{
		return vector<double>{this->erd, this->esd, this->ed};
	}
	return vector<double>();
}

double elevatorStatus::getIDLEPower()
{
	return IDLE_Power;
}

double elevatorStatus::getStandbyPower()
{
	return Standby_Power_5Min;
}

double elevatorStatus::getISOReferenceCycleEnergy()
{
	return ISO_Reference_Cycle_Energy;
}

void elevatorStatus::setIDLEPower(double power)
{
	calculate_this_elevator_energy_consumption = power != 0;
	IDLE_Power = power;
}

void elevatorStatus::setStandbyPower(double power)
{
	calculate_this_elevator_energy_consumption = power != 0;
	Standby_Power_5Min = power;
}

void elevatorStatus::setISOReferenceCycleEnergy(double power)
{
	calculate_this_elevator_energy_consumption = power != 0;
	ISO_Reference_Cycle_Energy = power;
}