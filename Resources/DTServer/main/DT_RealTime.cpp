﻿#include "DT_RealTime.h"
#include "DT_Simulation.h"
#include "config.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"
#include "simulation.h"
#include "elevator.h"

#include <codecvt>
#include <locale>
#include <sstream>

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::vector;
using std::chrono::system_clock;

void print_elapsed_time()
{
	int elapsed = 0;
    while(true)
    {
	    std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;
        std::wcout << "Elapsed Time " << elapsed << "seconds \n";
    }
}

Building::Building(int buttonMod)
{
    this->buildingElevatorInfo = new class_of_one_Building;
    this->buildingElevatorInfo->outsideButtonMod = buttonMod;
	this->currentButtonOutsideInfos = new vector<vector<int>>;
}

int Building::getButtonMod()
{
    return this->buildingElevatorInfo->outsideButtonMod;
}

void Building::getWhichElevatorToGetButtonOutside(vector<vector<int>> button_outside)
{
    Elevator* selectedElevator = NULL;
    Elevator* selectedElevatorIDLE = NULL;
    Elevator* selectedElevatorDifferentDirection = NULL;

    vector<Elevator*> thisBuildingElevators = this->buildingElevatorInfo->classOfAllElevators;

	vector<Elevator*> idleElevators;
    vector<Elevator*> workingSameDirectionElevators;
    vector<Elevator*> workingDifferentDirectionElevators;

    vector<vector<int>> addedButtonOutsideList;

    int calledFloor;
    bool calledDirection;

    double closestDistanceElevatorAltimeter;
    double closestDistanceElevatorAltimeterIDLE;

    //CHECK DIFF BETWEEN current Elevator and Embedded Call
	try
	{
		addedButtonOutsideList = getDiffBetweenCurrentAndEmbedded(button_outside, *(this->currentButtonOutsideInfos));

		if (addedButtonOutsideList.size() == 0)
		{
			std::wcout << "IN SERVER -> NO OUTSIDE CALL" << std::endl;
			*(this->currentButtonOutsideInfos) = button_outside;
			return;
		}

		else if (thisBuildingElevators.size() == 1)
		{
			std::wcout << thisBuildingElevators[0]->getBuildingName() << " -> " << thisBuildingElevators[0]->getDeviceName() << " is SELECTED" << std::endl;
			thisBuildingElevators[0]->sock->create_oneM2M_CIN_Only_Button_Outside(addedButtonOutsideList);
			*(this->currentButtonOutsideInfos) = button_outside;
			return;
		}

		for (vector<int> eachCall : addedButtonOutsideList)
		{
			vector<vector<int>> temp;

			calledFloor = eachCall[0];
			calledDirection = eachCall[1];

			closestDistanceElevatorAltimeter = 0.0;
			closestDistanceElevatorAltimeterIDLE = 0.0;

			for (auto elem : thisBuildingElevators)
			{
				if (elem->algorithmNumber == 1 || elem->algorithmNumber == 2) // SINGLE ALG
				{
					if (elem->thisElevatorAlgorithmSingle->thisFlags->IDLEFlag) // 정지한 엘리베이터들
					{
						double dist = abs(elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) - elem->p->current_altimeter);
						if (closestDistanceElevatorAltimeterIDLE == 0.0)
						{
							closestDistanceElevatorAltimeterIDLE = dist;
							selectedElevatorIDLE = elem;
						}
						else if (dist < closestDistanceElevatorAltimeterIDLE)
						{
							closestDistanceElevatorAltimeterIDLE = dist;
							selectedElevator = elem;
						}
						idleElevators.push_back(elem);
						std::wcout << elem->getBuildingName() << " -> " << elem->getDeviceName() << " IS IN IDLE..." << std::endl;
					}
					else
					{
						if (calledDirection)
						{
							if (!elem->p->lock) // IF CHOSEN ELEVATOR IS STOPPED AT SOME FLOOR
							{

							}
							if (elem->p->current_direction)
							{
								double dist = elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) - 2.5;
								if (elem->p->current_altimeter < dist)
								{
									if (closestDistanceElevatorAltimeter == 0.0)
									{
										closestDistanceElevatorAltimeter = dist - elem->p->current_altimeter;
										selectedElevator = elem;
									}
									else if (dist - elem->p->current_altimeter < closestDistanceElevatorAltimeter)
									{
										closestDistanceElevatorAltimeter = dist - elem->p->current_altimeter;
										selectedElevator = elem;
									}
									workingSameDirectionElevators.push_back(elem);
								}
								else
								{
									workingDifferentDirectionElevators.push_back(elem);
								}
							}
							else
							{
								workingDifferentDirectionElevators.push_back(elem);
							}
						}
						else
						{
							if (!elem->p->current_direction)
							{
								double dist = elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) + 2.5;
								if (elem->p->current_altimeter > dist)
								{
									if (closestDistanceElevatorAltimeter == 0.0)
									{
										closestDistanceElevatorAltimeter = elem->p->current_altimeter - dist;
										selectedElevator = elem;
									}
									else if (elem->p->current_altimeter - dist < closestDistanceElevatorAltimeter)
									{
										closestDistanceElevatorAltimeter = elem->p->current_altimeter - dist;
										selectedElevator = elem;
									}
									workingSameDirectionElevators.push_back(elem);
								}
								else
								{
									workingDifferentDirectionElevators.push_back(elem);
								}
							}
							else
							{
								workingDifferentDirectionElevators.push_back(elem);
							}
						}
					}
				}
			}

			if (selectedElevatorIDLE != NULL)
			{
				std::wcout << selectedElevatorIDLE->getBuildingName() << " -> " << selectedElevatorIDLE->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
				temp.push_back(eachCall);

				const int delta_second = selectedElevatorIDLE->thisElevatorAlgorithmSingle->printTimeDeltaNow();
				selectedElevatorIDLE->thisElevatorAlgorithmSingle->appendLogToLogList(CALL, 5, selectedElevatorIDLE->getBuildingName(), selectedElevatorIDLE->getDeviceName(), calledFloor, calledDirection, delta_second);

				selectedElevatorIDLE->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
			}

			else if (selectedElevator != NULL)
			{
				std::wcout << selectedElevator->getBuildingName() << " -> " << selectedElevator->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
				temp.push_back(eachCall);

				if (calledFloor == 2) {
					std::wcout << "CHECK" << std::endl;
				}

				const int delta_second = selectedElevator->thisElevatorAlgorithmSingle->printTimeDeltaNow();
				selectedElevator->thisElevatorAlgorithmSingle->appendLogToLogList(CALL, 5, selectedElevator->getBuildingName(), selectedElevator->getDeviceName(), calledFloor, calledDirection, delta_second);

				selectedElevator->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
			}

			else
			{
				int diff = 0;
				for (Elevator* elem : thisBuildingElevators)
				{
					int eachFloorLastDestination = (elem->p->s->main_trip_list.back())[0];

					if (diff == 0)
					{
						diff = abs(calledFloor - eachFloorLastDestination);
						selectedElevatorDifferentDirection = elem;
					}
					else if (abs(calledFloor - eachFloorLastDestination) < diff)
					{
						diff = abs(calledFloor - eachFloorLastDestination);
						selectedElevatorDifferentDirection = elem;
					}
				}

				if (selectedElevatorDifferentDirection != NULL)
				{
					std::wcout << selectedElevatorDifferentDirection->getBuildingName() << " -> " << selectedElevatorDifferentDirection->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
					temp.push_back(eachCall);

					const int delta_second = selectedElevatorDifferentDirection->thisElevatorAlgorithmSingle->printTimeDeltaNow();
					selectedElevatorDifferentDirection->thisElevatorAlgorithmSingle->appendLogToLogList(CALL, 5, selectedElevatorDifferentDirection->getBuildingName(), selectedElevatorDifferentDirection->getDeviceName(), calledFloor, calledDirection, delta_second);

					selectedElevatorDifferentDirection->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
				}
				else
				{
					std::wcout << "Error in Building::getWhichElevatorToGetButtonOutside -> selectedElevatorDifferentDirection is NULL" << std::endl;
				}

			}
		}
		*(this->currentButtonOutsideInfos) = button_outside;
	}
	catch (const std::exception& e)
	{
		std::wcout << "Error in Building::getWhichElevatorToGetButtonOutside : " << e.what() << std::endl;
	}
	return;
}

