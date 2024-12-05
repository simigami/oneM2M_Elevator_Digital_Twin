#include "DT_Simulation.h"
#include "config.h"
#include <future>
#include <algorithm>
#include <random>
#include <cctype>
#include <mutex>

std::vector<double> parseStringToFloatVector(const std::string& input) {
    // Step 1: Remove brackets from the string
    std::string cleanedInput = input;
    cleanedInput.erase(std::remove(cleanedInput.begin(), cleanedInput.end(), '['), cleanedInput.end());
    cleanedInput.erase(std::remove(cleanedInput.begin(), cleanedInput.end(), ']'), cleanedInput.end());

    // Step 2: Use stringstream to parse the values
    std::vector<double> doubleVector;
    std::stringstream ss(cleanedInput);
    std::string token;

    while (std::getline(ss, token, ',')) { // Split by comma
        doubleVector.push_back(std::stod(token)); // Convert to float and add to vector
    }

    return doubleVector;
}

dt_simulation::dt_simulation()
{
}

wstring dt_simulation::MakeCSVStringByGatherElevatorInfo(simElevator ThisElevator, const int Timestamp)
{
    const wstring BuildingName = ThisElevator.this_elevator_algorithm->getElevatorStatus()->get_building_name();
    const  wstring ElevatorName = ThisElevator.this_elevator_algorithm->getElevatorStatus()->get_device_name();

    const auto StartTime = ThisElevator.this_elevator_algorithm->getElevatorStatus()->start_time;
    const auto ExecuteTime = std::chrono::system_clock::to_time_t(StartTime + std::chrono::seconds(Timestamp));

    // DO NOT USE localtime because it is deprecated use localtime_s
    struct tm localTime;
    localtime_s(&localTime, &ExecuteTime);
    const int ExecuteYear = localTime.tm_year + 1900;
    const int ExecuteMonth = localTime.tm_mon + 1;
    const int ExecuteDay = localTime.tm_mday;
    const int ExecuteHour = localTime.tm_hour;
    const int ExecuteMinute = localTime.tm_min;
    const int ExecuteSecond = localTime.tm_sec;

    const int ExecuteDilation = Timestamp;

    const double Velocity = ThisElevator.this_elevator_algorithm->getElevatorStatus()->velocity;
    const double Acceleration = ThisElevator.this_elevator_algorithm->getElevatorStatus()->get_acceleration();
    const double Altimeter = ThisElevator.this_elevator_algorithm->getElevatorStatus()->altimeter;

    const double Erd = ThisElevator.energy_consumption_vector.back()[0];
    const double Esd = ThisElevator.energy_consumption_vector.back()[1];
    const double Ed = ThisElevator.energy_consumption_vector.back()[2];

    const wstring label = L"None";

    wstring RetVal = L"";

    RetVal.append(BuildingName + L",");
    RetVal.append(ElevatorName + L",");
    RetVal.append(std::to_wstring(ExecuteTime) + L",");
    RetVal.append(std::to_wstring(ExecuteYear) + L",");
    RetVal.append(std::to_wstring(ExecuteMonth) + L",");
    RetVal.append(std::to_wstring(ExecuteDay) + L",");
    RetVal.append(std::to_wstring(ExecuteHour) + L",");
    RetVal.append(std::to_wstring(ExecuteMinute) + L",");
    RetVal.append(std::to_wstring(ExecuteSecond) + L",");
    RetVal.append(std::to_wstring(ExecuteDilation) + L",");
    RetVal.append(std::to_wstring(Velocity) + L",");
    RetVal.append(std::to_wstring(Altimeter) + L",");
    RetVal.append(std::to_wstring(Erd) + L",");
    RetVal.append(std::to_wstring(Esd) + L",");
    RetVal.append(std::to_wstring(Ed) + L",");
    RetVal.append(label + L"\n");

    return RetVal;
}

vector<string> dt_simulation::split(string input, const char delimiter) 
{
    vector<string> ret;
    long long pos = 0;
    string token = "";
    while((pos = input.find(delimiter)) != string::npos) 
    {
        token = input.substr(0, pos);
        ret.push_back(token);
        input.erase(0, pos + 1);
    }    
    ret.push_back(input);
    return ret;
}

simBuilding::simBuilding()
{
    this->buildingName = L"";

    this->default_start_floor = -5;

    this->default_elevator_max_velocity = 2.5;
    this->default_elevator_max_acceleration = 1.25;

    this->default_elevator_stop_time = 4.0;
}

//TESTING
void dt_simulation::run()
{
    std::unordered_map<std::string, std::string> elevatorMap;

    log_file_path = fileSystem.chooseLog();

    this->buildings = makeInstance2(log_file_path);
    if (this->buildings == nullptr) {
		std::cerr << "Error: buildings is nullptr" << std::endl;
		return;
	}

    ReadAndAddAllTransactions();

    giveAllBuildingTransactions();

    runSimulation();

    WriteAllTransactionsToFile();

    return;
}

std::vector<simBuilding>* dt_simulation::makeInstance(const std::string& fileAddress)
{
    std::vector<simBuilding>* buildings = new std::vector<simBuilding>;
    simBuilding building;
    simElevator* latest_ev = nullptr;
    wstring building_name;
    wstring ev_name;
    std::ifstream file(fileAddress);
    std::string line;

    bool start_flag = false;
    bool ev_flag = false;

    if (!file.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return nullptr;
    }

    while (std::getline(file, line))
    {
        // Read each line
        if (line == "END INFO")
        {
            file.close();
            return buildings;
        }
        if (line == "INFO START")
        {
            start_flag = true;
        }
        else if (line == "INFO END")
		{
			start_flag = false;
            ev_flag = false;
		}
        else if (start_flag == true && ev_flag == false)
        {
            // Seperate the line by space
            std::istringstream iss(line);
            string key, value;

            iss >> key >> value;
            if (key == "BUILDING_NAME")
            {
                simBuilding building;
                building_name = std::wstring(value.begin(), value.end());
                building.buildingName = building_name;

                const wstring TimestampString = building.thisLogger->get_file_name_as_timestamp() + L"_" + building.buildingName;
                building.thisLogger->set_log_directory_Simulation();
                building.thisLogger->log_file_name = TimestampString + L"_TransactionList.txt";
                building.thisLogger->csv_file_name = TimestampString + L"_TransactionList.csv";

				buildings->push_back(building);
			}
			else if (key == "EV_NAME")
			{
                ev_name = std::wstring(value.begin(), value.end());
                simElevator elevator(building_name, ev_name);
				buildings->back().elevators->push_back(elevator);
                elevator.this_elevator_algorithm->thisLogger.set_log_directory_Simulation();
                latest_ev = &buildings->back().elevators->back();
				ev_flag = true;
            }
        }
        else
        {
            std::istringstream iss(line);
            string key, value;

            iss >> key >> value;

            assert(latest_ev != nullptr);

            if (key == "MAX_VELOCITY")
            {
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_max_velocity(std::stod(value));
            }
            else if (key == "ACCELERATION")
            {
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_acceleration(std::stod(value));
            }
            else if (key == "JERK")
            {
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_jerk(std::stod(value));
            }
            else if (key == "DOOR_OPEN_TIME")
            {
                latest_ev->this_elevator_algorithm->getElevatorStatus()->door_open_time = std::stod(value);
            }
            else if (key == "UNDERGROUND_FLOOR")
            {
                latest_ev->undergroundFloor = std::stoi(value);
            }
            else if (key == "GROUND_FLOOR")
            {
                latest_ev->groundFloor = std::stoi(value);
            }
            else if (key == "EACH_FLOOR_ALTIMETER")
            {
                value = value.substr(1, value.size() - 2);
                std::istringstream iss2(value);
                std::string altimeter;

                while (std::getline(iss2, altimeter, ','))
                {
                    latest_ev->each_floor_altimeter.push_back(std::stoi(altimeter));
                }
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_each_floor_altimeter(latest_ev->each_floor_altimeter);
            }
            else if (key == "E_IDLE")
            {
                // Check if value is numeric
                if (std::all_of(value.begin(), value.end(), ::isdigit))
                {
                    latest_ev->this_elevator_algorithm->getElevatorStatus()->setIDLEPower(std::stod(value));
                }
            }
            else if (key == "E_STANDBY")
            {
                // Check if value is numeric
                if (std::all_of(value.begin(), value.end(), ::isdigit))
                {
                    latest_ev->this_elevator_algorithm->getElevatorStatus()->setStandbyPower(std::stod(value));
                }
            }
            else if (key == "E_REF")
            {
                // Check if value is numeric
                if (std::all_of(value.begin(), value.end(), ::isdigit))
                {
                    latest_ev->this_elevator_algorithm->getElevatorStatus()->setISOReferenceCycleEnergy(std::stod(value));
                }

                // if three values are all filled set energy flag to true
                if (latest_ev->this_elevator_algorithm->getElevatorStatus()->getIDLEPower() != 0 && latest_ev->this_elevator_algorithm->getElevatorStatus()->getStandbyPower() != 0 && latest_ev->this_elevator_algorithm->getElevatorStatus()->getISOReferenceCycleEnergy() != 0)
                {
                    latest_ev->this_elevator_algorithm->getElevatorStatus()->set_this_elevator_energy_flag(true);
                }
            }
        }
    }

    throw std::runtime_error("Invalid log file format, must end with 'END INFO'");
}

