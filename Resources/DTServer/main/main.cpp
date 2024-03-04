#include "main.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"
#include "simulation.h"
#include "elevator.h"
#include "test.h"

#define PORT_EMBEDDED 10050
#define PORT_NOTIFICATION 10053
#define BUFFER_SIZE 1024

#define _WIN32_WINNT_WIN10 0x0A00
#define oneM2M_CSE_Server "192.168.0.178"
#define oneM2M_CSE_Port "10051"

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
        std::cout << "Elapsed Time " << elapsed << "seconds \n";
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

    addedButtonOutsideList = getDiffBetweenCurrentAndEmbedded(button_outside, *(this->currentButtonOutsideInfos));

    if(addedButtonOutsideList.size() == 0)
    {
		cout << "IN SERVER -> NO OUTSIDE CALL" << endl;
		*(this->currentButtonOutsideInfos) = button_outside;
	    return;
    }

	else if(thisBuildingElevators.size() == 1)
    {
		cout << thisBuildingElevators[0]->building_name << " -> " << thisBuildingElevators[0]->device_name << " is SELECTED" <<  endl;
	    thisBuildingElevators[0]->sock->create_oneM2M_CIN_Only_Button_Outside(addedButtonOutsideList);
		*(this->currentButtonOutsideInfos) = button_outside;
		return;
    }

    for(vector<int> eachCall : addedButtonOutsideList)
    {
		vector<vector<int>> temp;

	    calledFloor = addedButtonOutsideList[0][0];
		calledDirection = addedButtonOutsideList[0][1];

    	closestDistanceElevatorAltimeter = 0.0;
    	closestDistanceElevatorAltimeterIDLE = 0.0;

        for (auto elem : thisBuildingElevators)
	    {
		    if(elem->algorithmNumber == 1 || elem->algorithmNumber == 2) // SINGLE ALG
		    {
			    if(elem->thisElevatorAlgorithmSingle->thisFlags->IDLEFlag) // 정지한 엘리베이터들
			    {
	                double dist = abs(elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) - elem->p->current_altimeter);
	                if(closestDistanceElevatorAltimeterIDLE == 0.0)
	                {
	                    closestDistanceElevatorAltimeterIDLE = dist;
	                    selectedElevatorIDLE = elem;
	                }
	                else if(dist < closestDistanceElevatorAltimeterIDLE)
	                {
	                    closestDistanceElevatorAltimeterIDLE = dist;
	                    selectedElevator = elem;
	                }
	                idleElevators.push_back(elem);
				    cout << elem->building_name << " -> " << elem->device_name << " IS IN IDLE..." << endl;
			    }
	            else
	            {
	                if(calledDirection)
	                {
						if(!elem->p->lock) // IF CHOSEN ELEVATOR IS STOPPED AT SOME FLOOR
						{
							
						}
	                    if(elem->p->current_direction)
	                    {
	                        double dist = elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) - 2.5;
	                        if(elem->p->current_altimeter < dist)
	                        {
	                            if(closestDistanceElevatorAltimeter == 0.0)
	                            {
	                                closestDistanceElevatorAltimeter = dist - elem->p->current_altimeter;
	                                selectedElevator = elem;
	                            }
	                            else if(dist - elem->p->current_altimeter < closestDistanceElevatorAltimeter)
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
	                    if(!elem->p->current_direction)
	                    {
	                        double dist = elem->p->floorToAltimeter(calledFloor, elem->p->info.altimeter_of_each_floor) + 2.5;
		                    if(elem->p->current_altimeter > dist)
		                    {
	                            if(closestDistanceElevatorAltimeter == 0.0)
	                            {
	                                closestDistanceElevatorAltimeter = elem->p->current_altimeter - dist;
	                                selectedElevator = elem;
	                            }
	                            else if(elem->p->current_altimeter  - dist < closestDistanceElevatorAltimeter)
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

	    if(selectedElevatorIDLE != NULL)
	    {
	        cout << endl << selectedElevatorIDLE->building_name << " -> " << selectedElevatorIDLE->device_name << " is SELECTED FOR CALL " << calledFloor <<  endl;
			temp.push_back(eachCall);

	    	selectedElevatorIDLE->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
	    }

	    else if(selectedElevator != NULL)
	    {
	        cout << endl << selectedElevator->building_name << " -> " << selectedElevator->device_name << " is SELECTED FOR CALL " << calledFloor <<  endl;
			temp.push_back(eachCall);

			selectedElevator->sock->create_oneM2M_CIN_Only_Button_Outside(temp);
	    }
    }
	*(this->currentButtonOutsideInfos) = button_outside;

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

DTServer::DTServer()
{
}

bool DTServer::existsElevator(Building* one_building, string device_name)
{
    try
	{
		for(const Elevator* elem : one_building->buildingElevatorInfo->classOfAllElevators)
	    {
		    if(elem->device_name == device_name)
		    {
			    return true;
		    }
	    }
        return false;
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on DTServer::get_elevator : " << e.what() << endl;
        exit(0);
	}
}

Building* DTServer::get_building_vector(string ACOR_NAME)
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

Elevator* DTServer::getElevator(Building* class_of_one_building, string device_name)
{
    try
	{
        for(size_t i = 0; i < class_of_one_building->buildingElevatorInfo->classOfAllElevators.size(); i++)
        {
	        if(class_of_one_building->buildingElevatorInfo->classOfAllElevators[i]->device_name == device_name)
	        {
		        return class_of_one_building->buildingElevatorInfo->classOfAllElevators[i];
	        }
        }
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on DTServer::get_elevator : " << e.what() << endl;
        std::exit(0);
	}
}

socket_oneM2M DTServer::get_oneM2M_socket_based_on_AE_ID(vector<Elevator*> elevator_array, const string AE_ID)
{
    for(const Elevator* each_elevator_class : elevator_array)
    {
        socket_oneM2M socket = *(each_elevator_class->sock);
	    if(socket.building_name == AE_ID)
	    {
            return socket;
	    }
    }
    std::cout << "Error occurred in Class DTServer::get_oneM2M_socket_based_on_AE_ID -> AE_ID Not Found : " << AE_ID << endl;
    exit(0);
}

void DTServer::Run()
{
    boost::asio::io_context ioContext;

    tcp::endpoint EMBEDDED_END(tcp::v4(), PORT_EMBEDDED);
    tcp::endpoint NOTIFICATION_END(tcp::v4(), PORT_NOTIFICATION);

    tcp::acceptor acceptor1(ioContext, EMBEDDED_END, PORT_EMBEDDED);
    tcp::acceptor acceptor2(ioContext, NOTIFICATION_END, PORT_NOTIFICATION);
    //std::thread timerThread(print_elapsed_time);

    acceptor1.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_EMBEDDED << endl;
            handleConnection(socket, PORT_EMBEDDED);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_EMBEDDED << ": " << error.message() << endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor1,  ioContext);
    });

    acceptor2.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_NOTIFICATION << endl;
            handleConnection(socket, PORT_NOTIFICATION);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_NOTIFICATION << ": " << error.message() << endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor2,  ioContext);
    });

    ioContext.run();

    // Start accepting connections
    // (You would typically handle connections asynchronously)
}