vector<vector<int>> Building::getCurrentElevatorsButtonOutsideLists()
{
    vector<vector<int>> retValue;

    for(Elevator* eachElevator : this->buildingElevatorInfo->classOfAllElevators)
    {
        // eachTrip[1] = 1  -> inside, 0 -> outside
	    for(vector<int> eachTrip : eachElevator->p->s->main_trip_list)
	    {
		    if(eachTrip[1] == 0)
		    {
			    retValue.push_back(eachTrip);
		    }
	    }
        for(vector<int> eachTrip : eachElevator->p->s->reserved_trip_list_up)
	    {
		    if(eachTrip[1] == 0)
		    {
			    retValue.push_back(eachTrip);
		    }
	    }
        for(vector<int> eachTrip : eachElevator->p->s->reserved_trip_list_down)
	    {
		    if(eachTrip[1] == 0)
		    {
			    retValue.push_back(eachTrip);
		    }
	    }
    }

    return retValue;
}

vector<vector<int>> Building::getDiffBetweenCurrentAndEmbedded(vector<vector<int>> newList, vector<vector<int>> oldList)
{
	vector<vector<int>> newElements; // 'current'에만 있는 새로운 원소들
	vector<vector<int>> deletedElements; // 'call'에만 있는 삭제된 원소들

    int i = 0;
	int j = 0;

	while (i < newList.size() && j < oldList.size()) {
    	if (newList[i][0] == oldList[j][0])
        {
            // 첫 번째 원소가 같다면, 동일한 원소이므로 다음 원소로 넘어간다
            i++;
            j++;
        }
		else if (newList[i][0] < oldList[j][0]) 
        {
            // old 값이 new보다 크다면 어떤 층 값이 새롭게 생긴 것
            newElements.push_back(newList[i]);
            i++;
        }
		else 
        {
            // call[j]가 current[i]보다 작다면, call[j]는 삭제된 원소이다
            deletedElements.push_back(oldList[j]);
            j++;
        }
    }

    // 남아있는 원소들 처리
    while (i < newList.size()) 
    {
        newElements.push_back(newList[i]);
        i++;
    }
    while (j < oldList.size()) 
    {
        deletedElements.push_back(oldList[j]);
        j++;
    }

    // 결과 벡터 준비
    return newElements;
}

dt_real_time::dt_real_time() : parsed_struct()
{
	this->httpRequest = new std::wstring;
	this->noti_content = new notificationContent;
}

