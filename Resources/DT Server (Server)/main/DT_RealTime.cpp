#include <nlohmann/json.hpp>
#include "DT_RealTime.h"
#include "DT_Simulation.h"
#include "config.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"
#include "simulation.h"
#include "elevator.h"

#include <locale>
#include <codecvt>
#include <sstream>
#include <unordered_set>

#define _SILE

using namespace boost::asio;
using ip::tcp;
using std::chrono::system_clock;

Building::Building(wstring building_name, int buttonMod)
{
	this_log = new logger;

	this->buliding_start_time = system_clock::now();
	this->buildingElevatorInfo = new class_of_one_Building;
	this->buildingElevatorInfo->outsideButtonMod = buttonMod;

	this->buildingElevatorInfo->ACP_NAME = L"C" + building_name;
	this->buildingElevatorInfo->log_name_for_building = this_log->get_file_name_as_timestamp() + L"_" + building_name + L".txt";

	// All Outside Button Infos in Single Building
	this->currentButtonOutsideInfos = new vector<vector<int>>;
}

int Building::getButtonMod()
{
	return this->buildingElevatorInfo->outsideButtonMod;
}

vector<vector<int>> Building::getDiffBetweenNewAndCurrent(vector<vector<int>> newList, vector<vector<int>> oldList)
{
	vector<vector<int>> newElements;
	vector<vector<int>> deletedElements;

	unordered_set<vector<int>, hash_function> oldSet(oldList.begin(), oldList.end());

	for (const auto& vec : newList)
	{
		if (oldSet.find(vec) == oldSet.end()) {
			newElements.push_back(vec);
		}
	}

	for (const auto& vec : oldList)
	{
		if (oldSet.find(vec) == oldSet.end()) {
			deletedElements.push_back(vec);
		}
	}

	return newElements;
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
		addedButtonOutsideList = getDiffBetweenNewAndCurrent(button_outside, *(this->currentButtonOutsideInfos));

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
						//std::wcout << elem->getBuildingName() << " -> " << elem->getDeviceName() << " IS IN IDLE..." << std::endl;
					}
					else
					{
						if (calledDirection)
						{
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
				//std::wcout << selectedElevatorIDLE->getBuildingName() << " -> " << selectedElevatorIDLE->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
				std::wstring log_string = selectedElevatorIDLE->getBuildingName() + L" -> " + selectedElevatorIDLE->getDeviceName() + L" is SELECTED FOR CALL " + std::to_wstring(calledFloor);
				selectedElevatorIDLE->getElevatorAlgorithm()->write_log(log_string);

				std::wcout << log_string << std::endl;
				temp.push_back(eachCall);

				const int delta_second = selectedElevatorIDLE->thisElevatorAlgorithmSingle->printTimeDeltaNow();
				selectedElevatorIDLE->thisElevatorAlgorithmSingle->appendLogToLogList(CALL, 5, selectedElevatorIDLE->getBuildingName(), selectedElevatorIDLE->getDeviceName(), calledFloor, calledDirection, delta_second);

				selectedElevatorIDLE->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
			}

			else if (selectedElevator != NULL)
			{
				//				std::wcout << selectedElevator->getBuildingName() << " -> " << selectedElevator->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
				std::wstring log_string = selectedElevator->getBuildingName() + L" -> " + selectedElevator->getDeviceName() + L" is SELECTED FOR CALL " + std::to_wstring(calledFloor);
				selectedElevator->getElevatorAlgorithm()->write_log(log_string);

				std::wcout << log_string << std::endl;
				temp.push_back(eachCall);

				const int delta_second = selectedElevator->thisElevatorAlgorithmSingle->printTimeDeltaNow();
				selectedElevator->thisElevatorAlgorithmSingle->appendLogToLogList(CALL, 5, selectedElevator->getBuildingName(), selectedElevator->getDeviceName(), calledFloor, calledDirection, delta_second);

				selectedElevator->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
			}

			else
			{
				int diff = 0;
				for (Elevator* elem : thisBuildingElevators)
				{
					int eachFloorLastDestination;
					//check if elem->p->s->main_trip_list.back() is valid
					if (elem->p->s->main_trip_list.size() == 0)
					{
						// set goTo Floor as Last Destination
						eachFloorLastDestination = elem->getElevatorStatus()->go_to_floor;
					}
					else
					{
						eachFloorLastDestination = (elem->p->s->main_trip_list.back())[0];
					}


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
					//					std::wcout << selectedElevatorDifferentDirection->getBuildingName() << " -> " << selectedElevatorDifferentDirection->getDeviceName() << " is SELECTED FOR CALL " << calledFloor << std::endl;
					std::wstring log_string = selectedElevatorDifferentDirection->getBuildingName() + L" -> " + selectedElevatorDifferentDirection->getDeviceName() + L" is SELECTED FOR CALL " + std::to_wstring(calledFloor);
					selectedElevatorDifferentDirection->getElevatorAlgorithm()->write_log(log_string);

					std::wcout << log_string << std::endl;
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

dt_real_time::dt_real_time() : parsed_struct()
{
	this->httpRequest = new std::wstring;
	this->noti_content = new notificationContent;
}

Building* dt_real_time::getBuilding(const wstring& building_name)
{
    for(size_t i = 0 ; i < this->allBuildingInfo.size() ; i++)
    {
		if (this->allBuildingInfo[i]->buildingElevatorInfo->ACP_NAME == L"C" + building_name)
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

vector<elevator_resource_status> dt_real_time::checkoneM2M(http_request_header_data* httpRequestHeaderData, Wparsed_struct parsed_struct, send_oneM2M ACP_Validation_Socket)
{
	vector<elevator_resource_status> ret;

	int flag = ACP_Validation_Socket.acp_validate(parsed_struct.building_name, 0);

	if (flag == DT_ACP_NOT_FOUND)
	{
		ret.push_back(DT_ACP_NOT_FOUND);
		return ret;
	}

	flag = ACP_Validation_Socket.ae_validate(parsed_struct, 0);

	if (flag == BUILDING_NOT_FOUND)
	{
		ret.push_back(BUILDING_NOT_FOUND);
		return ret;
	}

	auto discovered = ACP_Validation_Socket.discovery_retrieve(L"C" + parsed_struct.building_name, 3, 1, 1, parsed_struct.building_name);

	if (discovered.size() == 0)
	{
		ret.push_back(ELEVATOR_NOT_FOUND);
		return ret;
	}

	bool notFound = true;
	// Parse each nhloman json array
	for (int i = 0; i < discovered.size(); i++)
	{
		// CHECK IF THIS ELEVATOR EXISTS
		if (discovered.at(i).serialize().find(parsed_struct.device_name) != wstring::npos)
		{
			notFound = false;
			break;
		}
	}
	if (notFound)
	{
		ret.push_back(ELEVATOR_NOT_FOUND);
		return ret;
	}

	discovered = ACP_Validation_Socket.discovery_retrieve(L"C" + parsed_struct.building_name, 3, 1, 2, parsed_struct.building_name, parsed_struct.device_name);

	if (httpRequestHeaderData->en == 1)
	{
		bool notFound = true;
		// Parse each nhloman json array
		for (int i = 0; i < discovered.size(); i++)
		{
			// CHECK IF THIS ELEVATOR EXISTS
			if (discovered.at(i).serialize().find(L"Energy") != wstring::npos)
			{
				notFound = false;
				break;
			}
		}
		if (notFound)
		{
			ret.push_back(ENERGY_CNT_NOT_FOUND);
		}
	}

	if (httpRequestHeaderData->ty == 3)
	{
		bool notFound = true;
		// Parse each nhloman json array
		for (int i = 0; i < discovered.size(); i++)
		{
			// CHECK IF THIS ELEVATOR EXISTS
			if (discovered.at(i).serialize().find(L"Physical") != wstring::npos)
			{
				notFound = false;
				break;
			}
		}
		if (notFound)
		{
			ret.push_back(PHYSICAL_CNT_NOT_FOUND);
		}
	}

	return ret;
}

elevator_resource_status dt_real_time::checkDTServer(vector<Building*> bulidings, Wparsed_struct parsed_struct)
{
	wstring building_name = parsed_struct.building_name;
	wstring device_name = parsed_struct.device_name;

	// CHECK IF THIS BUILDING EXISTS
	for (int i = 0; i < bulidings.size(); i++)
	{
		if (bulidings.at(i)->buildingElevatorInfo->ACP_NAME == L"C" + building_name)
		{
			// CHECK IF THIS ELEVATOR EXISTS
			for (int j = 0; j < bulidings.at(i)->buildingElevatorInfo->classOfAllElevators.size(); j++)
			{
				if (bulidings.at(i)->buildingElevatorInfo->classOfAllElevators.at(j)->getDeviceName() == device_name)
				{
					return ELEVATOR_FOUND;
				}
			}
			return ELEVATOR_NOT_FOUND;
		}
	}
	return BUILDING_NOT_FOUND;
}

void dt_real_time::CreateNewBuildingAndElevator(const string& httpRequestBody, const wstring& ACOR_NAME, elevator_resource_status status)
{
	this->ACP_NAMES.push_back(ACOR_NAME);

	int buildingAlgorithmNumber;
	std::wcout << "IN SERVER : BUILDING NAME : " << parsed_struct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
	std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
	std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

	std::cin >> buildingAlgorithmNumber;
	while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
	{
		std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
		std::cin >> buildingAlgorithmNumber;
	}

	Building* newBuilding = new Building(building_name, buildingAlgorithmNumber);

	// Create THIS AE(Building) Class
	int algNumber = 1;

	// Create THIS CNT(Building's Elevator) Class
	Elevator* thisBuildingElevator = new Elevator(status, parsed_struct, this->ACP_NAMES, algNumber, newBuilding->buliding_start_time);

	newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

	allBuildingInfo.push_back(newBuilding);

	//setConstructArgumentsOfElevator(newBuilding, thisBuildingElevator);

	if (httpRequestHeaderData->en == 1)
	{
		thisBuildingElevator->sock->create_oneM2M_CIN_EnergyConsumption(parsed_struct);
	}

	thisBuildingElevator->UEsock->VisualizationMod = this->VisualizeMod;

	thisBuildingElevator->getElevatorAlgorithm()->thisLogger.set_log_directory_RTS();
	thisBuildingElevator->getElevatorAlgorithm()->thisLogger.log_file_name_for_building_logs = newBuilding->buildingElevatorInfo->log_name_for_building;
	allBuildingInfo.back()->buildingElevatorInfo->classOfAllElevators.back()->sock->create_oneM2M_SUBs(parsed_struct, &(allBuildingInfo.back()->buildingElevatorInfo->subscriptionRI_to_RN));

	thisBuildingElevator->runElevator();
}

void dt_real_time::CreateNewBuilding(const string& httpRequestBody, const wstring& ACOR_NAME, elevator_resource_status status)
{
	this->ACP_NAMES.push_back(ACOR_NAME);

	int buildingAlgorithmNumber;
	std::wcout << "IN SERVER : BUILDING NAME : " << parsed_struct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
	std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
	std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

	std::cin >> buildingAlgorithmNumber;
	while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
	{
		std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
		std::cin >> buildingAlgorithmNumber;
	}

	Building* newBuilding = new Building(building_name, buildingAlgorithmNumber);

	allBuildingInfo.push_back(newBuilding);
}

void dt_real_time::CreateNewElevator(const string& httpRequestBody, Building* thisBuilding, elevator_resource_status status)
{
	// Create THIS AE(Building) Class
	int algNumber = 1;

	// Create THIS CNT(Building's Elevator) Class
	Elevator* thisBuildingElevator = new Elevator(status, parsed_struct, this->ACP_NAMES, algNumber, thisBuilding->buliding_start_time);

	thisBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

	if (httpRequestHeaderData->en == 1)
	{
		thisBuildingElevator->sock->create_oneM2M_CIN_EnergyConsumption(parsed_struct);
	}

	thisBuildingElevator->UEsock->VisualizationMod = this->VisualizeMod;

	thisBuildingElevator->getElevatorAlgorithm()->thisLogger.set_log_directory_RTS();
	thisBuildingElevator->getElevatorAlgorithm()->thisLogger.log_file_name_for_building_logs = thisBuilding->buildingElevatorInfo->log_name_for_building;

	allBuildingInfo.back()->buildingElevatorInfo->classOfAllElevators.back()->sock->create_oneM2M_SUBs(parsed_struct, &(thisBuilding->buildingElevatorInfo->subscriptionRI_to_RN));

	thisBuildingElevator->runElevator();
}

void dt_real_time::Run()
{
    boost::asio::io_context ioContext;

    tcp::endpoint EMBEDDED_END(tcp::v4(), EMBEDDED_LISTEN_PORT);
    tcp::endpoint NOTIFICATION_END(tcp::v4(), oneM2M_NOTIFICATION_LISTEN_PORT);

    tcp::acceptor acceptor_enbedded(ioContext, EMBEDDED_END, EMBEDDED_LISTEN_PORT);
    tcp::acceptor acceptor_notification(ioContext, NOTIFICATION_END, oneM2M_NOTIFICATION_LISTEN_PORT);
    //std::thread timerThread(print_elapsed_time);

    acceptor_enbedded.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            handleConnection(socket, EMBEDDED_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << EMBEDDED_LISTEN_PORT << ": " << error.message() << std::endl;
        }
		startAsyncAccept_Embedded(acceptor_enbedded,  ioContext);
    });

    acceptor_notification.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            handleConnection(socket, oneM2M_NOTIFICATION_LISTEN_PORT);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << oneM2M_NOTIFICATION_LISTEN_PORT << ": " << error.message() << std::endl;
        }
		startAsyncAccept_Notification(acceptor_notification,  ioContext);
    });
    ioContext.run();
}

