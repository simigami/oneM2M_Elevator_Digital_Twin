#include "DT_Simulation.h"
#include "config.h"
#include <algorithm>
#include <random>
#include <cctype>

dt_simulation::dt_simulation()
{
	this->fileLocation = new wchar_t[MAXFILEPATHLENGTH];
	ZeroMemory(&ofn, sizeof(ofn));
}

void dt_simulation::setSimulatedFileLocation()
{
	wchar_t szFile[MAXFILEPATHLENGTH] = { 0 };
    bool isTXTFlag = false;

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    std::wcout << "IN SERVER -> Choose a txt file that has Data for Elevator..." << std::endl;
    Sleep(1000);

    while(!isTXTFlag)
    {
	    // 파일 선택 대화 상자 열기
	    if (GetOpenFileName(&ofn) == TRUE) 
	    {
            if(true)
	        // 사용자가 파일을 선택한 경우 선택된 파일의 경로 출력
            {
	            isTXTFlag = true;
                ofn.lpstrFile = szFile;
            }
            else 
		    {
		        // 사용자가 취소한 경우 메시지 출력
		        std::wcout << L"IN SERVER -> Selected File is not txt, Please Choose a txt file..." << std::endl;
            	Sleep(500);
		    }
	    }
    }

    return;
}

wchar_t dt_simulation::getSimulatedFileLocation() const
{
    return *(this->ofn.lpstrFile);
}

//TESTING
void dt_simulation::run()
{
    std::unordered_map<std::string, std::string> elevatorMap;

    elevatorMap = findLogStart(LOGFILEPATH, elevatorMap);
    this->buildings = createBuildings(elevatorMap);
    ReadAndAddAllTransactions();
    giveAllBuildingTransactions();
    sendAllBuildingTransactions();

    //WriteAllTransactionsToFile();

    return;
}

std::unordered_map<std::string, std::string> dt_simulation::findLogStart(const std::string& fileAddress, std::unordered_map<std::string, std::string>& elevatorMap)
{
    std::ifstream file(fileAddress);
    if (!file.is_open()) {
        std::cerr << "Unable to open the file.\n";
        return elevatorMap;
    }

    std::string line;
    const std::string pattern = "LOG START";

    while (std::getline(file, line)) {
        if (line.find(pattern) != std::string::npos) {
            size_t startPos = line.find(pattern) + pattern.length();

            std::string buildingName, elevatorName, underground_floor, ground_floor, each_floor_altimeter;
            std::istringstream iss(line.substr(startPos));
            iss >> buildingName >> elevatorName >> underground_floor >> ground_floor >> each_floor_altimeter;

            // CHECK IF BUILDING EXISTS
            if (elevatorMap.find(buildingName) == elevatorMap.end()) {
				elevatorMap[buildingName] = underground_floor + " " + ground_floor + " " + each_floor_altimeter + " ";
			}
            else {
                elevatorMap[buildingName] += elevatorName + " ";
            }
        }
    }

    file.close();
    return elevatorMap;
}

std::vector<simBuilding>* dt_simulation::createBuildings(const std::unordered_map<std::string, std::string>& elevatorMap)
{
    std::vector<simBuilding>* buildings = new std::vector<simBuilding>;

    // Iterate through elevatorMap
    for (const auto& pair : elevatorMap) {
        simBuilding building;

        building.buildingName = std::wstring(pair.first.begin(), pair.first.end()); // Convert string to wstring

        // Split elevator names
        std::istringstream iss(pair.second);
        std::string undergroundFloor, abovegroundFloor, each_floor_altimeter;
        std::string elevatorName;
        iss >> undergroundFloor >> abovegroundFloor >> each_floor_altimeter;

        building.undergroundFloor = std::stoi(undergroundFloor);
        building.abovegroundFloor = std::stoi(abovegroundFloor);

        // Split char '[' and ']' with substr
        each_floor_altimeter = each_floor_altimeter.substr(1, each_floor_altimeter.size() - 2);

        // Split each_floor_altimeter [alt1,alt2,alt3, ...]
        std::istringstream iss2(each_floor_altimeter);
        std::string altimeter;

        while (std::getline(iss2, altimeter, ',')) {
            building.each_floor_altimeter.push_back(std::stoi(altimeter));
		}

        while (iss >> elevatorName) {
            simElevator elevator;
            elevator.elevatorName = std::wstring(elevatorName.begin(), elevatorName.end()); // Convert string to wstring

            // Add elevator to building
            building.elevators->push_back(elevator);
        }

        // Add building to buildings vector
        buildings->push_back(building);
    }

    return buildings;
}