bool dt_real_time::existsElevator(Building* one_building, const wstring& device_name)
{
    try
	{
		for(const Elevator* elem : one_building->buildingElevatorInfo->classOfAllElevators)
	    {
		    if(elem->getDeviceName() == device_name)
		    {
			    return true;
		    }
	    }
        return false;
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on dt_real_time::get_elevator : " << e.what() << std::endl;
        exit(0);
	}
}

string dt_real_time::get_RN_Path_From_SubscriptionRI(const string& substring)
{
#ifdef oneM2M_ACME
	std::ifstream file(DEFAULT_SUB_RI_FILE_NAME);
	if (!file.is_open()) {
		std::cerr << "Error: Unable to open file " << DEFAULT_SUB_RI_FILE_NAME << std::endl;
		return ""; // Return empty string if file cannot be opened
	}

	std::string line;
	std::string result;

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string string1;

		// Read string1 (the first string separated by spaces)
		ss >> string1;

		// Check if string1 contains the substring
		if (string1.find(substring) != std::string::npos) {
			// If substring exists in string1, return the rest of the line (string2)
			std::getline(ss, result);
			return result;
		}
	}
#endif // oneM2M_ACME
	return ""; // Return empty string if no matching string1 found
}

Building* dt_real_time::get_building_vector(const wstring& ACOR_NAME)
{
    for(size_t i = 0 ; i < this->allBuildingInfo.size() ; i++)
    {
        if(this->allBuildingInfo[i]->buildingElevatorInfo->ACP_NAME == ACOR_NAME)
	    {
		    return this->allBuildingInfo[i];
	    }
    }
    return NULL;
}

Elevator* dt_real_time::getElevator(Building* class_of_one_building, const wstring& device_name)
{
    try
	{
        for(size_t i = 0; i < class_of_one_building->buildingElevatorInfo->classOfAllElevators.size(); i++)
        {
	        if(class_of_one_building->buildingElevatorInfo->classOfAllElevators[i]->getDeviceName() == device_name)
	        {
		        return class_of_one_building->buildingElevatorInfo->classOfAllElevators[i];
	        }
        }
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on dt_real_time::get_elevator : " << e.what() << std::endl;
        std::exit(0);
	}

	return NULL;
}

socket_oneM2M dt_real_time::get_oneM2M_socket_based_on_AE_ID(vector<Elevator*> elevator_array, const wstring& AE_ID)
{
    for(const Elevator* each_elevator_class : elevator_array)
    {
        socket_oneM2M socket = *(each_elevator_class->sock);
	    if(socket.building_name == AE_ID)
	    {
            return socket;
	    }
    }
    std::wcout << "Error occurred in Class dt_real_time::get_oneM2M_socket_based_on_AE_ID -> AE_ID Not Found : " << AE_ID << std::endl;
    exit(0);
}

void dt_real_time::Run()
{
    boost::asio::io_context ioContext;

    tcp::endpoint EMBEDDED_END(tcp::v4(), EMBEDDED_LISTEN_PORT);
    tcp::endpoint NOTIFICATION_END(tcp::v4(), oneM2M_NOTIFICATION_LISTEN_PORT);

    tcp::acceptor acceptor1(ioContext, EMBEDDED_END, EMBEDDED_LISTEN_PORT);
    tcp::acceptor acceptor2(ioContext, NOTIFICATION_END, oneM2M_NOTIFICATION_LISTEN_PORT);
    //std::thread timerThread(print_elapsed_time);

    acceptor1.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::wcout << "Accepted connection on port " << PORT_EMBEDDED << std::endl;
            handleConnection(socket, EMBEDDED_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << EMBEDDED_LISTEN_PORT << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor1,  ioContext);
    });

    acceptor2.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::wcout << "Accepted connection on port " << PORT_NOTIFICATION << std::endl;
            handleConnection(socket, oneM2M_NOTIFICATION_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << oneM2M_NOTIFICATION_LISTEN_PORT << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor2,  ioContext);
    });
    ioContext.run();
}

void dt_real_time::startAsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::wcout << "Accepted connection on port " << PORT_EMBEDDED << std::endl;
            handleConnection(socket, EMBEDDED_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << EMBEDDED_LISTEN_PORT << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor, ioContext);
    });
}

void dt_real_time::startAsyncAccept2(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::wcout << "Accepted connection on port " << PORT_NOTIFICATION << std::endl;
            handleConnection(socket, oneM2M_NOTIFICATION_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << oneM2M_NOTIFICATION_LISTEN_PORT << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor, ioContext);
    });
}

void dt_real_time::handleConnection(boost::asio::ip::tcp::socket& socket, int port)
{
    boost::asio::streambuf receiveBuffer(BUFFER_SIZE);
    boost::system::error_code error;
    string httpRequest;

	try
	{
		if (port == oneM2M_NOTIFICATION_LISTEN_PORT)
		{
			//boost::asio::read_until(socket, receiveBuffer, "\0", error);
#ifdef oneM2M_ACME
			boost::asio::read_until(socket, receiveBuffer, '}}', error);
#endif
#ifdef oneM2M_tinyIoT
			boost::asio::read_until(socket, receiveBuffer, '\0', error);
#endif

			istream inputStream(&receiveBuffer);
			ostringstream httpBodyStream;

			// Read the HTTP headers
			while (getline(inputStream, httpRequest) && httpRequest != "\r") {
				// Process or ignore HTTP headers as needed
			}

			// Read the HTTP body (JSON string)
			if (inputStream.peek() != EOF) {
				std::getline(inputStream, httpRequest); // Read the empty line after the headers
			}
		}
		else
		{
			boost::asio::read_until(socket, receiveBuffer, "\n");

			std::istream inputStream(&receiveBuffer);
			std::getline(inputStream, httpRequest);
		}

		if (!error)
		{
			if (port == EMBEDDED_LISTEN_PORT)
			{
				wstring temp;
				temp.assign(httpRequest.begin(), httpRequest.end());

				this->Running_Embedded2(temp);
			}
			else if (port == oneM2M_NOTIFICATION_LISTEN_PORT)
			{
				this->Running_Notification(httpRequest);
			}
		}
	}
	catch (const std::exception&)
	{
		std::cerr << "Error in dt_real_time::handleConnection : " << error.message() << std::endl;
		exit(0);

	}
}