std::vector<simBuilding>* dt_simulation::makeInstance2(const std::string& fileAddress)
{
    simElevator* latest_ev = nullptr;
    std::ifstream file(fileAddress);
    std::string line;

    const auto Prefix = "LOG START";

    if (!file.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return nullptr;
    }

	while (std::getline(file, line))
	{
		// Read each line
		if (line.find(Prefix)  != string::npos)
		{
			line = line.substr(9, line.size() - 9);

            const char delimeter = ' ';
			vector<string> params =  split(line, delimeter);

			assert(params.size() == 11 || params.size() == 14);

			const wstring BuildingName = string_to_wstring(params[1]);
			const wstring ElevatorName = string_to_wstring(params[2]);
			const double MaxVelocity = std::stod(params[3]);
			const double Acceleration = std::stod(params[4]);
			const double Jerk = std::stod(params[5]);
			const double DoorOpenTime = std::stod(params[6]);
			const int UndergroundFloor = std::stoi(params[7]);
			const int GroundFloor = std::stoi(params[8]);
			const vector<double> EachFloorAltimeter = parseStringToFloatVector(params[9]);

            assert(UndergroundFloor + GroundFloor == EachFloorAltimeter.size());

            float IdlePower = 0.0;
			float StandbyPower = 0.0;
			float ISOReferenceCycleEnergy = 0.0;

            if (params.size() == 14)
            {
				IdlePower = std::stof(params[11]);
				StandbyPower = std::stof(params[12]);
				ISOReferenceCycleEnergy = std::stof(params[13]);
            }

            // Check Building Exsists
			simBuilding thisBuilding = getBuilding(BuildingName);

			if (thisBuilding.buildingName == L"")
			{
                simBuilding building;
                building.buildingName = BuildingName;

                const wstring TimestampString = building.thisLogger->get_file_name_as_timestamp() + L"_" + building.buildingName;
                building.thisLogger->set_log_directory_Simulation();
                building.thisLogger->log_file_name = TimestampString + L"_TransactionList.txt";
                building.thisLogger->csv_file_name = TimestampString + L"_TransactionList.csv";

				buildings->push_back(building);
                thisBuilding = building;
			}

			// Check Elevator Exsists
			simElevator thisElevator = getElevator(thisBuilding, ElevatorName);

			if (thisElevator.elevatorName == L"")
			{
				simElevator elevator(BuildingName, ElevatorName);
				thisElevator = elevator;
				buildings->back().elevators->push_back(elevator);

                thisElevator.this_elevator_algorithm->thisLogger.set_log_directory_Simulation();
                latest_ev = &buildings->back().elevators->back();
                
                latest_ev->groundFloor = GroundFloor;
                latest_ev->undergroundFloor = UndergroundFloor;
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_each_floor_altimeter(EachFloorAltimeter);
                latest_ev->each_floor_altimeter = EachFloorAltimeter;

                latest_ev->this_elevator_algorithm->getElevatorStatus()->setIDLEPower(IdlePower);
                latest_ev->this_elevator_algorithm->getElevatorStatus()->setStandbyPower(StandbyPower);
                latest_ev->this_elevator_algorithm->getElevatorStatus()->setISOReferenceCycleEnergy(ISOReferenceCycleEnergy);
                latest_ev->this_elevator_algorithm->getElevatorStatus()->set_this_elevator_energy_flag(IdlePower == 0 ? false : true);
            }
		}
    }

    return buildings;
}

void dt_simulation::ReadAndAddAllTransactions()
{
    std::wifstream file(log_file_path);
    if (!file.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        std::wistringstream iss(line);
        std::wstring keyword;
        iss >> keyword;

        if (keyword == L"LOG") {
            std::wstring start_or_end, buildingName, elevatorName;
            iss >> start_or_end >> buildingName >> elevatorName;

            if (start_or_end == L"START")
            {
                simBuilding building = getBuilding(buildingName);
                AddTransactionOfThisElevator(building, &file);
			}
        }
    }
    file.close();

    return;
}

void dt_simulation::WriteAllTransactionsToFile() {
    std::wcout << "SIMULATION COMLPETE AND SAVING THE SIMULATION DATA" << std::endl;
    wstring log_string;

    for (auto& building : *buildings)
    {
        log_string = L"";

        log_string.append(L"LOG " + building.buildingName + L" START\n");
        for (const auto& transaction : *building.transactions)
        {
            log_string.append(L"start Floor : " + std::to_wstring(transaction.start_floor) + L"/" + L" destination Floors : ");
            for (const auto& floor : *(transaction.destination_floors))
            {
                log_string.append(std::to_wstring(floor));
                if (floor != transaction.destination_floors->back()) {
                    log_string.append(L",");
                }
            }
            log_string.append(L"/ Timestamp : " + std::to_wstring(transaction.timestamp) + L"\n");
        }
        log_string.append(L"LOG " + building.buildingName + L" END\n");

        building.thisLogger->write_log(log_string);
    }
}

void dt_simulation::PrintAllTransactions()
{
    for (const auto& building : *buildings) {
		std::wcout << L"Building Name : " << building.buildingName << "/";
        for (auto& transaction : *building.transactions)
        {
        std:; std::wcout << L"start Floor : " << transaction.start_floor << "/";
            std::wcout << L"destination Floors : ";
            for (const auto& floor : *(transaction.destination_floors)) 
            {
				std::wcout << floor << L",";
			}
			std::wcout << L"/Timestamp : " << transaction.timestamp << std::endl;
		}
	}
}

simBuilding dt_simulation::getBuilding(const std::wstring& buildingName)
{
    for (const auto& building : *buildings)
    {
        if (building.buildingName == buildingName)
        {
			return building;
		}
	}
    return simBuilding();
}

simElevator dt_simulation::getElevator(const simBuilding ThisBuilding, const std::wstring& buildingName)
{
	for (const auto& elevator : *ThisBuilding.elevators)
	{
		if (elevator.elevatorName == buildingName)
		{
			return elevator;
        }
    }
    return simElevator(L"", L"");
}

void dt_simulation::AddTransactionOfThisElevator(simBuilding this_building, wifstream* file)
{
    std::wstring line;
    while (std::getline(*file, line)) {
		std::wistringstream iss(line);
		std::wstring keyword;
		iss >> keyword;

        if (keyword == L"LOG") {
            break;
        }
        else if (keyword == L"CALL") {
            SimulationLogCALL(this_building, line);
        }
        else if (keyword == L"MOV") {
			SimulationLogMOV(this_building, line);
		}
        else if (keyword == L"STOP") {
			SimulationLogSTOP(this_building, line);
		}
        else if (keyword == L"PRESS") {
			SimulationLogPRESS(this_building, line);
		}
        else if (keyword == L"IDLE") {
			SimulationLogIDLE(this_building, line);
		}
	}
	sortTransactions(this_building);
}