void DTServer::startAsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_EMBEDDED << endl;
            handleConnection(socket, PORT_EMBEDDED);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_EMBEDDED << ": " << error.message() << endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor, ioContext);
    });
}

void DTServer::startAsyncAccept2(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_NOTIFICATION << endl;
            handleConnection(socket, PORT_NOTIFICATION);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_NOTIFICATION << ": " << error.message() << endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor, ioContext);
    });
}

void DTServer::handleConnection(boost::asio::ip::tcp::socket& socket, int port)
{
    boost::asio::streambuf receiveBuffer(BUFFER_SIZE);
    boost::system::error_code error;
    std::string httpRequest;

    if(port == PORT_NOTIFICATION)
    {
	    //boost::asio::read_until(socket, receiveBuffer, "\0", error);
        boost::asio::read(socket, receiveBuffer, error);

        std::istream inputStream(&receiveBuffer);
        std::ostringstream httpBodyStream;

        // Read the HTTP headers
        while (std::getline(inputStream, httpRequest) && httpRequest != "\r") {
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
        if (port == PORT_EMBEDDED)
        {
            this->Running_Embedded(httpRequest);
        }
    	else if (port == PORT_NOTIFICATION) 
        {
            this->Running_Notification(httpRequest);
        }
    }
	else
    {
        std::cerr << "Error in DTServer::handleConnection : " << error.message() << endl;
        exit(0);
    }
}

void DTServer::Running_Embedded(const std::string& httpResponse)
{
    Elevator* thisBuildingElevator;

    parse_json c;
    const parse_json::parsed_struct parsedStruct = c.parsing(httpResponse);

	int ACP_FLAG;

    string ACOR_NAME = parsedStruct.building_name;
    string AE_NAME = parsedStruct.building_name;
    string CNT_NAME = parsedStruct.device_name;

	std::cout << endl << endl << "FROM EMBEDDED -> SERVER : " << " Receive Data From : " << parsedStruct.device_name << endl;

    ACP_FLAG = ACP_Validation_Socket.acp_validate(ACOR_NAME, 0);

    // CHECK THIS ACP Exists
    if(ACP_FLAG == 0 || ACP_FLAG == 1) // oneM2M에 빌딩 ACP가 존재하지 않는 경우
    {
    	std::cout << "IN SERVER : " << " ACP : C" << parsedStruct.building_name << "Not Exists" << endl;

    	Building* newBuilding;
        this->ACP_NAMES.push_back(ACOR_NAME);

        if(ACOR_NAME == "SejongAI")
        {
	         newBuilding = new Building(1);
        }
        else
        {
	        newBuilding = new Building(1);
        }

        // Create THIS AE(Building) Class
        int algNumber;

        //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
        //cin >> algNumber;

        // TEST
        if(CNT_NAME == "EV1" || CNT_NAME == "EV2")
        {
	        algNumber = 1;
        }
        else
        {
	        algNumber = 2;
        }

        // Create THIS CNT(Building's Elevator) Class
        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

        newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
        newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

        allBuildingInfo.push_back(newBuilding);

        thisBuildingElevator->runElevator();
	    thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct);

        if(newBuilding->getButtonMod())
        {
	        thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);

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
        std::cout << "IN SERVER -> Building ACP Exists, Check Building " << AE_NAME << " Exists In Server" << endl;

        // GET THIS BUILDING CLASS and CHECK THIS CNT(Device Name) Exists
        Building* thisBuildingFlag = this->get_building_vector(ACOR_NAME);

        // if TEMP == NULL, It means oneM2M Building Resource Exists, But Server Restarted
        if(thisBuildingFlag == NULL) // oneM2M에 빌딩 ACP가 존재하는데 서버에 클래스가 없는 경우
        {
            std::cout << "IN SERVER -> Building Resource Not Exist in Server Create New " << AE_NAME << " Resource with " << CNT_NAME << endl;
	        this->ACP_NAMES.push_back(ACOR_NAME);

	        // Create THIS AE(Building) Class
	        Building* newBuilding;

            if(ACOR_NAME == "SejongAI")
	        {
		         newBuilding = new Building(1);
	        }
	        else
	        {
		        newBuilding = new Building(1);
	        }

            int algNumber;
            //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
			//cin >> algNumber;

            if(CNT_NAME == "EV1" || CNT_NAME == "EV2")
	        {
		        algNumber = 1;
	        }
	        else
	        {
		        algNumber = 2;
	        }

	        // Create THIS CNT(Building's Elevator) Class
	        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

	        newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
	        newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

	        allBuildingInfo.push_back(newBuilding);

	        thisBuildingElevator->runElevator();

	        if(newBuilding->getButtonMod())
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

        else  // oneM2M에 빌딩 ACP가 존재하고 서버에도 클래스가 존재하는 경우
        {
        	std::cout << "IN SERVER -> Building Resource Exists, Check Elevator " << CNT_NAME << " Resource Exists IN Server" << endl;

        	bool flag = this->existsElevator(thisBuildingFlag, CNT_NAME);

	        if(flag) // oneM2M과 서버에 해당 엘리베이터 클래스가 모두 존재하는 경우
	        {
	            // THIS CNT Exists
	            Elevator* thisBuildingElevator = this->getElevator(thisBuildingFlag, CNT_NAME);

	            // Update CIN Value
	            std::cout << "IN SERVER -> Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << endl;

                if(thisBuildingFlag->getButtonMod())
		        {
			        thisBuildingElevator->sock->create_oneM2M_CIN_Except_Button_Outside(parsedStruct);
					thisBuildingFlag->getWhichElevatorToGetButtonOutside(parsedStruct.button_outside);
		            // SELECT WHICH ELEVATOR TO GIVE BUTTON OUTSIDE INFO
		        }

		        else
		        {
			        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
		        }

	            //DISCOVERY TEST
		        //This_Elevator.sock.socket.discovery_retrieve(This_Elevator.sock.originator_name, 0);

	            //RCN TEST
	            //This_Elevator.sock.socket.result_content_retrieve(This_Elevator.sock.originator_name, 0);
	        }
	        else // oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
	        {
                //CHECK THIS CNT EXISTS in oneM2M
                string originator_name = "C" + AE_NAME;

        		ACP_Validation_Socket.URL_TO_AE = ACP_Validation_Socket.URL_TO_CSE + "/" + AE_NAME;

        		bool flag = ACP_Validation_Socket.cnt_validate(originator_name, 1, CNT_NAME); //수정 필요, 엘리베이터 CNT 존재하는 지 확인

                if(!flag) // oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
                {
	                // THIS CNT NOT Exists in DT Server and oneM2M
		            std::cout << "IN SERVER -> Elevator Not Found, Creating New Elevator : " << CNT_NAME << endl;

	                int algNumber;
		            //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
					//cin >> algNumber;

	                if(CNT_NAME == "EV1" || CNT_NAME == "EV2")
			        {
				        algNumber = 1;
			        }
			        else
			        {
				        algNumber = 2;
			        }

		            // Create THIS CNT(Building's Elevator) Class
			        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

			        thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);

			        thisBuildingElevator->runElevator();
				    thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct);

			        if(thisBuildingFlag->getButtonMod())
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
	                std::cout << "IN SERVER -> Elevator Resource Not Exist in DT Server... Create New CNT " << CNT_NAME << " Resource" << endl;
			        this->ACP_NAMES.push_back(ACOR_NAME);

		            int algNumber;
		            //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
					//cin >> algNumber;

		            if(CNT_NAME == "EV1" || CNT_NAME == "EV2")
			        {
				        algNumber = 1;
			        }
			        else
			        {
				        algNumber = 2;
			        }

			        // Create THIS CNT(Building's Elevator) Class
                    thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

			        thisBuildingFlag->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
			        thisBuildingFlag->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

			        thisBuildingElevator->runElevator();

			        if(thisBuildingFlag->getButtonMod())
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

void DTServer::Running_Notification(const std::string& httpResponse)
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
                auto rep_json = nlohmann::json::parse(nev["rep"].get<std::string>());

                if (rep_json.contains("m2m:sub")) 
				{
					return ;
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

					string building_name = sur_values[1];
					string device_name = sur_values[2];
					string majorCNTName = sur_values[3];

                    Building* thisBuilding = this->get_building_vector(building_name);
                    Elevator* thisElevator = this->getElevator(thisBuilding, device_name);

					string con;
					std::string rep_value_str = json_body["m2m:sgn"]["nev"]["rep"];

			        // Parse the value of "rep" as JSON
					nlohmann::json rep_value_json = nlohmann::json::parse(rep_value_str);

                    tempContentMoveToAlgorithm = new notificationContent;

					if(majorCNTName == "Elevator_physics")
					{
						string minorCNTName = sur_values[4];

						if(minorCNTName.find("Velocity") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
                            tempContentMoveToAlgorithm->velocity = stod(con);
						}
						else if(minorCNTName.find("Altimeter") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
							tempContentMoveToAlgorithm->altimeter = stod(con);
						}
                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
					}
					else if(majorCNTName == "Elevator_button_inside")
					{
						string minorCNTName = sur_values[4];

						if(minorCNTName.find("Button_List") != string::npos)
						{
				            con = rep_value_json["m2m:cin"]["con"].get<string>();

							std::vector<int> integers;
							std::istringstream iss(con);
						    std::string token;
						    while (iss >> token) 
							{
                                // Convert each token (integer string) to an integer and store it in the vector
                                if(token[0] == 'B')
                                {
	                                integers.push_back(-1 * stoi(token.substr(1)));
                                }

                                else
                                {
	                                integers.push_back(stoi(token));
                                }
							}
                            if(integers[0] == -1)
                            {
	                            cout << "" <<endl;
                            }
							tempContentMoveToAlgorithm->button_inside_list = integers;
						}
						else if(minorCNTName.find("goTo") != string::npos)
						{
					        con = rep_value_json["m2m:cin"]["con"].get<string>();
							tempContentMoveToAlgorithm->goTo = stoi(con);
						}

                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
                        thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
					else if(majorCNTName == "Elevator_button_outside")
					{
						string minorCNTName = sur_values[4];
						con = rep_value_json["m2m:cin"]["con"].get<string>();

				        int called_floor;
				        bool direction;

                        //가정 1. 바깥에 누른 층은 한번 누르면 끝
						if(con == "None") //바깥의 층에 도달하여 자연스럽게 사라진 경우 None
						{
							return ;
						}
				        else if(con == "Up")
				        {
					        direction = true;
				        }
				        else
				        {
					        direction = false;
				        }
						if(!minorCNTName.empty() && minorCNTName[0] == 'B')
				        {
					        called_floor = (minorCNTName[minorCNTName.size()-1] - '0') * -1;
							tempContentMoveToAlgorithm->added_button_outside_altimeter = thisElevator->p->info.altimeter_of_each_floor[called_floor+thisElevator->p->info.underground_floor];
				        }
				        else
				        {
					        called_floor = stoi(minorCNTName);
							tempContentMoveToAlgorithm->added_button_outside_altimeter = thisElevator->p->info.altimeter_of_each_floor[called_floor+thisElevator->p->info.underground_floor-1];
				        }
						tempContentMoveToAlgorithm->added_button_outside_floor = called_floor;
						tempContentMoveToAlgorithm->added_button_outside_direction = direction;

                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
                        thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
                }
            }
			else 
			{
                std::cout << "rep field does not exist within nev." << endl;
				exit(0);
            }
        }
		else 
		{
            std::cout << "m2m:sgn field does not exist." << endl;
			exit(0);
        }
	}
	catch (std::exception e)
	{
		std::cerr << "Error in DTServer::Running_Notification : " << e.what() << endl;
	}
}

int main()
{
    try
    {
    	DTServer digitalTwinServer;
    	digitalTwinServer.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << endl;
    }
    return 0;
}