void dt_real_time::Running_Embedded(const wstring& httpResponse)
{
    Elevator* thisBuildingElevator;

    parse_json c;
	Wparsed_struct parsedStruct;

	parsedStruct = c.parsingOnlyBuildingName(httpResponse);
	send_oneM2M ACP_Validation_Socket(parsedStruct);

    wstring ACOR_NAME = parsedStruct.building_name;
    wstring AE_NAME = parsedStruct.building_name;
    wstring CNT_NAME = parsedStruct.device_name;

	std::wcout << endl << "FROM EMBEDDED -> SERVER : " << " Receive Data From : " << parsedStruct.device_name << std::endl;

    int ACP_FLAG = ACP_Validation_Socket.acp_validate(ACOR_NAME, 0);

    if(ACP_FLAG == 0 || ACP_FLAG == 1) // oneM2M에 빌딩 ACP가 존재하지 않는 경우
    {
    	std::wcout << "IN SERVER : " << " ACP : C" << parsedStruct.building_name << " Not Exists" << std::endl;

    	Building* newBuilding;
        this->ACP_NAMES.push_back(ACOR_NAME);

		int buildingAlgorithmNumber;
		std::wcout << "IN SERVER : BUILDING NAME : " << parsedStruct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
		std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
		std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;
		
		std::cin >> buildingAlgorithmNumber;
		while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1) 
		{
			std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
			std::cin >> buildingAlgorithmNumber;
		}
	    
		newBuilding = new Building(buildingAlgorithmNumber);

        // Create THIS AE(Building) Class
        int algNumber;

        if(CNT_NAME == L"EV1" || CNT_NAME == L"EV2")
        {
	        algNumber = 1;
        }
        else
        {
	        algNumber = 2;
        }

		parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, newBuilding->getButtonMod());
		send_oneM2M ACP_Validation_Socket(parsedStruct);

        // Create THIS CNT(Building's Elevator) Class
        thisBuildingElevator = new Elevator(ACP_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

        newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
        newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

        allBuildingInfo.push_back(newBuilding);

        thisBuildingElevator->runElevator();
	    thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct, &(newBuilding->buildingElevatorInfo->subscriptionRI_to_RN));

        if(newBuilding->getButtonMod() && CNT_NAME == L"OUT")
        {
	        //thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);

        	// SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
        	newBuilding->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
        }

        else
        {
	        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
        }

        //DISCOVERY TEST
	    //this_Building_Elevator->sock.socket.discovery_retrieve(this_Building_Elevator->sock.originator_name, 0);

        //RCN TEST
        //this_Building_Elevator->sock.socket.result_content_retrieve(this_Building_Elevator->sock.originator_name, 0);
    }
    else // oneM2M에 빌딩 ACP가 존재하는 경우
    {
        std::wcout << "IN SERVER -> Building ACP Exists, Check Building " << AE_NAME << " Exists In Server" << std::endl;

        // GET THIS BUILDING CLASS and CHECK THIS CNT(Device Name) Exists
        Building* thisBuildingFlag = this->get_building_vector(ACOR_NAME);
		thisBuildingFlag->buildingElevatorInfo->subscriptionRI_to_RN = map<wstring, wstring>();

        // if TEMP == NULL, It means oneM2M Building Resource Exists, But Server Restarted
        if(thisBuildingFlag == NULL) // oneM2M에 빌딩 ACP가 존재하는데 서버에 클래스가 없는 경우
        {
            std::wcout << "IN SERVER -> Building Resource Not Exist in Server Create New " << AE_NAME << " Resource with " << CNT_NAME << std::endl;
	        this->ACP_NAMES.push_back(ACOR_NAME);

			int buildingAlgorithmNumber;
			std::wcout << "IN SERVER : BUILDING NAME : " << parsedStruct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
			std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
			std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

			std::cin >> buildingAlgorithmNumber;
			while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
			{
				std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
				std::cin >> buildingAlgorithmNumber;
			}

	        // Create THIS AE(Building) Class
	        Building* newBuilding;
			newBuilding = new Building(buildingAlgorithmNumber);

            int algNumber = 1;

			parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, newBuilding->getButtonMod());
			send_oneM2M ACP_Validation_Socket(parsedStruct);

			//서버에서 빌딩 클래스 재생성 하고, 이제 해당 엘리베이터가 oneM2M에 존재하는지 확인
			bool flag = ACP_Validation_Socket.cnt_validate(1, CNT_NAME);

			if (flag) //oneM2M 서버에 해당 엘리베이터 클래스가 존재하는 경우
			{
				thisBuildingElevator = new Elevator(ELEVATOR_DATA_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

				newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
				newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

				allBuildingInfo.push_back(newBuilding);

				thisBuildingElevator->runElevator();

				if (newBuilding->getButtonMod() && CNT_NAME == L"OUT")
				{
					//thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);

					newBuilding->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
					// SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
				}
				else
				{
					thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
				}
			}
			else //oneM2M 서버에 해당 엘리베이터 클래스가 존재하지 않는 경우
			{
				thisBuildingElevator = new Elevator(ELEVATOR_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

				newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
				newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

				allBuildingInfo.push_back(newBuilding);

				thisBuildingElevator->runElevator();

				if (newBuilding->getButtonMod())
				{
					thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);

					newBuilding->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
					// SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
				}
				else
				{
					thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
				}
			}
        }

        else  // oneM2M에 빌딩 ACP가 존재하고 서버에도 클래스가 존재하는 경우
        {
        	std::wcout << "IN SERVER -> Building Resource Exists, Check Elevator " << CNT_NAME << " Resource Exists IN Server" << std::endl;

			parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, thisBuildingFlag->getButtonMod());
			send_oneM2M ACP_Validation_Socket(parsedStruct);

        	bool flag = this->existsElevator(thisBuildingFlag, CNT_NAME);

	        if(flag) // oneM2M과 서버에 해당 엘리베이터 클래스가 모두 존재하는 경우
	        {
	            // THIS CNT Exists
	            Elevator* thisBuildingElevator = this->getElevator(thisBuildingFlag, CNT_NAME);

	            // Update CIN Value
	            std::wcout << "IN SERVER -> Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << std::endl;

                if(thisBuildingFlag->getButtonMod() && CNT_NAME == L"OUT")
		        {
			        thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);
					thisBuildingFlag->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
		            // SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
		        }
		        else
		        {
			        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
		        }
	        }
	        else // oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
	        {
                //CHECK THIS CNT EXISTS in oneM2M
        		bool flag = ACP_Validation_Socket.cnt_validate(1, CNT_NAME); //수정 필요, 엘리베이터 CNT 존재하는 지 확인

                if(!flag) // oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
                {
	                // THIS CNT NOT Exists in DT Server and oneM2M
		            std::wcout << "IN SERVER -> Elevator Not Found, Creating New Elevator : " << CNT_NAME << std::endl;

	                int algNumber;
		            //std::wcout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
					//cin >> algNumber;

					if(CNT_NAME == L"EV1" || CNT_NAME == L"EV2" || CNT_NAME == L"EV3")
			        {
				        algNumber = 1;
			        }
			        else
			        {
				        algNumber = 2;
			        }

		            // Create THIS CNT(Building's Elevator) Class
			        thisBuildingElevator = new Elevator(ELEVATOR_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

			        thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

			        thisBuildingElevator->runElevator();
				    thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct, &(thisBuildingFlag->buildingElevatorInfo->subscriptionRI_to_RN));

			        if(thisBuildingFlag->getButtonMod() && CNT_NAME == L"OUT")
			        {
				        thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);
						thisBuildingFlag->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
			            // SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
			        }

			        else
			        {
				        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
			        }

                }

                // THIS CNT NOT Exists in DT Server but in oneM2M
                else // // oneM2M에 해당 엘리베이터 클래스가 존재하는데 서버에 없는 경우
                {
	                std::wcout << "IN SERVER -> Elevator Resource Not Exist in DT Server... Create New CNT " << CNT_NAME << " Resource" << std::endl;
			        this->ACP_NAMES.push_back(ACOR_NAME);

		            int algNumber;
		            //std::wcout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
					//cin >> algNumber;

					if(CNT_NAME == L"EV1" || CNT_NAME == L"EV2" || CNT_NAME == L"EV3")
			        {
				        algNumber = 1;
			        }
			        else
			        {
				        algNumber = 2;
			        }

			        // Create THIS CNT(Building's Elevator) Class
                    thisBuildingElevator = new Elevator(ELEVATOR_DATA_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

			        thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
			        thisBuildingFlag->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

			        thisBuildingElevator->runElevator();

			        if(thisBuildingFlag->getButtonMod() && CNT_NAME == L"OUT")
			        {
				        thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);
						thisBuildingFlag->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
			            // SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
			        }

			        else
			        {
				        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
			        }
                }
	        }
        }
    }
}