void dt_simulation::SimulationLogCALL(simBuilding this_building, std::wstring line)
{
    std::wistringstream iss(line);
    std::wstring keyword, building_name, device_name, call_floor, call_floor_direction, timestamp_wstr;
    double timestamp;

    iss >> keyword >> building_name >> device_name >> call_floor >> call_floor_direction >> timestamp_wstr;

    // GET ELEVATOR FROM THIS BUILDING
    simElevator* this_elevator = nullptr;
    for (auto& each_elevator : *this_building.elevators) {
        if (each_elevator.elevatorName == device_name) {
			this_elevator = &each_elevator;
            break;
        }
    }

    // APPEND CURRENT_CALLED_FLOOR TO THIS ELEVATOR
    this_elevator->current_called_floors.push_back(std::stoi(call_floor));

	return;
}

void dt_simulation::SimulationLogMOV(simBuilding this_building, std::wstring line)
{
    std::wistringstream iss(line);
	std::wstring keyword, building_name, device_name, mov_floor, timestamp_wstr;
	double timestamp;

	iss >> keyword >> building_name >> device_name >> mov_floor >> timestamp_wstr;

	timestamp = std::stod(timestamp_wstr);

}

void dt_simulation::SimulationLogSTOP(simBuilding this_building, std::wstring line)
{
    std::wistringstream iss(line);
    std::wstring keyword, building_name, device_name, new_transaction_start, timestamp_wstr;
    double timestamp;

    iss >> keyword >> building_name >> device_name >> new_transaction_start >> timestamp_wstr;

    timestamp = std::stod(timestamp_wstr);

    //std::wcout << L"STOP : " << building_name << L" " << device_name << L" " << new_transaction_start << L" " << timestamp << std::endl;

    // STEP1, FIRST, WE HAVE TO CHECK IF THIS STOP IS DEFINED BY CALL LOG
    // THIS CAN BE DONE BY LOOKING CALLED_FLOOR_LIST OF THIS ELEVATOR
    simElevator* this_elevator = nullptr;
    for (auto& each_elevator : *this_building.elevators) {
        if (each_elevator.elevatorName == device_name) {
			this_elevator = &each_elevator;
		}
	}
    bool flag = false;
    assert(this_elevator != nullptr);

    this_elevator->latest_floor = std::stoi(new_transaction_start);

    // IF THIS ELEVATOR IS NOT FOUND, THEN THIS STOP IS NOT DEFINED BY CALL LOG
    for (const auto elem : this_elevator->current_called_floors)
    {
        if (elem == std::stoi(new_transaction_start))
        {
            this_elevator->current_called_floors.erase(std::remove(this_elevator->current_called_floors.begin(), this_elevator->current_called_floors.end(), elem), this_elevator->current_called_floors.end());
		    flag = true;
			break;
        }
	}

    if (flag)
    {
        transaction new_transaction;

        // THERE IS A UNIQUE CASE THAT STOP FLOOR IS ALREADY INSIDE A DESTINATION FLOOR
        for (int i = this_building.transactions->size() - 1; i >= 0; i--)
        {
            if (this_building.transactions->at(i).transaction_owner == this_elevator->elevatorName && *(this_building.transactions->at(i).closed) == false)
            {
                for(auto elem : *(this_building.transactions->at(i).inside_floors))
				{
					if (elem == std::stoi(new_transaction_start))
					{
                        // ADD TO DETINATION FLOOR
                        this_building.transactions->at(i).destination_floors->push_back(std::stoi(new_transaction_start));

                        // ERASE FROM INSIDE FLOOR
                        this_building.transactions->at(i).inside_floors->erase(this_building.transactions->at(i).inside_floors->find(elem));
						
                        new_transaction.start_floor = std::stoi(new_transaction_start);
                        new_transaction.timestamp = timestamp;
                        new_transaction.transaction_owner = device_name;
                        new_transaction.inside_floors = new set<int>(this_building.transactions->at(i).inside_floors->begin(), this_building.transactions->at(i).inside_floors->end());

                        this_building.transactions->push_back(new_transaction);
                        
                        return;
					}
				}
            }
        }

        // THIS MEANS THAT THIS STOP IS CALLED FLOOR STOP
        if(this_elevator->init == true)
		{
            this_elevator->init = false;

			new_transaction.start_floor = std::stoi(new_transaction_start);
			new_transaction.timestamp = timestamp;
			new_transaction.transaction_owner = device_name;
			new_transaction.inside_floors = new set<int>();

			this_building.transactions->push_back(new_transaction);
			return;
		}

        else
        {
            for(int i = this_building.transactions->size() - 1; i >= 0; i--)
            {
                if (this_building.transactions->at(i).transaction_owner == this_elevator->elevatorName && *(this_building.transactions->at(i).closed) == false)
                {
                    *(this_building.transactions->at(i).closed) = true;

                    // IF THIS STOP IS DEFINED BY CALL LOG ERASE ELEM FROM THIS_ELEVATOR->CURRENT_CALLED_FLOORS
                    new_transaction.start_floor = std::stoi(new_transaction_start);
                    new_transaction.timestamp = timestamp;
                    new_transaction.transaction_owner = device_name;
                    new_transaction.inside_floors = new set<int>(this_building.transactions->at(i).inside_floors->begin(), this_building.transactions->at(i).inside_floors->end());

                    this_building.transactions->push_back(new_transaction);

                    break;
                }
            }
        }
    }
    else
    {
        // THIS MEANS THAT THIS STOP IS ONE OF A DESTINATION POINT OF THIS ELEVATOR CURRENT TRANSACTION
        for(int i = this_building.transactions->size() - 1; i >= 0; i--)
        {
            if (this_building.transactions->at(i).transaction_owner == this_elevator->elevatorName && this_building.transactions->at(i).inside_floors->size() != 0)
            {
                // CHECK IF THERE IS new_transaction_start IN SET OF INSIDE_FLOORS
                for (auto elem : *(this_building.transactions->at(i).inside_floors))
                {
                    if (elem == std::stoi(new_transaction_start))
                    {
						this_building.transactions->at(i).inside_floors->erase(this_building.transactions->at(i).inside_floors->find(elem));
                        this_building.transactions->at(i).destination_floors->push_back(std::stoi(new_transaction_start));
                        this_building.transactions->at(i).end_timestamp = timestamp;
                        
                        break;
					}
				}
			}
		}
    }
}

void dt_simulation::SimulationLogPRESS(simBuilding this_building, std::wstring line)
{
	std::wistringstream iss(line);
	std::wstring keyword, building_name, device_name, press_floor_list, timestamp_wstr;
    vector<int> pressed_floors;
	double timestamp;

	iss >> keyword >> building_name >> device_name >> press_floor_list >> timestamp_wstr;

    std::wstring cleaned_list = press_floor_list.substr(1, press_floor_list.size() - 2);

    std::wistringstream iss2(cleaned_list);
    std::wstring token;
    while (std::getline(iss2, token, L',')) {
        // Convert the token to integer and add to the vector
        pressed_floors.push_back(std::stoi(token));
    }

	timestamp = std::stod(timestamp_wstr);

    simElevator* this_elevator = nullptr;
    for (auto& each_elevator : *this_building.elevators) {
        if (each_elevator.elevatorName == device_name) {
            this_elevator = &each_elevator;
        }
    }
    
    // THERE IS A UNIQUE CASE THAT PRESS LOG APEARES AFTER IDLE LOG
    // or PRESS Appears without CALL State
    if (this_elevator->init == true)
    {
        this_elevator->init = false;
        transaction new_transaction;

        new_transaction.start_floor = this_elevator->init_floor;
        new_transaction.timestamp = timestamp;
        new_transaction.transaction_owner = device_name;
        new_transaction.inside_floors = new set<int>();

        for(const auto& elem : pressed_floors)
		{
			new_transaction.inside_floors->insert(elem);
		}

        this_building.transactions->push_back(new_transaction);
        /*
        else
        {
            for (int i = this_building.transactions->size() - 1; i >= 0; i--)
            {
                if (this_building.transactions->at(i).transaction_owner == this_elevator->elevatorName && *(this_building.transactions->at(i).idle_with_zero_trip) == true)
                {
                    *(this_building.transactions->at(i).idle_with_zero_trip) = false;
                    // APPEND new_transaction_start TO THIS transaction
                    for (auto& elem : pressed_floors)
                    {
                        this_building.transactions->at(i).inside_floors->insert(elem);
                    }

                    break;
                }
            }
        }
        */
        this_elevator->init = false;
        return;
    }

    for (int i = this_building.transactions->size() - 1; i >= 0; i--)
    {
        if (this_building.transactions->at(i).transaction_owner == this_elevator->elevatorName && *(this_building.transactions->at(i).closed) == false)
        {
            // APPEND new_transaction_start TO THIS transaction
            for (auto& elem : pressed_floors)
            {
                this_building.transactions->at(i).inside_floors->insert(elem);
            }

            break;
        }
    }
    return;
}