void dt_real_time::startAsyncAccept_Embedded(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
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
		startAsyncAccept_Embedded(acceptor, ioContext);
    });
}

void dt_real_time::startAsyncAccept_Notification(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
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
		startAsyncAccept_Notification(acceptor, ioContext);
    });
}

void dt_real_time::handleConnection(boost::asio::ip::tcp::socket& socket, int port)
{
    boost::asio::streambuf receiveBuffer;
	receiveBuffer.prepare(BUFFER_SIZE);

    boost::system::error_code error;
	string line;

	try
	{
		//boost::asio::read_until(socket, receiveBuffer, "\0", error);
#ifdef oneM2M_ACME
		boost::asio::read_until(socket, receiveBuffer, '}}', error);
#endif
#ifdef oneM2M_tinyIoT
		std::uint32_t size;
		size_t len = boost::asio::read_until(socket, receiveBuffer, '}}', error);

		if (error && error != boost::asio::error::eof) {
			throw boost::system::system_error(error);
		}
#endif
		// Seperate the HTTP header and body
		std::string message(boost::asio::buffers_begin(receiveBuffer.data()), boost::asio::buffers_end(receiveBuffer.data()));
		std::string delimeter = "\r\n\r\n";

		auto pos = message.find(delimeter);

		if (pos == std::string::npos)
		{
			std::cerr << "Error in dt_real_time::handleConnection : " << "http header <-> body delimeter not found" << std::endl;
			throw std::exception();
		}

		std::string http_header = message.substr(0, pos);
		std::string http_body = message.substr(pos + delimeter.length());

		// \\\" -> \" in http body
		//http_body.erase(std::remove(http_body.begin(), http_body.end(), '\\'), http_body.end());
		//http_body = http_body.substr(1, http_body.length() - 2);

		// Remove \r in each line of http header
		http_header.erase(std::remove(http_header.begin(), http_header.end(), '\r'), http_header.end());

		std::map<std::string, std::string> http_header_map;
		std::istringstream http_header_stream(http_header);
		while(std::getline(http_header_stream, line))
		{
			std::string key = line.substr(0, line.find(':'));
			std::string value = line.substr(line.find(':') + 1);
			http_header_map[key] = value;
		}

		// Check ty an en value in http header, Content-Type ty=?&en=?
		string content_type;
		if (http_header_map.find("Content-Type") != http_header_map.end())
		{
			content_type = http_header_map["Content-Type"];
		}
		else
		{
			std::cerr << "Error in dt_real_time::handleConnection : " << "Content-Type not found in http header" << std::endl;
			throw std::exception();
		}
		vector<int> tyen = data_parser.parse_content_type(content_type);

		if (tyen.size() == 1)
		{
			httpRequestHeaderData->ty = tyen[0];
		}
		else if (tyen.size() == 2)
		{
			httpRequestHeaderData->ty = tyen[0];
			httpRequestHeaderData->en = tyen[1];
		}

		if (!error)
		{
			const string httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";

			if (port == EMBEDDED_LISTEN_PORT)
			{
				if (tyen[0] == 0)
				{
					this->Running_Init(http_body);
					boost::asio::write(socket, boost::asio::buffer(httpResponse), error);
				}
				else if (tyen[0] == 1)
				{
					// Case when Outside Button is pressed
					parsed_struct = data_parser.parsingWithBulidingAlgorithms(http_body, 1);
					Building* thisBuilding = getBuilding(parsed_struct.building_name);
					thisBuilding->getWhichElevatorToGetButtonOutside(parsed_struct.button_outside);
					boost::asio::write(socket, boost::asio::buffer(httpResponse), error);
				}
				else
				{
					boost::asio::write(socket, boost::asio::buffer(httpResponse), error);
					const int res = this->Running_Embedded(http_body);
					if (res)
					{
						throw std::exception();
					}
				}
			}
			else if (port == oneM2M_NOTIFICATION_LISTEN_PORT)
			{
				this->Running_Notification(http_body);
			}
		}
	}
	catch (const std::exception&)
	{
		std::cerr << "Error in dt_real_time::handleConnection : " << error.message() << std::endl;

		const string httpResponse = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
		boost::asio::write(socket, boost::asio::buffer(httpResponse), error);

		exit(0);
	}
}