void dt_simulation::run2()
{
    vector<wstring> TCs;
    wstring TCName;

    std::wcout << L"Please Enter a TestCase Name, This Name Will Be Your AE Name : ";
    wcin >> TCName;

	std::wcout << L"Your TestCase Name is \"" << TCName << "\"" << std::endl;

    bool flag = false;
    while(!flag)
    {
        if(find(this->TCNames.begin(), this->TCNames.end(), TCName) == TCNames.end())
        {
            TCNames.push_back(TCName);
            oneM2MSocket->socket.acp_create_one_ACP(TCNames, 0);
            oneM2MSocket->socket.ae_create(TCName);
            flag = true;
        }
        else
        {
	        std::wcout << L"Please Enter a TestCase Name, This Name Will Be Your AE Name : ";
			wcin >> TCName;
			std::wcout << L"Your TestCase Name is \"" << TCName << "\"" << std::endl;
        }
    }

    this->setSimulatedFileLocation();
	this->readFileAndCreateoneM2MResource(TCName);

    std::wcout << L"Test Data Complete" << std::endl;
}

void dt_simulation::readFileAndCreateoneM2MResource(wstring TCName)
{
	wchar_t path = getSimulatedFileLocation();
    parse_json parsingClass;
	wstring wline;
    wifstream wfile;

    wfile.open(L"E:\\ML\\Elevator Git\\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\\Resources\\Sensors\\testlog2.txt");

    if (!wfile.is_open()) 
    {
        std::wcerr << L"IN SERVER -> dt_simulation ERROR : Unable to open file." << std::endl;
        return;
    }

    while (getline(wfile, wline)) // Read each line from the file
    {
        Wparsed_struct eachLineParsedStruct = parsingClass.parsingText(wline);
        eachLineParsedStruct.TCName = TCName;

    	oneM2MSocket->init(eachLineParsedStruct);

        auto it = thisTestCaseResourceInfo.find(eachLineParsedStruct.building_name);

        if(it == thisTestCaseResourceInfo.end())
        {
            oneM2MSocket->createBuilding(eachLineParsedStruct);
            thisTestCaseResourceInfo.emplace(make_pair(eachLineParsedStruct.building_name, map<wstring, int>()));
            thisTestCaseResourceInfo[eachLineParsedStruct.building_name].emplace(make_pair(eachLineParsedStruct.device_name, 1));
        }
        else
        {
            auto itit = thisTestCaseResourceInfo[eachLineParsedStruct.building_name].find(eachLineParsedStruct.device_name);
            if(itit == thisTestCaseResourceInfo[eachLineParsedStruct.building_name].end())
			{
            	oneM2MSocket->createElevator(eachLineParsedStruct);
	            thisTestCaseResourceInfo[eachLineParsedStruct.building_name].emplace(make_pair(eachLineParsedStruct.device_name, 1));
			}
            else
            {
            	oneM2MSocket->createNewData(eachLineParsedStruct);
            }
        }
    }

    wfile.close(); // Close the file
}
transaction dt_simulation::getPreviousTransaction(simBuilding this_building) 
{
    auto transactions = *(this_building.transactions);

    if (transactions.size() > 1) 
    {
        return transactions[this_building.transactions->size() - 2];
	}
    else 
    {
        return transaction();
    }
}