void dt_simulation::SimulationLogIDLE(simBuilding this_building, std::wstring line)
{
	std::wistringstream iss(line);
	std::wstring keyword, building_name, device_name, timestamp_wstr;
	double timestamp;

	iss >> keyword >> building_name >> device_name >> timestamp_wstr;

	timestamp = std::stod(timestamp_wstr);

	//std::wcout << L"IDLE : " << building_name << L" " << device_name << L" " << timestamp << std::endl;

    // STEP 0. if back is NULL add new_transaction_start to this_building.transactions
    if (this_building.transactions->size() == 0) 
    {
        std::wcerr << "ERROR OCCURRED ON dt_simulation::SimulationLogIDLE : LOG CANNOT BE IDLE IF THERE IS NO TRANSACTIONS" << std::endl;
        return;
    }

    simElevator* this_elevator = nullptr;
    for (auto& each_elevator : *this_building.elevators) {
        if (each_elevator.elevatorName == device_name) {
            this_elevator = &each_elevator;
        }
    }

    this_elevator->current_called_floors.clear();

    this_elevator->init = true;
    this_elevator->init_floor = this_elevator->latest_floor;

    transaction latest_transaction = this_building.transactions->back();

    if (latest_transaction.destination_floors->size() == 0)
    {
        // STEP 2. POP latest_transaction from this_building.transactions
        *(latest_transaction.idle_with_zero_trip) = true;
        return;
    }
    else
    {
        *(latest_transaction.closed) = true;
        latest_transaction.inside_floors->clear();

        return;
    }
}

bool compareTransactions(const transaction& a, const transaction& b)
{
	return a.timestamp < b.timestamp;
}

bool compareTimestamps(const UE5Transaction& a, const UE5Transaction& b)
{
    return a.timestamp < b.timestamp;
}

void dt_simulation::sortTransactions(simBuilding this_building)
{
    if (this_building.transactions->size() > 0)
    {
		std::sort(this_building.transactions->begin(), this_building.transactions->end(), compareTransactions);
	}
}

string dt_simulation::set_elevator_Status_JSON_STRING(simBuilding this_building, simElevator ThisElevator, UE5Transaction each_timestamp)
{
    nlohmann::json jsonObj;

    string building_name(this_building.buildingName.begin(), this_building.buildingName.end());
    string device_name(each_timestamp.owner.begin(), each_timestamp.owner.end());

    jsonObj["building_name"] = building_name;
    jsonObj["device_name"] = device_name;

    jsonObj["acceleration"] = this_building.default_elevator_max_acceleration;
    jsonObj["max_velocity"] = this_building.default_elevator_max_velocity;

    jsonObj["underground_floor"] = ThisElevator.undergroundFloor;
	jsonObj["ground_floor"] = ThisElevator.groundFloor;

    auto revised_altimeter = ThisElevator.each_floor_altimeter;
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
    jsonObj["goToFloor"] = each_timestamp.goTo_floor;

    jsonObj["tta"] = each_timestamp.tta;
    jsonObj["ttm"] = each_timestamp.ttm;
    jsonObj["ttd"] = each_timestamp.ttd;

    const string json_string = jsonObj.dump();

    return json_string;
}

void dt_simulation::runSimulation() {
    // EACH THREAD WILL RUN sendAllBuildingTransactions FUNCTION FOR EACH BUILDING
    std::vector<std::future<void>> futures;
    std::mutex mutex;

    int world_time_dilation = 0;
    int visualtion_mode = 0;

    // ASK IN CONSOLE HOW FAST DOES THE SIMULATION RUN(WORLD TIME DILATION)
    std::wcout << L"PLEASE ENTER SIMULATION SPEED (1.0 = REAL TIME, MAX = 10x) : ";
    std::cin >> world_time_dilation;

    // CHECK VARIABLE
    while (world_time_dilation < 1 || world_time_dilation > 10)
    {
		std::wcout << L"PLEASE ENTER SIMULATION SPEED (1.0 = REAL TIME, MAX = 10x) : ";
		std::cin >> world_time_dilation;
	}

    // ASK IN CONSOLE IF USER WANTS TO SEE VISUALIZATION
    std::wcout << L"DO YOU WANT TO SEE VISUALIZATION? (1 = YES, 0 = NO) : ";
    std::cin >> visualtion_mode;

    // CHECK VARIABLE
    while (visualtion_mode < 0 || visualtion_mode > 1)
    {
        std::wcout << L"PLEASE PRESS 1 or 0 (1 = YES, 0 = NO) : ";
        std::cin >> visualtion_mode;
    }

    // EACH THREAD WILL BE DETACHED
    for (auto& building : *buildings) 
    {
        futures.push_back(std::async(std::launch::async, &dt_simulation::sendAllBuildingTransactions, this, &building, &mutex, world_time_dilation, visualtion_mode));
    }

    // Wait for all threads to finish
    for (auto& future : futures) 
    {
        future.wait();
    }
}

void dt_simulation::send_data(std::string request_content)
{
#ifdef UE5_SERVER_ADDR
    boost::asio::io_context context;
    boost::asio::ip::tcp::socket UE5AcceptSocket(context);
    bool isAcceptSocket = false;

    try
    {
        boost::asio::ip::tcp::resolver resolver(context);

        // Create a resolver to resolve the server address
        std::string server_ip = UE5_SERVER_ADDR;
        std::string server_port = std::to_string(UE5_SERVER_LISTEN_PORT);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(boost::asio::ip::tcp::v4(), server_ip, server_port);

        // Create a socket and connect to the server
        boost::asio::connect(UE5AcceptSocket, endpoints);

        boost::asio::write(UE5AcceptSocket, boost::asio::buffer(request_content + "\n")); // "\n"을 추가하여 메시지 종료를 명시
        //boost::asio::async_write(socket, boost::asio::buffer(json), handle_write);

        // RESPONSE 잘 받지 못하는 경우에 생김
        boost::asio::streambuf buffer;
        boost::asio::read_until(UE5AcceptSocket, buffer, '\n');

        // Extract response content
        std::istream response_stream(&buffer);
        std::string response_content;
        std::getline(response_stream, response_content);

        // Check response content and exit function if successful
        if (response_content == "BuildingDataNULL")
        {
            std::cout << "FROM UE5 TO SERVER -> Error : Building Data NULL\n\n";
            return;
        }
        if (response_content == "Received")
        {
            std::cout << "FROM UE5 TO SERVER -> DATA SEND AND RECEIVED COMPLETE\n\n";
            return;
        }

    }
    catch (const boost::system::system_error& e)
    {
        std::cerr << "FROM SERVER TO UE5 -> ERROR : " << e.what() << std::endl;
    }
#endif
}