int dt_real_time::Running_Embedded(string& httpRequestBody)
{
	Elevator* thisBuildingElevator;
	send_oneM2M ACP_Validation_Socket(parsed_struct);

	//parsed_struct = data_parser.parsingOnlyBuildingName(httpRequestBody);
	parsed_struct = data_parser.parsingWithBulidingAlgorithms(httpRequestBody, 1);
	building_name = parsed_struct.building_name;
	ACOR_NAME = parsed_struct.building_name;
	AE_NAME = parsed_struct.building_name;
	CNT_NAME = parsed_struct.device_name;

	try
	{
		std::wcout << endl << "FROM EMBEDDED -> TO SERVER : " << " Receive Data From " << parsed_struct.building_name << " - " << parsed_struct.device_name << std::endl;

		// GET Building and Elevator Instances
		Building* thisBuilding = getBuilding(AE_NAME);
		Elevator* thisBuildingElevator = getElevator(thisBuilding, CNT_NAME);

		// Assert null check
		if (thisBuilding == NULL || thisBuildingElevator == NULL)
		{
			throw std::exception("Error in dt_real_time::Running_Embedded -> thisBuilding is NULL");
		}

		thisBuildingElevator->sock->create_oneM2M_CINs(parsed_struct);
	}
	catch (const std::exception& e)
	{
		std::wcout << "Error in dt_real_time::Running_Embedded : " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

void dt_real_time::Running_Init(string& httpRequestBody)
{
	Elevator* thisBuildingElevator;
	send_oneM2M ACP_Validation_Socket(parsed_struct);

	//parsed_struct = data_parser.parsingOnlyBuildingName(httpRequestBody);
	parsed_struct = data_parser.parsingWithBulidingAlgorithms(httpRequestBody, 1);
	building_name = parsed_struct.building_name;
	ACOR_NAME = parsed_struct.building_name;
	AE_NAME = parsed_struct.building_name;
	CNT_NAME = parsed_struct.device_name;

	try
	{
		std::wcout << endl << "FROM EMBEDDED -> TO INIT : " << " Receive Data From " << parsed_struct.building_name << " - " << parsed_struct.device_name << std::endl;

		vector<elevator_resource_status> result_oneM2M = checkoneM2M(httpRequestHeaderData, parsed_struct, ACP_Validation_Socket);
		elevator_resource_status result_DT = checkDTServer(this->allBuildingInfo, parsed_struct);

		Building* thisBuilding = nullptr;
		Elevator* thisBuildingElevator = nullptr;

		if (!result_oneM2M.empty())
		{
			if (result_oneM2M[0] == DT_ACP_NOT_FOUND || result_oneM2M[0] == BUILDING_NOT_FOUND)
			{
				CreateNewBuildingAndElevator(httpRequestBody, ACOR_NAME, DT_ACP_NOT_FOUND);
				allBuildingInfo.back()->buildingElevatorInfo->classOfAllElevators.back()->sock->create_oneM2M_CINs(parsed_struct);
			}
			else if (result_oneM2M[0] == ELEVATOR_NOT_FOUND)
			{
				if (result_DT == BUILDING_NOT_FOUND)
				{
					this->ACP_NAMES.push_back(ACOR_NAME);

					int buildingAlgorithmNumber;
					std::wcout << "IN SERVER : BUILDING NAME : " << parsed_struct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
					std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
					std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

					std::cin >> buildingAlgorithmNumber;
					while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
					{
						std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
						std::cin >> buildingAlgorithmNumber;
					}

					thisBuilding = new Building(building_name, buildingAlgorithmNumber);
					allBuildingInfo.push_back(thisBuilding);
				}
				else
				{
					thisBuilding = this->getBuilding(ACOR_NAME);
				}
				CreateNewElevator(httpRequestBody, thisBuilding, ELEVATOR_NOT_FOUND);
				allBuildingInfo.back()->buildingElevatorInfo->classOfAllElevators.back()->sock->create_oneM2M_CINs(parsed_struct);
			}
			else if (result_oneM2M[0] == ENERGY_CNT_NOT_FOUND || result_DT == PHYSICAL_CNT_NOT_FOUND)
			{
				if (result_DT == BUILDING_NOT_FOUND)
				{
					this->ACP_NAMES.push_back(ACOR_NAME);

					int buildingAlgorithmNumber;
					std::wcout << "IN SERVER : BUILDING NAME : " << parsed_struct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
					std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
					std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

					std::cin >> buildingAlgorithmNumber;
					while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
					{
						std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
						std::cin >> buildingAlgorithmNumber;
					}

					thisBuilding = new Building(building_name, buildingAlgorithmNumber);
					allBuildingInfo.push_back(thisBuilding);

					result_DT == ELEVATOR_NOT_FOUND;
				}
				if (result_DT == ELEVATOR_NOT_FOUND)
				{
					thisBuilding = this->getBuilding(ACOR_NAME);
					thisBuildingElevator = new Elevator(MAKE_DT_ONLY, parsed_struct, this->ACP_NAMES, thisBuilding->getButtonMod(), thisBuilding->buliding_start_time);
					
					thisBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

					thisBuildingElevator->UEsock->VisualizationMod = this->VisualizeMod;

					thisBuildingElevator->getElevatorAlgorithm()->thisLogger.set_log_directory_RTS();
					thisBuildingElevator->getElevatorAlgorithm()->thisLogger.log_file_name_for_building_logs = thisBuilding->buildingElevatorInfo->log_name_for_building;

					thisBuildingElevator->runElevator();
				}
				else
				{
					thisBuilding = this->getBuilding(ACOR_NAME);
					thisBuildingElevator = this->getElevator(thisBuilding, CNT_NAME);
				}

				if (result_DT == ENERGY_CNT_NOT_FOUND)
				{
					thisBuildingElevator->sock->create_oneM2M_CIN_EnergyConsumption(parsed_struct);
				}
				else if(result_DT == PHYSICAL_CNT_NOT_FOUND)
				{
					thisBuildingElevator->sock->create_oneM2M_CIN_Physics(parsed_struct);
				}

				allBuildingInfo.back()->buildingElevatorInfo->classOfAllElevators.back()->sock->create_oneM2M_CINs(parsed_struct);
			}
			return;
		}

		if (result_DT == BUILDING_NOT_FOUND)
		{
			this->ACP_NAMES.push_back(ACOR_NAME);

			int buildingAlgorithmNumber;
			std::wcout << "IN SERVER : BUILDING NAME : " << parsed_struct.building_name << " Is Newly Added. Please Enter This Building's Lobby Call Algorithm" << std::endl;
			std::wcout << "0 : IF EVERY ELEVATOR HAS UNIQUE OUTSIDE BUTTON " << std::endl;
			std::wcout << "1 : IF BUILDING HAS ELEVATOR CROWD CONTROL SYSTEM" << std::endl;

			std::cin >> buildingAlgorithmNumber;
			while (buildingAlgorithmNumber != 0 && buildingAlgorithmNumber != 1)
			{
				std::wcout << "PLEASE ENTER 0 OR 1" << std::endl;
				std::cin >> buildingAlgorithmNumber;
			}

			thisBuilding = new Building(building_name, buildingAlgorithmNumber);
			allBuildingInfo.push_back(thisBuilding);

			thisBuildingElevator = new Elevator(MAKE_DT_ONLY, parsed_struct, this->ACP_NAMES, thisBuilding->getButtonMod(), thisBuilding->buliding_start_time);
			thisBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

			thisBuildingElevator->UEsock->VisualizationMod = this->VisualizeMod;

			thisBuildingElevator->getElevatorAlgorithm()->thisLogger.set_log_directory_RTS();
			thisBuildingElevator->getElevatorAlgorithm()->thisLogger.log_file_name_for_building_logs = thisBuilding->buildingElevatorInfo->log_name_for_building;

			thisBuildingElevator->runElevator();
		}
		else if (result_DT == ELEVATOR_NOT_FOUND)
		{
			thisBuilding = this->getBuilding(ACOR_NAME);
			thisBuildingElevator = new Elevator(MAKE_DT_ONLY, parsed_struct, this->ACP_NAMES, thisBuilding->getButtonMod(), thisBuilding->buliding_start_time);
			thisBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

			thisBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

			thisBuildingElevator->UEsock->VisualizationMod = this->VisualizeMod;

			thisBuildingElevator->getElevatorAlgorithm()->thisLogger.set_log_directory_RTS();
			thisBuildingElevator->getElevatorAlgorithm()->thisLogger.log_file_name_for_building_logs = thisBuilding->buildingElevatorInfo->log_name_for_building;

			thisBuildingElevator->runElevator();
		}
		else
		{
		}
		return;
	}
	catch (const std::exception& e)
	{
		std::wcout << "Error in dt_real_time::Running_Embedded : " << e.what() << std::endl;
		exit(0);
	}
}

void dt_real_time::Running_Notification(string& http_body)
{
    try
	{
		auto json_body = nlohmann::json::parse(http_body);
		
		if (json_body.contains("m2m:sgn")) 
		{
            auto nev = json_body["m2m:sgn"]["nev"];

            // Check if "rep" object exists within "nev"
            if (nev.contains("rep")) 
			{
#ifdef oneM2M_tinyIoT
				auto rep = nev["rep"];

				if (rep.contains("m2m:sub"))
				{
					return;
				}
				else if(rep.contains("m2m:cin"))
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

					string building_name = sur_values[1];
					string device_name = sur_values[2];
					string majorCNTName = sur_values[3];

					static std::locale loc("");

					wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
					wstring Wbuilding_name = converter.from_bytes(building_name);
					wstring Wdevice_name = converter.from_bytes(device_name);

					Building* thisBuilding = this->getBuilding(Wbuilding_name);
					Elevator* thisElevator = this->getElevator(thisBuilding, Wdevice_name);

					string con;

					noti_content = new notificationContent;

					if (majorCNTName == "Elevator_physics")
					{
						string minorCNTName = sur_values[4];

						if (minorCNTName.find("Velocity") != string::npos)
						{
							con = rep["m2m:cin"]["con"].get<string>();
							noti_content->current_velocity = stod(con);
						}
						else if (minorCNTName.find("Altimeter") != string::npos)
						{
							con = rep["m2m:cin"]["con"].get<string>();
							noti_content->current_altimeter = stod(con);
						}
						thisElevator->setNotificationContent(noti_content);
					}
					else if (majorCNTName == "Elevator_button_inside")
					{
						string minorCNTName = sur_values[4];

						if (minorCNTName.find("Button_List") != string::npos)
						{
							con = rep["m2m:cin"]["con"].get<string>();

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
						con = rep["m2m:cin"]["con"].get<string>();

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