void dt_simulation::ReadAndAddAllTransactions()
{
    std::wifstream file(LOGFILEPATH);
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
    wcout << "SIMULATION COMLPETE PLEASE ENTER LOG NAME TO SAVE THE SIMULATION DATA" << std::endl;
    wstring logName;
    const wstring logExt = L".txt";

    wcin >> logName;
    while (logName.empty())
    {
        wcout << "LOG NAME CANNOT BE EMPTY, PLEASE ENTER LOG NAME TO SAVE THE SIMULATION DATA" << std::endl;
        wcin >> logName;
    }

    logName.append(logExt);

    while (wifstream(logName).good())
    {
		wcout << "LOG NAME ALREADY EXISTS, PLEASE ENTER LOG NAME TO SAVE THE SIMULATION DATA" << std::endl;
		wcin >> logName;
		logName.append(logExt);
	}

    wofstream logFile(logName, ios::app);
    for (const auto& building : *buildings)
    {
        logFile << L"LOG " << building.buildingName << " START" << std::endl;
        for (const auto& transaction : *building.transactions)
        {
            logFile << L"start Floor : " << transaction.start_floor << L"/" << L"destination Floors : ";
            for (const auto& floor : *(transaction.destination_floors))
            {
                logFile << floor;
                if (floor != transaction.destination_floors->back()) {
                    logFile << L",";
                }
            }
            logFile << L"/Timestamp : " << transaction.timestamp << std::endl;
        }
        logFile << L"LOG " << building.buildingName << " END" << std::endl;
    }
    logFile.close();

    return;
}