void dt_simulation::sendAllBuildingTransactions(simBuilding* each_building, std::mutex* this_mutex, int dilation, int visualtion_mode) {
    double count = 0.0;
    bool FirstFlag = true;

    for (const UE5Transaction& each_transaction : *each_building->timestamp_for_each_floor) {
        if (visualtion_mode)
        {
            while (count < each_transaction.timestamp) {
                // SET TIMER HERE using chrno
                chrono::steady_clock::time_point start = chrono::steady_clock::now();

                // SET TIMER HERE
                chrono::steady_clock::time_point end = chrono::steady_clock::now();

                // GET TIMER DELTA
                const auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); // Calculate the execution time
                auto remaining = (MILLISECONDHUNDRED / dilation - execution_time.count());

                // Sleep delta seconds
                if (remaining > 0.0)
                {
                    Sleep(remaining);
                }
                count += 0.1;
                //std::wcout << L"COUNT SECOND : " << count << std::endl;
            }
            // SET TIMER HERE using chrno
            chrono::steady_clock::time_point start = chrono::steady_clock::now();

            // get this elevator from this building
            calculateEnergyConsumption(each_building, each_transaction);

            // find this elevator from this building
            simElevator* thisElevator = nullptr;
            for (auto& each_elevator : *each_building->elevators) {
                if (each_elevator.elevatorName == each_transaction.owner) {
                    thisElevator = &each_elevator;
                }
            }

            thisElevator->this_elevator_algorithm->getElevatorStatus()->go_to_floor = each_transaction.goTo_floor;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->tta = each_transaction.tta;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->ttm = each_transaction.ttm;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->ttd = each_transaction.ttd;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->dilation = dilation;

            this_mutex->lock();
            thisElevator->this_elevator_algorithm->set_elevator_Status_JSON_STRING();
            send_data(thisElevator->this_elevator_algorithm->getJSONString());
            wstring log_string = L"TRANSACTION AT : " + std::to_wstring(each_transaction.timestamp) + L" Seconds BY BUILDING : " + each_building->buildingName + L" EV : " + each_transaction.owner + L" DEST FLOOR : " + std::to_wstring(each_transaction.goTo_floor) + 
                L"E_IDLE : " + std::to_wstring(thisElevator->energy_consumption_vector.back()[0]) +
                L"E_STANDBY : " + std::to_wstring(thisElevator->energy_consumption_vector.back()[1]) + 
                L"E_ISO : " + std::to_wstring(thisElevator->energy_consumption_vector.back()[2]) + 
                L"\n";

            thisElevator->this_elevator_algorithm->thisLogger.write_log(log_string);
            std::wcout << log_string;
            this_mutex->unlock();

            // SET TIMER HERE
            chrono::steady_clock::time_point end = chrono::steady_clock::now();

            // GET TIMER DELTA
            const auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); // Calculate the execution time
            auto remaining = (MILLISECONDHUNDRED / dilation - execution_time.count());

            // Sleep delta seconds
            if (remaining > 0.0)
            {
                Sleep(remaining);
            }
        }

        else
        {
            // get this elevator from this building
            calculateEnergyConsumption(each_building, each_transaction);

            // find this elevator from this building
            simElevator* thisElevator = nullptr;
            for (auto& each_elevator : *each_building->elevators) {
                if (each_elevator.elevatorName == each_transaction.owner) {
                    thisElevator = &each_elevator;
                }
            }

            thisElevator->this_elevator_algorithm->getElevatorStatus()->go_to_floor = each_transaction.goTo_floor;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->tta = each_transaction.tta;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->ttm = each_transaction.ttm;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->ttd = each_transaction.ttd;
            thisElevator->this_elevator_algorithm->getElevatorStatus()->dilation = dilation;

            this_mutex->lock();
            wstring log_string = L"TRANSACTION AT : " + std::to_wstring(each_transaction.timestamp) + L" Seconds BY BUILDING : " + each_building->buildingName + L" EV : " + each_transaction.owner + L" DEST FLOOR : " + std::to_wstring(each_transaction.goTo_floor) + L"\n";
            wstring CSVString = MakeCSVStringByGatherElevatorInfo(*thisElevator, each_transaction.timestamp);
            
            if (FirstFlag)
            {
                thisElevator->this_elevator_algorithm->thisLogger.write_sim_csv_header();
				FirstFlag = false;
            }
            thisElevator->this_elevator_algorithm->thisLogger.write_log(log_string);
            thisElevator->this_elevator_algorithm->thisLogger.write_csv(CSVString);
            std::wcout << log_string;
            this_mutex->unlock();
        }
	}
}

void dt_simulation::calculateEnergyConsumption(simBuilding* thisBuilding, UE5Transaction each_transaction)
{
    simElevator* this_elevator = nullptr;
    for (auto& each_elevator : *thisBuilding->elevators) {
        if (each_elevator.elevatorName == each_transaction.owner && each_elevator.this_elevator_algorithm->getElevatorStatus()->get_this_elevator_energy_consumption() == true)
        {
            // SET ELEVATOR STATUS
            this_elevator = &each_elevator;
            break;
        }
    }

    if (this_elevator)
    {
        if (this_elevator->latest_floor == 0)
        {
            this_elevator->latest_floor = thisBuilding->default_start_floor;
            this_elevator->will_reach_floor = each_transaction.goTo_floor;
        }

        auto alg = this_elevator->this_elevator_algorithm;
        elevatorStatus* stats = alg->getElevatorStatus();

        stats->number_of_trips += 1;

        // get latest floor
        int latest_floor = this_elevator->latest_floor;

        // get current floor
        int current_floor = this_elevator->will_reach_floor;

        // Calculate Altimeter diff between latest -> current
        double altimeter_diff = getDisatanceBetweenTwoFloors(*thisBuilding, *this_elevator, latest_floor, current_floor);

        // Add diff to total move distance
        this_elevator->move_distance += altimeter_diff;

        stats->total_move_distance += this_elevator->move_distance;

        // Calculate Energy Consumption
        stats->set_this_elevator_daily_energy_consumption(stats->total_move_distance);

        vector<double> energy_consumption = stats->get_this_elevator_daily_energy_consumption();

        // Add to this vector to elevator vector
        this_elevator->energy_consumption_vector.push_back(energy_consumption);
    }
    else
    {
        this_elevator->energy_consumption_vector.push_back({0,0,0});
    }
}

void dt_simulation::giveAllBuildingTransactions() {
    for (const auto& building : *buildings) {
		giveElevatorTransaction(building);
	}
}

const int dt_simulation::setSimulationAlgorithm(simBuilding this_building)
{
    int algorithm = 0;
    std::wcout << L"Please Choose A Simulation Algorithm For Building : " << this_building.buildingName << endl;
    std::wcout << L"1. Default Simulation Algorithm" << endl;
    std::wcout << L"2. Random Simulation Algorithm" << endl;
    std::wcout << L"3. Give Lowest Trip Count Elevator First" << endl;

    wcin >> algorithm;

    while (algorithm < 1 || algorithm > 3)
    {
		wcout << L"Please Choose A Simulation Algorithm For Building : " << this_building.buildingName << endl;
		wcout << L"1. Default Simulation Algorithm" << endl;
		wcout << L"2. Random Simulation Algorithm" << endl;
        wcout << L"3. Give Lowest Trip Count Elevator First" << endl;

		wcin >> algorithm;
	}
    
	return algorithm;
}