void dt_real_time::Running_Embedded2(const wstring& httpResponse)
{
	parse_json c;
	Elevator* thisBuildingElevator;
	Wparsed_struct parsedStruct;

	parsedStruct = c.parsingOnlyBuildingName(httpResponse);
	send_oneM2M ACP_Validation_Socket(parsedStruct);

	wstring ACOR_NAME = parsedStruct.building_name;
	wstring AE_NAME = parsedStruct.building_name;
	wstring CNT_NAME = parsedStruct.device_name;

	std::wcout << endl << "FROM EMBEDDED -> SERVER : " << " Receive Data From : " << parsedStruct.device_name << std::endl;

	int ACP_FLAG = ACP_Validation_Socket.acp_validate(ACOR_NAME, 0);

	Building* thisBuildingFlag = this->get_building_vector(ACOR_NAME);

	// oneM2M에 빌딩 ACP가 존재하지 않는 경우
	if (ACP_FLAG == 0 || ACP_FLAG == 1) 
	{
		// 임베디드 장치로 온 신호가 OUT 신호인 경우 -> REJECT
		if (CNT_NAME == L"OUT")
		{
			wcout << "CANNOT MAKE BUILDING WHEN OUT SIGNAL INCOMES" << endl;

			return;
		}
		
		// 임베디드 장치로 온 신호가 엘리베이터 내부 신호인 경우 -> ACCEPT
		else
		{
			Building* newBuilding;
			this->ACP_NAMES.push_back(ACOR_NAME);

			int buildingAlgorithmNumber;
			std::wcout << "IN SERVER : BUILDING NAME : " << parsedStruct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
			std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
			std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

			std::cin >> buildingAlgorithmNumber;
			while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
			{
				std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
				std::cin >> buildingAlgorithmNumber;
			}

			newBuilding = new Building(buildingAlgorithmNumber);

			// Create THIS AE(Building) Class
			int algNumber = 1;

			parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, newBuilding->getButtonMod());
			send_oneM2M ACP_Validation_Socket(parsedStruct);

			// Create THIS CNT(Building's Elevator) Class
			Elevator* thisBuildingElevator = new Elevator(ACP_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

			newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
			newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

			allBuildingInfo.push_back(newBuilding);

			thisBuildingElevator->runElevator();
			thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct, &(newBuilding->buildingElevatorInfo->subscriptionRI_to_RN));
			thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
		}
	}

	// oneM2M에 빌딩 ACP가 존재하는 경우
	else
	{
		std::wcout << "IN SERVER -> Building ACP Exists, Check Building " << AE_NAME << " Exists In Server" << std::endl;

		// GET THIS BUILDING CLASS and CHECK THIS CNT(Device Name) Exists
		Building* thisBuildingFlag = this->get_building_vector(ACOR_NAME);

		// oneM2M에 빌딩 ACP가 존재하는데 서버에 데이터가 없는 경우
		if (thisBuildingFlag == NULL) 
		{
			if (CNT_NAME == L"OUT")
			{
				wcout << "CANNOT MAKE BUILDING WHEN OUT SIGNAL INCOMES" << endl;

				return;
			}
			else 
			{
				std::wcout << "IN SERVER -> Building Resource Not Exist in Server Create New " << AE_NAME << " Resource with " << CNT_NAME << std::endl;
				this->ACP_NAMES.push_back(ACOR_NAME);

				int buildingAlgorithmNumber;
				std::wcout << "IN SERVER : BUILDING NAME : " << parsedStruct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
				std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
				std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

				std::cin >> buildingAlgorithmNumber;
				while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
				{
					std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
					std::cin >> buildingAlgorithmNumber;
				}

				// Create THIS AE(Building) Class
				Building* newBuilding;
				newBuilding = new Building(buildingAlgorithmNumber);

				int algNumber = 1;

				parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, newBuilding->getButtonMod());
				send_oneM2M ACP_Validation_Socket(parsedStruct);

				//서버에서 빌딩 클래스 재생성 하고, 이제 해당 엘리베이터가 oneM2M에 존재하는지 확인
				bool flag = ACP_Validation_Socket.cnt_validate(1, CNT_NAME);

				if (flag) //oneM2M 서버에 해당 엘리베이터 클래스가 존재하는 경우
				{
					Elevator* thisBuildingElevator = new Elevator(ELEVATOR_DATA_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

					newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
					newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

					allBuildingInfo.push_back(newBuilding);

					thisBuildingElevator->runElevator();
					thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
				}
				else //oneM2M 서버에 해당 엘리베이터 클래스가 존재하지 않는 경우
				{
					Elevator* thisBuildingElevator = new Elevator(ELEVATOR_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

					newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
					newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

					allBuildingInfo.push_back(newBuilding);

					thisBuildingElevator->runElevator();

					if (newBuilding->getButtonMod() && CNT_NAME == L"OUT")
					{
						//thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);
						newBuilding->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
						// SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
					}
					else
					{
						thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
					}
				}
			}
		}

		// oneM2M에 빌딩 ACP가 존재하고 서버에도 데이터가 존재하는 경우
		else  
		{
			std::wcout << "IN SERVER -> Building Resource Exists, Check Elevator " << CNT_NAME << " Resource Exists IN Server" << std::endl;
			thisBuildingFlag->buildingElevatorInfo->subscriptionRI_to_RN = map<wstring, wstring>();

			parsedStruct = c.parsingWithBulidingAlgorithms(httpResponse, thisBuildingFlag->getButtonMod());
			send_oneM2M ACP_Validation_Socket(parsedStruct);

			// 해당 빌딩이 군중화 알고리즘을 사용하는 경우
			if (thisBuildingFlag->getButtonMod())
			{
				if (CNT_NAME == L"OUT")
				{
					thisBuildingFlag->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
				}
				else 
				{
					bool flag = this->existsElevator(thisBuildingFlag, CNT_NAME);
					// oneM2M과 서버에 해당 엘리베이터 클래스가 모두 존재하는 경우
					if (flag) 
					{
						Elevator* thisBuildingElevator = this->getElevator(thisBuildingFlag, CNT_NAME);

						// Update CIN Value
						std::wcout << "IN SERVER -> Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << std::endl;
						thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
					}
					// oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
					else 
					{
						//CHECK THIS CNT EXISTS in oneM2M
						bool flag = ACP_Validation_Socket.cnt_validate(1, CNT_NAME);
						// oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
						if (!flag) 
						{
							// THIS CNT NOT Exists in DT Server and oneM2M
							std::wcout << "IN SERVER -> Elevator Not Found, Creating New Elevator : " << CNT_NAME << std::endl;

							int algNumber = 1;

							// Create THIS CNT(Building's Elevator) Class
							Elevator* thisBuildingElevator = new Elevator(ELEVATOR_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

							thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

							thisBuildingElevator->runElevator();
							thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct, &(thisBuildingFlag->buildingElevatorInfo->subscriptionRI_to_RN));
							thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
						}
						// oneM2M에 해당 엘리베이터 클래스가 존재하는데 서버에 없는 경우
						else 
						{
							std::wcout << "IN SERVER -> Elevator Resource Not Exist in DT Server... Create New CNT " << CNT_NAME << " Resource" << std::endl;

							int algNumber = 1;

							// Create THIS CNT(Building's Elevator) Class
							Elevator* thisBuildingElevator = new Elevator(ELEVATOR_DATA_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

							thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
							thisBuildingFlag->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

							thisBuildingElevator->runElevator();

							if (thisBuildingFlag->getButtonMod() && CNT_NAME == L"OUT")
							{
								thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
							}
						}
					}
				}
			}

			// 해당 빌딩이 군중화 알고리즘을 사용하지 않는 경우
			else 
			{
				if (CNT_NAME == L"OUT")
				{
					std::wcout << "IN SERVER -> ERROR IN RUNNIG EMBEDDED : BUILDING WITH DEDICATED LOBBY CALL CANNOT ACCEPT LOG NAME 'OUT'" << endl;
					return;
				}
				else
				{
					bool flag = this->existsElevator(thisBuildingFlag, CNT_NAME);

					// oneM2M과 서버에 해당 엘리베이터 클래스가 모두 존재하는 경우
					if (flag) 
					{
						// THIS CNT Exists
						Elevator* thisBuildingElevator = this->getElevator(thisBuildingFlag, CNT_NAME);

						// Update CIN Value
						std::wcout << "IN SERVER -> Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << std::endl;

						thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
					}
					
					// DT서버에 해당 엘리베이터 클래스가 존재하지 않는 경우
					else 
					{
						//CHECK THIS CNT EXISTS in oneM2M
						bool flag = ACP_Validation_Socket.cnt_validate(1, CNT_NAME);

						// DT서버와 oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
						if (!flag) 
						{
							// THIS CNT NOT Exists in DT Server and oneM2M
							std::wcout << "IN SERVER -> Elevator Not Found, Creating New Elevator : " << CNT_NAME << std::endl;

							int algNumber = 1;

							// Create THIS CNT(Building's Elevator) Class
							thisBuildingElevator = new Elevator(ELEVATOR_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

							thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

							thisBuildingElevator->runElevator();
							thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct, &(thisBuildingFlag->buildingElevatorInfo->subscriptionRI_to_RN));
							thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
						}

						// oneM2M에는 엘리베이터 리소스가 존재하나, DT 서버에 존재하지 않는 경우
						else
						{
							std::wcout << "IN SERVER -> Elevator Resource Not Exist in DT Server... Create New CNT " << CNT_NAME << " Resource" << std::endl;
							this->ACP_NAMES.push_back(ACOR_NAME);

							int algNumber = 1;

							// Create THIS CNT(Building's Elevator) Class
							thisBuildingElevator = new Elevator(ELEVATOR_DATA_NOT_FOUND, parsedStruct, this->ACP_NAMES, algNumber);

							thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
							thisBuildingFlag->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

							thisBuildingElevator->runElevator();
							thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
						}
					}
				}
			}
		}
	}
}

void dt_real_time::Running_Notification(const string& httpResponse)
{
    try
	{
		auto json_body = nlohmann::json::parse(httpResponse);

		if (json_body.contains("m2m:sgn")) 
		{
            auto nev = json_body["m2m:sgn"]["nev"];

            // Check if "rep" object exists within "nev"
            if (nev.contains("rep")) 
			{
#ifdef oneM2M_tinyIoT
				auto rep = nev["rep"];

				auto rep_json = nlohmann::json::parse(nev["rep"].get<std::string>());

				if (rep_json.contains("m2m:sub"))
				{
					return;
				}
				else
				{
					//START ANALYZE
					std::string sur_string = json_body["m2m:sgn"]["sur"];
					std::string rep_value_str = json_body["m2m:sgn"]["nev"]["rep"];
					nlohmann::json rep_value_json = nlohmann::json::parse(rep_value_str);

					std::stringstream ss(sur_string);

					std::vector<std::string> sur_values;
					std::string item;

					while (std::getline(ss, item, '/'))
					{
						sur_values.push_back(item);
					}

					string building_name = sur_values[1];
					string device_name = sur_values[2];
					string majorCNTName = sur_values[3];

					wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
					wstring Wbuilding_name = converter.from_bytes(building_name);
					wstring Wdevice_name = converter.from_bytes(device_name);

					Building* thisBuilding = this->get_building_vector(Wbuilding_name);
					Elevator* thisElevator = this->getElevator(thisBuilding, Wdevice_name);

					string con;

					noti_content = new notificationContent;

					if (majorCNTName == "Elevator_physics")
					{
						string minorCNTName = sur_values[4];

						if (minorCNTName.find("Velocity") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
							noti_content->current_velocity = stod(con);
						}
						else if (minorCNTName.find("Altimeter") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
							noti_content->current_altimeter = stod(con);
						}
						thisElevator->setNotificationContent(noti_content);
					}
					else if (majorCNTName == "Elevator_button_inside")
					{
						string minorCNTName = sur_values[4];

						if (minorCNTName.find("Button_List") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();

							std::vector<int> integers;
							std::istringstream iss(con);
							std::string token;
							while (iss >> token)
							{
								// Convert each token (integer string) to an integer and store it in the vector
								if (token[0] == 'B')
								{
									integers.push_back(-1 * stoi(token.substr(1)));
								}

								else
								{
									integers.push_back(stoi(token));
								}
							}
							if (integers[0] == -1)
							{
								std::wcout << "" << std::endl;
							}
							noti_content->button_inside = integers;
						}

						thisElevator->setNotificationContent(noti_content);
						thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
					else if (majorCNTName == "Elevator_button_outside")
					{
						string minorCNTName = sur_values[4];
						con = rep_value_json["m2m:cin"]["con"].get<string>();

						int called_floor;
						bool direction;

						//가정 1. 바깥에 누른 층은 한번 누르면 끝
						if (con == "None") //바깥의 층에 도달하여 자연스럽게 사라진 경우 None
						{
							return;
						}
						else if (con == "Up")
						{
							direction = true;
						}
						else
						{
							direction = false;
						}
						if (!minorCNTName.empty() && minorCNTName[0] == 'B')
						{
							called_floor = (minorCNTName[minorCNTName.size() - 1] - '0') * -1;
							noti_content->button_outside_altimeter = thisElevator->getAltimeterFromFloor(called_floor);
						}
						else
						{
							called_floor = stoi(minorCNTName);
							noti_content->button_outside_altimeter = thisElevator->getAltimeterFromFloor(called_floor);
						}
						noti_content->button_outside_floor = called_floor;
						noti_content->button_outside_direction = direction;

						thisElevator->setNotificationContent(noti_content);
						thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
				}
#endif // oneM2M_tinyIoT
#ifdef ACME
				auto rep_json = nev["rep"];

				if (rep_json.contains("m2m:sub"))
				{
					return;
				}
				else
				{
					//START ANALYZE
					std::string sur_string = json_body["m2m:sgn"]["sur"];
					std::stringstream ss(sur_string);

					std::vector<std::string> sur_values;
					std::string item;

					while (std::getline(ss, item, '/'))
					{
						sur_values.push_back(item);
					}

					string CSE_ID = sur_values[1];
					string subscription_RI = sur_values[2];

					string rn_path = get_RN_Path_From_SubscriptionRI(subscription_RI);
					std::stringstream sss(rn_path);

					std::vector<std::string> path_values;
					std::string item2;

					while (std::getline(sss, item2, '/'))
					{
						path_values.push_back(item2);
					}

					string building_name = path_values[4];
					string device_name = path_values[5];
					string majorCNTName = path_values[6];

					wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
					wstring Wbuilding_name = converter.from_bytes(building_name);
					wstring Wdevice_name = converter.from_bytes(device_name);

					Building* thisBuilding = this->get_building_vector(Wbuilding_name);
					Elevator* thisElevator = this->getElevator(thisBuilding, Wdevice_name);

					string con;
					noti_content = new notificationContent;

					if (majorCNTName == "Elevator_physics")
					{
						string minorCNTName = path_values[7];

						if (minorCNTName.find("Velocity") != string::npos)
						{
							con = rep_json["m2m:cin"]["con"].get<string>();
							noti_content->current_velocity = stod(con);
						}
						else if (minorCNTName.find("Altimeter") != string::npos)
						{
							con = rep_json["m2m:cin"]["con"].get<string>();
							noti_content->current_altimeter = stod(con);
						}
						thisElevator->setNotificationContent(noti_content);
					}
					else if (majorCNTName == "Elevator_button_inside")
					{
						string minorCNTName = path_values[7];

						if (minorCNTName.find("Button_List") != string::npos)
						{
							con = rep_json["m2m:cin"]["con"].get<string>();

							std::vector<int> integers;
							std::istringstream iss(con);
							std::string token;
							while (iss >> token)
							{
								// Convert each token (integer string) to an integer and store it in the vector
								if (token[0] == 'B')
								{
									integers.push_back(-1 * stoi(token.substr(1)));
								}

								else
								{
									integers.push_back(stoi(token));
								}
							}
							if (integers[0] == -1)
							{
								std::wcout << "" << std::endl;
							}
							noti_content->button_inside = integers;
						}

						thisElevator->setNotificationContent(noti_content);
						thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
					else if (majorCNTName == "Elevator_button_outside")
					{
						string minorCNTName = path_values[7];
						con = rep_json["m2m:cin"]["con"].get<string>();

						int called_floor;
						bool direction;

						//가정 1. 바깥에 누른 층은 한번 누르면 끝
						if (con == "None") //바깥의 층에 도달하여 자연스럽게 사라진 경우 None
						{
							return;
						}
						else if (con == "Up")
						{
							direction = true;
						}
						else
						{
							direction = false;
						}
						if (!minorCNTName.empty() && minorCNTName[0] == 'B')
						{
							called_floor = (minorCNTName[minorCNTName.size() - 1] - '0') * -1;
							noti_content->button_outside_altimeter = thisElevator->getAltimeterFromFloor(called_floor);
						}
						else
						{
							called_floor = stoi(minorCNTName);
							noti_content->button_outside_altimeter = thisElevator->getAltimeterFromFloor(called_floor);
						}
						noti_content->button_outside_floor = called_floor;
						noti_content->button_outside_direction = direction;

						thisElevator->setNotificationContent(noti_content);
						thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
				}
#endif
            }
			else 
			{
                std::wcout << "rep field does not exist within nev." << std::endl;
				exit(0);
            }
        }
		else 
		{
            std::wcout << "m2m:sgn field does not exist." << std::endl;
			exit(0);
        }
	}
	catch (std::exception e)
	{
		std::cerr << "Error in dt_real_time::Running_Notification : " << e.what() << std::endl;
	}
}