void dt_simulation::PrintAllTransactions()
{
    for (const auto& building : *buildings) {
		std::wcout << L"Building Name : " << building.buildingName << "/";
        for (auto& transaction : *building.transactions)
        {
        std:; wcout << L"start Floor : " << transaction.start_floor << "/";
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

    // IF CALL THERE IS MUST A TRANSACTION
    transaction new_transaction;
    new_transaction.start_floor = std::stoi(call_floor);
	new_transaction.timestamp = std::stod(timestamp_wstr);
	new_transaction.transaction_owner = device_name;

	this_building.transactions->push_back(new_transaction);
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

    // STEP 0. if back is NULL add new_transaction_start to this_building.transactions
    if (this_building.transactions->size() == 0)
    {
        transaction new_transaction;
        new_transaction.start_floor = std::stoi(new_transaction_start);
        new_transaction.timestamp = timestamp;
        new_transaction.transaction_owner = device_name;

        this_building.transactions->push_back(new_transaction);
        return;
    }

    // STEP1, FIRST, WE HAVE TO CHECK IF THIS STOP IS DEFINED BY CALL LOG
    transaction latest_transaction = this_building.transactions->back();
    if (latest_transaction.start_floor == std::stoi(new_transaction_start))
	{
		return;
	}

    transaction new_transaction;
    new_transaction.start_floor = std::stoi(new_transaction_start);
    new_transaction.timestamp = timestamp;
    new_transaction.transaction_owner = device_name;

    // STEP 1. CHECK LATEST TRANSACTION's vector<int> destination_floors length is 0
    latest_transaction = this_building.transactions->back();

    if (latest_transaction.destination_floors->size() == 0)
    {
		// STEP 2. POP latest_transaction from this_building.transactions
        this_building.transactions->pop_back();
        this_building.transactions->push_back(new_transaction);
        return;
	}
    else
    {
		// STEP 3. ADD new_transaction_start to latest_transaction.destination_floors
        this_building.transactions->push_back(new_transaction);
        return;
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

    // CHECK PREVIOUS TRANSACTION
    transaction latest_transaction = this_building.transactions->back();
    transaction previous_transaction = getPreviousTransaction(this_building);

    if (previous_transaction.timestamp == 0.0 || previous_transaction.transaction_owner != latest_transaction.transaction_owner)
    {
        latest_transaction.destination_floors->clear();

        for (const auto& floor : pressed_floors)
        {
            latest_transaction.destination_floors->push_back(floor);
        }

        sort(latest_transaction.destination_floors->begin(), latest_transaction.destination_floors->end());
		return;
	}

    vector<int> non_overlapping_pressed_floors(previous_transaction.destination_floors->size() + pressed_floors.size());
    auto iter = set_difference(pressed_floors.begin(), pressed_floors.end(), previous_transaction.destination_floors->begin(), previous_transaction.destination_floors->end(), non_overlapping_pressed_floors.begin());
    non_overlapping_pressed_floors.erase(iter, non_overlapping_pressed_floors.end());

    latest_transaction.destination_floors->clear();

    for (const auto& floor : non_overlapping_pressed_floors)
    {
        latest_transaction.destination_floors->push_back(floor);
    }

    sort(latest_transaction.destination_floors->begin(), latest_transaction.destination_floors->end());
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

    transaction latest_transaction = this_building.transactions->back();

    if (latest_transaction.destination_floors->size() == 0)
    {
        // STEP 2. POP latest_transaction from this_building.transactions
        this_building.transactions->pop_back();
        return;
    }
    else
    {
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

string dt_simulation::set_elevator_Status_JSON_STRING(simBuilding this_building, UE5Transaction each_timestamp)
{
    nlohmann::json jsonObj;

    string building_name(this_building.buildingName.begin(), this_building.buildingName.end());
    string device_name(each_timestamp.owner.begin(), each_timestamp.owner.end());

    jsonObj["building_name"] = building_name;
    jsonObj["device_name"] = device_name;

    jsonObj["acceleration"] = this_building.default_elevator_max_acceleration;
    jsonObj["max_velocity"] = this_building.default_elevator_max_velocity;

    jsonObj["underground_floor"] = this_building.undergroundFloor;
	jsonObj["ground_floor"] = this_building.abovegroundFloor;

    auto revised_altimeter = this_building.each_floor_altimeter;
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
            std::cout << "FROM UE5 TO SERVER -> Error : Building Data NULL" << std::endl;
            return;
        }
        if (response_content == "Received")
        {
            std::cout << "FROM UE5 TO SERVER -> DATA SEND AND RECEIVED COMPLETE" << std::endl;
            return;
        }

    }
    catch (const boost::system::system_error& e)
    {
        std::cerr << "FROM SERVER TO UE5 -> ERROR : " << e.what() << std::endl;
    }
#endif
}

void dt_simulation::sendAllBuildingTransactions() {
    for (auto& building : *buildings) {
        const auto this_building_transactions = building.timestamp_for_each_floor;

        int count = 0;

       for (const auto& each_transaction : *this_building_transactions) {
           while (count < each_transaction.timestamp) {
               // SET TIMER HERE using chrno
               chrono::steady_clock::time_point start = chrono::steady_clock::now();

               // SET TIMER HERE
               chrono::steady_clock::time_point end = chrono::steady_clock::now();

               // GET TIMER DELTA
               const auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); // Calculate the execution time
               auto remaining = SECOND - execution_time.count();

               // Sleep delta seconds
               if (remaining > 0.0)
               {
                   Sleep(remaining);
               }
               count += 1;
               std::wcout << L"COUNT SECOND : " << count << std::endl;
           }

           std::wcout << L"TRANSACTION OCCURRED AT : " << count << " BY BUILDING : " << building.buildingName << " EV : " << each_transaction.owner << std::endl;
           building.json_string = set_elevator_Status_JSON_STRING(building, each_transaction);
           send_data(building.json_string);
		}
    }
}

void dt_simulation::giveAllBuildingTransactions() {
    for (const auto& building : *buildings) {
		giveElevatorTransaction(building);
	}


}

void dt_simulation::giveElevatorTransaction(simBuilding this_building) {
    std::random_device rd;
    std::mt19937 gen(rd());

    const auto this_building_transactions = this_building.transactions;

    if (this_building.transactions->size() == 0) {
        return;
    }

    for (auto& each_transaction : *this_building.transactions) {
        srand(time(NULL));
        const int timestamp_of_this_transaction = each_transaction.timestamp;

        // REALLOCATION ALL ELEVATOR POSITIONS TO timestamp_of_this_transaction
        reallocateAllElevatorOfThisBuilding(this_building, timestamp_of_this_transaction);

        //simElevator* chosen_elevator = findIDLEElevator(this_building.elevators, each_transaction);

        vector<int> IDLE_elevator_indexes;
        simElevator* chosen_elevator = new simElevator;

        for(int i =0 ; i < this_building.elevators->size(); i++)
		{
            if (this_building.elevators->at(i).current_transaction.empty())
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
            chosen_elevator = &this_building.elevators->at(random_index);
        }

        else 
        {
            const int underGroundFloor = this_building.undergroundFloor;
            double closest_delta_altimeter = 0.0;
            int index = -1;
            // // STEP 1. GET LAST DESTINATION FLOOR of EACH ELEVATOR TRANSACTION
            for (int i = 0; i < this_building.elevators->size(); i++)
            {
                simElevator elevator = this_building.elevators->at(i);

                int last_destination_floors = elevator.current_transaction.back().destination_floors->back();
                double last_destination_altimeter = 0.0;
                double transaction_start_altimeter = 0.0;
                // CHANGE FLOOR TO ALTIMETER By Using each_floor_altimeter

                if (last_destination_floors < 0) {
                    // UNDERGROUND FLOOR
                    last_destination_altimeter = this_building.each_floor_altimeter.at(last_destination_floors + underGroundFloor);
                }
                else if(last_destination_floors >= 1) {
                    // ABOVEGROUND FLOOR
                    last_destination_altimeter = this_building.each_floor_altimeter.at(last_destination_floors + underGroundFloor - 1);
                }

                if(each_transaction.start_floor < 0)
				{
					// UNDERGROUND FLOOR
                    transaction_start_altimeter  = this_building.each_floor_altimeter.at(each_transaction.start_floor + underGroundFloor);
				}
				else if(each_transaction.start_floor >= 1)
				{
					// ABOVEGROUND FLOOR
					transaction_start_altimeter = this_building.each_floor_altimeter.at(each_transaction.start_floor + underGroundFloor - 1);
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

            chosen_elevator = &this_building.elevators->at(index);
        }

        // CHECK IF CHOSEN ELEVATOR IS NULL
        each_transaction.transaction_owner = chosen_elevator->elevatorName;

        // STEP 4. CALCULATE END TIME WHEN IT REACHES TO END OF DESTINATION FLOOR
        // STEP 4-1. FIRST, WE HAVE TO CALCULATE TIME BETWEEN END DEST FLOOR TO TRANSACTION START FLOOR
        // IF THRERE IS NO PREVIOUS TRANSACTION, START FLOOR WILL BE DEFAULT START FLOOR
        int current_elevator_floor;
        if (chosen_elevator->current_transaction.empty() && chosen_elevator->previous_transactions.empty())
		{
            current_elevator_floor = this_building.default_start_floor;
            each_transaction.end_timestamp = 0.0;
		}
		else if (chosen_elevator->current_transaction.empty())
		{
			current_elevator_floor = chosen_elevator->previous_transactions.back().destination_floors->back();
            each_transaction.end_timestamp = each_transaction.timestamp;
		}
		else
		{
			current_elevator_floor = chosen_elevator->current_transaction.back().destination_floors->back();
            each_transaction.end_timestamp = each_transaction.timestamp;
		}
        each_transaction.end_timestamp += getTimeBetweenTwoFloors(this_building, current_elevator_floor, each_transaction.start_floor);
        each_transaction.end_timestamp += this_building.default_elevator_stop_time;

        // STEP 4-2. SECOND, WE HAVE TO CALCULATE TIME BETWEEN EACH TRANSACTION's START FLOOR TO END DEST FLOOR
        int start_floor = each_transaction.start_floor;
        for(const auto& each_destination_floor : *each_transaction.destination_floors)
		{
			const double time_to_reach_destination = getTimeBetweenTwoFloors(this_building, start_floor, each_destination_floor);
			each_transaction.end_timestamp += time_to_reach_destination;

            // ADD STOP TIME
            each_transaction.end_timestamp += this_building.default_elevator_stop_time;

            // SET START FLOOR TO DESTINATION FLOOR
            start_floor = each_destination_floor;
		}
        each_transaction.end_timestamp += this_building.default_elevator_stop_time;
        each_transaction.end_timestamp = round(each_transaction.end_timestamp * 10) / 10;
        
        chosen_elevator->current_transaction.push_back(each_transaction);
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
            int goTo_floor = each_transaction.start_floor;
            int timestamp = each_transaction.timestamp;
            
            double tta = 0.0;
            double ttm = 0.0;
            double ttd = 0.0;

			for(const auto& each_destination_floor : *each_transaction.destination_floors)
			{
                const auto time_between_two_floors = getTimeBetweenTwoFloors(this_building, elevator_current_floor, goTo_floor);
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
                UE5Transaction new_ue5_transaction = { goTo_floor, timestamp , tta, ttm, ttd, each_transaction.transaction_owner };
                this_building.timestamp_for_each_floor->push_back(new_ue5_transaction);

                timestamp += (int)round(time_between_two_floors);
                timestamp += this_building.default_elevator_stop_time;

                elevator_current_floor = goTo_floor;
				goTo_floor = each_destination_floor;
			}

            const auto time_between_two_floors = getTimeBetweenTwoFloors(this_building, elevator_current_floor, goTo_floor);
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

            UE5Transaction new_ue5_transaction = { goTo_floor, timestamp , tta, ttm, ttd, each_transaction.transaction_owner };
            this_building.timestamp_for_each_floor->push_back(new_ue5_transaction);
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

double dt_simulation::getTimeBetweenTwoFloors(simBuilding this_building, int start_floor, int end_floor) {
    // GET ALTITUDE OF START FLOOR if start_floor < 0, it means underground floor, and if start_floor > 0, it means aboveground floor
    double start_altitude = 0.0;
    if (start_floor < 0) {
		start_altitude = this_building.each_floor_altimeter[start_floor + this_building.undergroundFloor];
	}
    else {
		start_altitude = this_building.each_floor_altimeter[start_floor + this_building.undergroundFloor - 1];
    }

    // GET ALTITUDE OF END FLOOR if end_floor < 0, it means underground floor, and if end_floor > 0, it means aboveground floor
    double end_altitude = 0.0;
    if (end_floor < 0) {
        end_altitude = this_building.each_floor_altimeter[end_floor + this_building.undergroundFloor];
    }
    else {
		end_altitude = this_building.each_floor_altimeter[end_floor + this_building.undergroundFloor - 1];
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
		// Elevator will exceed end altimeter if it reaches Max Velocity
		// Then getTimeBetweenTwoFloors will be time to max velocity + time to zero deceleration(which is same as time to max velocity)
		double t_to_max_velocity = sqrt(2 * distance_between_two_altimeter / max_acceleration);
        return t_to_max_velocity + t_to_max_velocity;
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
	return new simElevator;
}

simElevator* dt_simulation::findNearestElevator(simBuilding this_building, vector<simElevator>* this_building_elevators, transaction this_transaction) {
    // NEAREST Elevator
    const int underGroundFloor = this_building.undergroundFloor;
    double closest_delta_altimeter = 0.0;
    // // STEP 1. GET LAST DESTINATION FLOOR of EACH ELEVATOR TRANSACTION
    simElevator* return_class = new simElevator();
    for (auto& elevator : *this_building_elevators)
    {
        const int last_destination_floors = elevator.current_transaction.begin()->destination_floors->back();
        double last_destination_altimeter = 0.0;
        // CHANGE FLOOR TO ALTIMETER By Using each_floor_altimeter
        
        if (last_destination_floors < 0) {
            // UNDERGROUND FLOOR
			double last_destination_altimeter = this_building.each_floor_altimeter[last_destination_floors + underGroundFloor];
		}
        else {
			// ABOVEGROUND FLOOR
			double last_destination_altimeter = this_building.each_floor_altimeter[last_destination_floors + underGroundFloor -1];
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

simElevator::simElevator()
{
    this->elevatorName = L"";
    this->current_velocity = 0.0;
    this->current_altimeter = 0.0;
    this->move_distance = 0.0;
    this->current_transaction = vector<transaction>();
    this->previous_transactions = vector<transaction>();
}

simBuilding::simBuilding()
{
	this->buildingName = L"";
    this->undergroundFloor = 0;
    this->abovegroundFloor = 0;

    this->default_start_floor = -5;

    this->default_elevator_max_velocity = 2.5;
    this->default_elevator_max_acceleration = 1.25;

    this->default_elevator_stop_time = 4.0;
}