void dt_simulation::giveElevatorTransaction(simBuilding this_building) {
    const int algorithm_number = setSimulationAlgorithm(this_building);

    if (this_building.transactions->size() == 0) 
    {
        return;
    }

    for (transaction each_transaction : *this_building.transactions) {
        if(each_transaction.destination_floors->size() == 0)
        {
            continue;
        }

        const int timestamp_of_this_transaction = each_transaction.timestamp;

        // REALLOCATION ALL ELEVATOR POSITIONS TO timestamp_of_this_transaction
        reallocateAllElevatorOfThisBuilding(this_building, timestamp_of_this_transaction);
        SimulationWithAlgorithm(algorithm_number, &this_building, each_transaction);
	}

    reallocateAllElevatorOfThisBuilding(this_building, INT_MAX);

    // SET EACH ELEVATOR timestamp_for_each_floor
    for (auto& each_elevator : *this_building.elevators) {
        //CHECK IF THERE IS NO PREVIOUS TRANSACTION
        if (each_elevator.previous_transactions.empty())
		{
			continue;
		}

        int elevator_current_floor = this_building.default_start_floor;

		for(const auto& each_transaction : each_elevator.previous_transactions)
		{
            UE5Transaction new_ue5_transaction;
            int goTo_floor = each_transaction.start_floor;
            int timestamp = each_transaction.timestamp;
            
            double tta = 0.0;
            double ttm = 0.0;
            double ttd = 0.0;
            double time_between_two_floors;

            // ADD current floor to this transaction start floor
            time_between_two_floors = getTimeBetweenTwoFloors(this_building, each_elevator, elevator_current_floor, goTo_floor);
            elevator_current_floor = goTo_floor;

            if (time_between_two_floors > 2 * (this_building.default_elevator_max_velocity / this_building.default_elevator_max_acceleration)) {
                tta = this_building.default_elevator_max_velocity / this_building.default_elevator_max_acceleration;
                tta = round(tta * 10) / 10;
                ttd = tta;
                ttm = time_between_two_floors - tta - ttd;
            }
            else {
                tta = time_between_two_floors / 2;
                tta = round(tta * 10) / 10;
                ttd = tta;
                ttm = 0.0;
            }
            new_ue5_transaction = { goTo_floor, timestamp , tta, ttm, ttd, each_transaction.transaction_owner };
            this_building.timestamp_for_each_floor->push_back(new_ue5_transaction);
            timestamp += (int)round(time_between_two_floors);
            timestamp += this_building.default_elevator_stop_time;

			for(const auto& each_destination_floor : *each_transaction.destination_floors)
			{
                goTo_floor = each_destination_floor;

                time_between_two_floors = getTimeBetweenTwoFloors(this_building, each_elevator, elevator_current_floor, goTo_floor);
                if (time_between_two_floors > 2 * (this_building.default_elevator_max_velocity / this_building.default_elevator_max_acceleration)) {
                    tta = this_building.default_elevator_max_velocity / this_building.default_elevator_max_acceleration;
                    tta = round(tta * 10) / 10;
                    ttd = tta;
                    ttm = time_between_two_floors - tta - ttd;
                }
                else {
					tta = time_between_two_floors/2;
                    tta = round(tta * 10) / 10;
					ttd = tta;
					ttm = 0.0;
                }
                new_ue5_transaction = { goTo_floor, timestamp , tta, ttm, ttd, each_transaction.transaction_owner };
                this_building.timestamp_for_each_floor->push_back(new_ue5_transaction);

                timestamp += (int)round(time_between_two_floors);
                timestamp += this_building.default_elevator_stop_time;

                elevator_current_floor = goTo_floor;
			}
		}
    }

    // SORT timestamp_for_each_floor based on [1] index value
    std::sort(this_building.timestamp_for_each_floor->begin(), this_building.timestamp_for_each_floor->end(), compareTimestamps);
}

void dt_simulation::reallocateAllElevatorOfThisBuilding(simBuilding this_building, int timestamp) {

    for (simElevator& elevator : *this_building.elevators) {
		// REALLOCATE ELEVATOR POSITION TO timestamp
        // STEP 0. CHEKC IF ELEVATOR HAS TRANSACTION
        if (elevator.current_transaction.empty()) {
			continue;
		}

        // STEP 1. Get Last Transaction of This Elevator
        transaction last_transaction = elevator.current_transaction.back();
        const int laset_transaction_end_timestamp = last_transaction.end_timestamp;

        // STEP 2-1. If Argument Timestamp is Smaller than Last Transaction Timestamp, Move All Transactions to previous_transactions
        if (timestamp >= laset_transaction_end_timestamp) {
            elevator.previous_transactions.insert(elevator.previous_transactions.end(), elevator.current_transaction.begin(), elevator.current_transaction.end());
            
            // STEP 2-2. Clear All Transactions
            elevator.current_transaction.clear();

            continue;
        }

        // STEP 3. IF Argument Timestamp is Smaller than Last Transaction Timestamp, It means Elevator is Moving
        // STEP 4. IF STEP 3, GET transaction that is currently running
        transaction current_transaction;
        vector<transaction>::iterator current_transaction_iterator = elevator.current_transaction.begin();
        for (const auto& transaction : elevator.current_transaction)
        {
            if (timestamp < transaction.end_timestamp) {
                if(current_transaction_iterator > elevator.current_transaction.begin()){
                    current_transaction_iterator--;
                }
                break;
			}
            current_transaction = transaction;
            current_transaction_iterator++;
		}
        // STEP 5. IF STEP 4, Append All Transactions that are prior of current_transaction to previous_transaction using iterator
        if (current_transaction_iterator != elevator.current_transaction.begin()) {
            elevator.previous_transactions.insert(elevator.previous_transactions.end(), elevator.current_transaction.begin(), current_transaction_iterator);
		}
        // STEP 6. Erase All Transactions that are prior of current_transaction using iterator
        if (current_transaction_iterator != elevator.current_transaction.begin()) {
			elevator.current_transaction.erase(elevator.current_transaction.begin(), current_transaction_iterator);
        }
    }
}

void dt_simulation::SimulationWithAlgorithm(const int alg_num, simBuilding* this_building, transaction tran)
{
    switch (alg_num)
    {
	case 1:
		SimulationAlgorithmDefault(this_building, tran);
		break;
	case 2:
		SimulationAlgorithmRandom(this_building, tran);
		break;
    case 3:
        SimulationAlgorithmShortestTransactionFirst(this_building, tran);
        break;
	default:
		SimulationAlgorithmDefault(this_building, tran);
		break;
	}
}

void dt_simulation::SimulationAlgorithmDefault(simBuilding* this_building, transaction tran)
{
    srand(time(NULL));

    std::random_device rd;
    std::mt19937 gen(rd());

    vector<int> IDLE_elevator_indexes;
    simElevator* chosen_elevator = nullptr;

    for (int i = 0; i < this_building->elevators->size(); i++)
    {
        if (this_building->elevators->at(i).current_transaction.empty())
        {
            IDLE_elevator_indexes.push_back(i);
        }
    }

    if (IDLE_elevator_indexes.size() > 0)
    {
        // Return Random IDLE Elevator
        const int max_index = IDLE_elevator_indexes.size();
        std::uniform_int_distribution<int> dist(0, max_index - 1);
        const int random_index = IDLE_elevator_indexes[dist(gen)];
        chosen_elevator = &this_building->elevators->at(random_index);
    }

    else
    {
        
        double closest_delta_altimeter = 0.0;
        int index = -1;
        // // STEP 1. GET LAST DESTINATION FLOOR of EACH ELEVATOR TRANSACTION
        for (int i = 0; i < this_building->elevators->size(); i++)
        {
            simElevator elevator = this_building->elevators->at(i);
            const int underGroundFloor = elevator.undergroundFloor;

            int last_destination_floors = elevator.current_transaction.back().destination_floors->back();
            double last_destination_altimeter = 0.0;
            double transaction_start_altimeter = 0.0;
            // CHANGE FLOOR TO ALTIMETER By Using each_floor_altimeter

            if (last_destination_floors < 0) {
                // UNDERGROUND FLOOR
                last_destination_altimeter = elevator.each_floor_altimeter.at(last_destination_floors + underGroundFloor);
            }
            else if (last_destination_floors >= 1) {
                // ABOVEGROUND FLOOR
                last_destination_altimeter = elevator.each_floor_altimeter.at(last_destination_floors + underGroundFloor - 1);
            }

            if (tran.start_floor < 0)
            {
                // UNDERGROUND FLOOR
                transaction_start_altimeter = elevator.each_floor_altimeter.at(tran.start_floor + underGroundFloor);
            }
            else if (tran.start_floor >= 1)
            {
                // ABOVEGROUND FLOOR
                transaction_start_altimeter = elevator.each_floor_altimeter.at(tran.start_floor + underGroundFloor - 1);
            }

            // STEP 2. GET DELTA ALTIMETER OF LAST DESTINATION FLOOR and THIS TRANSACTION FLOOR
            const double delta_altimeter = abs(last_destination_altimeter - transaction_start_altimeter);

            // STEP 3. GET MINIMUM DELTA ALTIMETER Elevator
            if (index == -1 || delta_altimeter < closest_delta_altimeter)
            {
                closest_delta_altimeter = delta_altimeter;
                index = i;
            }
        }

        chosen_elevator = &this_building->elevators->at(index);
    }

    // CHECK IF CHOSEN ELEVATOR IS NULL
    tran.transaction_owner = chosen_elevator->elevatorName;

    // STEP 4. CALCULATE END TIME WHEN IT REACHES TO END OF DESTINATION FLOOR
    // STEP 4-1. FIRST, WE HAVE TO CALCULATE TIME BETWEEN END DEST FLOOR TO TRANSACTION START FLOOR
    // IF THRERE IS NO PREVIOUS TRANSACTION, START FLOOR WILL BE DEFAULT START FLOOR
    int current_elevator_floor;
    if (chosen_elevator->current_transaction.empty() && chosen_elevator->previous_transactions.empty())
    {
        current_elevator_floor = this_building->default_start_floor;
        tran.end_timestamp = 0.0;
    }
    else if (chosen_elevator->current_transaction.empty())
    {
        current_elevator_floor = chosen_elevator->previous_transactions.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }
    else
    {
        tran.timestamp = chosen_elevator->current_transaction.back().end_timestamp;
        current_elevator_floor = chosen_elevator->current_transaction.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }
    tran.end_timestamp += getTimeBetweenTwoFloors(*this_building, *chosen_elevator, current_elevator_floor, tran.start_floor);
    tran.end_timestamp += this_building->default_elevator_stop_time;

    // STEP 4-2. SECOND, WE HAVE TO CALCULATE TIME BETWEEN EACH TRANSACTION's START FLOOR TO END DEST FLOOR
    int start_floor = tran.start_floor;
    for (const auto& each_destination_floor : *tran.destination_floors)
    {
        const double time_to_reach_destination = getTimeBetweenTwoFloors(*this_building, *chosen_elevator, start_floor, each_destination_floor);
        tran.end_timestamp += time_to_reach_destination;

        // ADD STOP TIME
        tran.end_timestamp += this_building->default_elevator_stop_time;

        // SET START FLOOR TO DESTINATION FLOOR
        start_floor = each_destination_floor;
    }
    tran.end_timestamp += this_building->default_elevator_stop_time;
    tran.end_timestamp = round(tran.end_timestamp * 10) / 10;

    chosen_elevator->current_transaction.push_back(tran);
}
void dt_simulation::SimulationAlgorithmRandom(simBuilding* this_building, transaction tran)
{
    srand(time(NULL));

    std::random_device rd;
    std::mt19937 gen(rd());

    vector<int> IDLE_elevator_indexes;
    simElevator* chosen_elevator = nullptr;

    for (int i = 0; i < this_building->elevators->size(); i++)
    {
        if (this_building->elevators->at(i).current_transaction.empty())
        {
            IDLE_elevator_indexes.push_back(i);
        }
    }

    if (IDLE_elevator_indexes.size() > 0)
    {
        // Return Random IDLE Elevator
        const int max_index = IDLE_elevator_indexes.size();
        std::uniform_int_distribution<int> dist(0, max_index - 1);
        const int random_index = IDLE_elevator_indexes[dist(gen)];
        chosen_elevator = &this_building->elevators->at(random_index);
    }
    else
    {
        const int elevator_count = this_building->elevators->size();
        std::uniform_int_distribution<int> dist(0, elevator_count - 1);
        const int random_index = dist(gen);

        chosen_elevator = &this_building->elevators->at(random_index);
    }

    // CHECK IF CHOSEN ELEVATOR IS NULL
    tran.transaction_owner = chosen_elevator->elevatorName;

    // STEP 4. CALCULATE END TIME WHEN IT REACHES TO END OF DESTINATION FLOOR
    // STEP 4-1. FIRST, WE HAVE TO CALCULATE TIME BETWEEN END DEST FLOOR TO TRANSACTION START FLOOR
    // IF THRERE IS NO PREVIOUS TRANSACTION, START FLOOR WILL BE DEFAULT START FLOOR
    int current_elevator_floor;
    if (chosen_elevator->current_transaction.empty() && chosen_elevator->previous_transactions.empty())
    {
        current_elevator_floor = this_building->default_start_floor;
        tran.end_timestamp = 0.0;
    }
    else if (chosen_elevator->current_transaction.empty())
    {
        current_elevator_floor = chosen_elevator->previous_transactions.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }
    else
    {
        tran.timestamp = chosen_elevator->current_transaction.back().end_timestamp;
        current_elevator_floor = chosen_elevator->current_transaction.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }

	if(current_elevator_floor != tran.start_floor)
	{
		tran.end_timestamp += getTimeBetweenTwoFloors(*this_building, *chosen_elevator, current_elevator_floor, tran.start_floor);
		tran.end_timestamp += this_building->default_elevator_stop_time;
	}

    // STEP 4-2. SECOND, WE HAVE TO CALCULATE TIME BETWEEN EACH TRANSACTION's START FLOOR TO END DEST FLOOR
    int start_floor = tran.start_floor;
    for (const auto& each_destination_floor : *tran.destination_floors)
    {
        const double time_to_reach_destination = getTimeBetweenTwoFloors(*this_building, *chosen_elevator, start_floor, each_destination_floor);
        tran.end_timestamp += time_to_reach_destination;

        // ADD STOP TIME
        tran.end_timestamp += this_building->default_elevator_stop_time;

        // SET START FLOOR TO DESTINATION FLOOR
        start_floor = each_destination_floor;
    }
    tran.end_timestamp += this_building->default_elevator_stop_time;
    tran.end_timestamp = round(tran.end_timestamp * 10) / 10;

    chosen_elevator->current_transaction.push_back(tran);
}

void dt_simulation::SimulationAlgorithmShortestTransactionFirst(simBuilding* this_building, transaction tran)
{
    srand(time(NULL));

    std::random_device rd;
    std::mt19937 gen(rd());

    vector<int> shortest_elevator_indexes;
    simElevator* chosen_elevator = nullptr;

    // GET Elevator that has minimum TRIP COUNT
    int min_trip_count = INT_MAX;

    for (int i = 0; i < this_building->elevators->size(); i++)
    {
        simElevator elevator = this_building->elevators->at(i);
        if (min_trip_count == INT_MAX)
        {
            min_trip_count = elevator.current_transaction.size() + elevator.previous_transactions.size();
            shortest_elevator_indexes.push_back(i);
        }
        else if (elevator.current_transaction.size() + elevator.previous_transactions.size() == min_trip_count)
        {
            shortest_elevator_indexes.push_back(i);
        }
        else if (elevator.current_transaction.size() + elevator.previous_transactions.size() < min_trip_count)
        {
            min_trip_count = elevator.current_transaction.size() + elevator.previous_transactions.size();
            shortest_elevator_indexes.clear();
            shortest_elevator_indexes.push_back(i);
        }
    }
    // SET chosen elevator to random elevator
    std::uniform_int_distribution<int> dist(0, shortest_elevator_indexes.size() - 1);
    const int random_index = shortest_elevator_indexes[dist(gen)];
    chosen_elevator = &this_building->elevators->at(random_index);

    // CHECK IF CHOSEN ELEVATOR IS NULL
    tran.transaction_owner = chosen_elevator->elevatorName;

    // STEP 4. CALCULATE END TIME WHEN IT REACHES TO END OF DESTINATION FLOOR
    // STEP 4-1. FIRST, WE HAVE TO CALCULATE TIME BETWEEN END DEST FLOOR TO TRANSACTION START FLOOR
    // IF THRERE IS NO PREVIOUS TRANSACTION, START FLOOR WILL BE DEFAULT START FLOOR
    int current_elevator_floor;
    if (chosen_elevator->current_transaction.empty() && chosen_elevator->previous_transactions.empty())
    {
        current_elevator_floor = this_building->default_start_floor;
        tran.end_timestamp = 0.0;
    }
    else if (chosen_elevator->current_transaction.empty())
    {
        current_elevator_floor = chosen_elevator->previous_transactions.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }
    else
    {
        tran.timestamp = chosen_elevator->current_transaction.back().end_timestamp;
        current_elevator_floor = chosen_elevator->current_transaction.back().destination_floors->back();
        tran.end_timestamp = tran.timestamp;
    }
    tran.end_timestamp += getTimeBetweenTwoFloors(*this_building, *chosen_elevator, current_elevator_floor, tran.start_floor);
    tran.end_timestamp += this_building->default_elevator_stop_time;

    // STEP 4-2. SECOND, WE HAVE TO CALCULATE TIME BETWEEN EACH TRANSACTION's START FLOOR TO END DEST FLOOR
    int start_floor = tran.start_floor;
    for (const auto& each_destination_floor : *tran.destination_floors)
    {
        const double time_to_reach_destination = getTimeBetweenTwoFloors(*this_building, *chosen_elevator, start_floor, each_destination_floor);
        tran.end_timestamp += time_to_reach_destination;

        // ADD STOP TIME
        tran.end_timestamp += this_building->default_elevator_stop_time;

        // SET START FLOOR TO DESTINATION FLOOR
        start_floor = each_destination_floor;
    }
    tran.end_timestamp += this_building->default_elevator_stop_time;
    tran.end_timestamp = round(tran.end_timestamp * 10) / 10;

    chosen_elevator->current_transaction.push_back(tran);
}


double dt_simulation::getDisatanceBetweenTwoFloors(simBuilding this_building, simElevator ThisElevator, int start_floor, int end_floor)
{
    double start_altitude = 0.0;
    double end_altitude = 0.0;

    if (start_floor < 0) 
    {
        start_altitude = ThisElevator.each_floor_altimeter[start_floor + ThisElevator.undergroundFloor];
    }
    else 
    {
        start_altitude = ThisElevator.each_floor_altimeter[start_floor + ThisElevator.undergroundFloor - 1];
    }

    if (end_floor < 0) 
    {
        end_altitude = ThisElevator.each_floor_altimeter[end_floor + ThisElevator.undergroundFloor];
    }
    else 
    {
        end_altitude = ThisElevator.each_floor_altimeter[end_floor + ThisElevator.undergroundFloor - 1];
    }

    return abs(start_altitude - end_altitude);
}

double dt_simulation::getTimeBetweenTwoFloors(simBuilding this_building, simElevator ThisElevator, int start_floor, int end_floor) {
    // GET ALTITUDE OF START FLOOR if start_floor < 0, it means underground floor, and if start_floor > 0, it means aboveground floor
    double start_altitude = 0.0;
    if (start_floor < 0) 
    {
		start_altitude = ThisElevator.each_floor_altimeter[start_floor + ThisElevator.undergroundFloor];
	}
    else 
    {
		start_altitude = ThisElevator.each_floor_altimeter[start_floor + ThisElevator.undergroundFloor - 1];
    }

    // GET ALTITUDE OF END FLOOR if end_floor < 0, it means underground floor, and if end_floor > 0, it means aboveground floor
    double end_altitude = 0.0;
    if (end_floor < 0) {
        end_altitude = ThisElevator.each_floor_altimeter[end_floor + ThisElevator.undergroundFloor];
    }
    else {
		end_altitude = ThisElevator.each_floor_altimeter[end_floor + ThisElevator.undergroundFloor - 1];
	}

    // GET MAX VELOCITY OF ELEVATOR
    double max_velocity = this_building.default_elevator_max_velocity;

    // GET MAX ACCELERATION OF ELEVATOR
    double max_acceleration = this_building.default_elevator_max_acceleration;

    // GET Zero To Max Acceleration Time with .2 round
    double t_to_max_velocity = round(max_velocity / max_acceleration * 10) / 10;

    // Calculate distance covered during acceleration phase
    double distance_covered_during_acceleration = 0.5 * max_acceleration * pow(t_to_max_velocity, 2);

    // CHECK IF start_altitude to end_altitude can reach Max Velocity
    // IF Two Altimeter is Close, Elevator will exceed end altimeter if it reaches Max Velocity
    // IF Two Altimeter is Far, Elevator will reach Max Velocity
    double distance_between_two_altimeter = abs(start_altitude - end_altitude);

    if (2 * distance_covered_during_acceleration > distance_between_two_altimeter)
    {
        double total_time = sqrt((4* distance_between_two_altimeter) / max_acceleration);
        
        // round to .2
        total_time = round(total_time * 10) / 10;
        
        return total_time;
	}
    else
    {
		// Elevator will reach Max Velocity
        // Then getTimeBetweenTwoFloors will be acceleration time + constant speed time + deceleration time(which is same as acceleration time)
        double t_constant_speed = (distance_between_two_altimeter - 2 * distance_covered_during_acceleration) / max_velocity;
        return t_to_max_velocity + t_constant_speed + t_to_max_velocity;
	}

    return 0.0;
}

simElevator* dt_simulation::findIDLEElevator(vector<simElevator>* this_building_elevators, transaction this_transaction) 
{
    // IDLE Elevators
    vector<simElevator>* IDLE_elevators = new vector<simElevator>();

    for (auto& elevator : *this_building_elevators)
    {
        if (elevator.current_transaction.empty())
        {
			IDLE_elevators->push_back(elevator);
		}
	}

    if (IDLE_elevators->size() > 0)
    {
		// Return Random IDLE Elevator
        const int max_index = IDLE_elevators->size() - 1;
        const int random_index = rand() % max_index;

        return &IDLE_elevators->at(random_index);
	}

	// RETURN NEAREST ELEVATOR
	return nullptr;
}

simElevator* dt_simulation::findNearestElevator(simElevator ThisElevator, vector<simElevator>* this_building_elevators, transaction this_transaction) {
    // NEAREST Elevator
    double closest_delta_altimeter = 0.0;
    // // STEP 1. GET LAST DESTINATION FLOOR of EACH ELEVATOR TRANSACTION
    simElevator* return_class = nullptr;

    for (auto& elevator : *this_building_elevators)
    {
        const int last_destination_floors = elevator.current_transaction.begin()->destination_floors->back();
        double last_destination_altimeter = 0.0;
        const int underGroundFloor = ThisElevator.undergroundFloor;
        // CHANGE FLOOR TO ALTIMETER By Using each_floor_altimeter
        
        if (last_destination_floors < 0) {
            // UNDERGROUND FLOOR
			double last_destination_altimeter = elevator.each_floor_altimeter[last_destination_floors + underGroundFloor];
		}
        else {
			// ABOVEGROUND FLOOR
			double last_destination_altimeter = elevator.each_floor_altimeter[last_destination_floors + underGroundFloor -1];
        }

        // STEP 2. GET DELTA ALTIMETER OF LAST DESTINATION FLOOR and THIS TRANSACTION FLOOR
        const double delta_altimeter = abs(last_destination_altimeter - this_transaction.start_floor);

        // STEP 3. GET MINIMUM DELTA ALTIMETER Elevator
        if (closest_delta_altimeter == 0.0 || delta_altimeter < closest_delta_altimeter)
        {
			closest_delta_altimeter = delta_altimeter;
			return_class = &elevator;
		}
    }

    return return_class;
}

bool dt_simulation::bBuildingExist(const std::wstring& buildingName)
{
	for (const auto& building : *buildings)
	{
		if (building.buildingName == buildingName)
		{
			return true;
		}
	}
    return false;
}

simElevator::simElevator(wstring building_name, wstring device_name)
{
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    this->this_elevator_algorithm = new elevatorAlgorithmDefault(building_name, device_name, start);
    this->elevatorName = device_name;
    this->current_velocity = 0.0;
    this->current_altimeter = 0.0;
    this->move_distance = 0.0;
    this->current_transaction = vector<transaction>();
    this->previous_transactions = vector<transaction>();
}