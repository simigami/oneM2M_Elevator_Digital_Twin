#include "main.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"
#include "simulation.h"
#include "elevator.h"
#include "test.h"

#define embedded_port 10050

#define _WIN32_WINNT_WIN10 0x0A00
#define oneM2M_CSE_Server "192.168.0.178"
#define oneM2M_CSE_Port "10051"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::vector;
using std::chrono::system_clock;

int cin_numbering = 1;

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

DTServer::DTServer()
{
}

bool DTServer::exists_elevator(class_of_one_Building one_building, string device_name)
{
	try
	{
		for(const Elevator& elem : one_building.class_of_all_Elevators)
	    {
		    if(elem.device_name == device_name)
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

class_of_one_Building DTServer::get_building_vector(vector<class_of_one_Building> class_of_all_Buildings, string ACOR_NAME)
{
    for(auto elem : class_of_all_Buildings)
    {
	    if(elem.ACP_NAME == ACOR_NAME)
	    {
		    return elem;
	    }
    }
    return class_of_one_Building();
}

Elevator DTServer::get_elevator(class_of_one_Building class_of_one_building, string device_name)
{
	try
	{
		for(auto elem : class_of_one_building.class_of_all_Elevators)
	    {
		    if(elem.device_name == device_name)
		    {
			    return elem;
		    }
	    }
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on DTServer::get_elevator : " << e.what() << endl;
        std::exit(0);
	}
}

socket_oneM2M DTServer::get_oneM2M_socket_based_on_AE_ID(vector<Elevator> elevator_array, const string AE_ID)
{
    for(auto each_elevator_class : elevator_array)
    {
        socket_oneM2M socket = each_elevator_class.sock;
        //std::cout << "Socket Name : " << socket.socket_name << std::endl;
	    if(socket.socket_name == AE_ID)
	    {
		    //std::cout << "Found Socket " << AE_ID << std::endl;
            return socket;
	    }
    }
    std::cout << "Error occured in Class DTServer::get_oneM2M_socket_based_on_AE_ID -> AE_ID Not Found : " << AE_ID << std::endl;
    exit(0);
}

void DTServer::Running()
{
    Elevator* this_Building_Elevator;
    tcp::acceptor acceptor(this->io, tcp::endpoint(tcp::v4(), embedded_port));
    //std::thread timerThread(print_elapsed_time);

	while(true)
    {
        tcp::socket socket(io);
		acceptor.accept(socket);

        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\n");

        std::istream input_stream(&buf);
        string json_data;
        std::getline(input_stream, json_data);

        parse_json c;
        auto parsed_struct = c.parsing(json_data);
        string ACOR_NAME = parsed_struct.building_name;

        string AE_NAME = parsed_struct.building_name;
        string CNT_NAME = parsed_struct.device_name;

        int ACP_FLAG = ACP_Validation_Socket.acp_validate(ACOR_NAME, 0);

        // CHECK THIS ACP Exists
        if(ACP_FLAG == 0 || ACP_FLAG == 1)
        {
            this->ACP_NAMES.push_back(ACOR_NAME);

            // Create THIS AE(Building) Class
            class_of_one_Building temp;

            // Create THIS CNT(Building's Elevator) Class
            this_Building_Elevator = new Elevator(parsed_struct, this->ACP_NAMES);

            temp.class_of_all_Elevators.push_back(*this_Building_Elevator);
            temp.ACP_NAME = ACOR_NAME;

            class_of_all_Buildings.push_back(temp);

            //DISCOVERY TEST
		    //this_Building_Elevator->sock.socket.discovery_retrieve(this_Building_Elevator->sock.originator_name, 0);

            //RCN TEST
            //this_Building_Elevator->sock.socket.result_content_retrieve(this_Building_Elevator->sock.originator_name, 0);

            //THREADING
            this_Building_Elevator->start_thread();
        }
        else
        {
            std::cout << "Building Exists, Check CNT" << CNT_NAME << " Exists..." << std::endl;
            // GET THIS BUILDING CLASS
            class_of_one_Building temp = this->get_building_vector(this->class_of_all_Buildings, ACOR_NAME);

            // CHECK THIS CNT(Device Name) Exists
            bool flag = this->exists_elevator(temp, CNT_NAME);

            if(flag)
            {
	            // THIS CNT Exists
                Elevator This_Elevator = this->get_elevator(temp, CNT_NAME);

                // Update CIN Value
                std::cout << "CNT Found, Updating CIN based on this CNT..." << CNT_NAME << std::endl;
                This_Elevator.sock.create_oneM2M_under_CNTs(parsed_struct);

                //DISCOVERY TEST
		        //This_Elevator.sock.socket.discovery_retrieve(This_Elevator.sock.originator_name, 0);

                //RCN TEST
                //This_Elevator.sock.socket.result_content_retrieve(This_Elevator.sock.originator_name, 0);
            }
            else
            {
	            // THIS CNT NOT Exists
                std::cout << "CNT Not Found, Creating New CNT..." << CNT_NAME << std::endl;

                this_Building_Elevator = new Elevator(parsed_struct, this->ACP_NAMES);

	            temp.class_of_all_Elevators.push_back(*this_Building_Elevator);
	            temp.ACP_NAME = ACOR_NAME;

	            class_of_all_Buildings.push_back(temp);

                //THREADING
				this_Building_Elevator->start_thread();

                //DISCOVERY TEST
				//this_Building_Elevator->sock.socket.discovery_retrieve(this_Building_Elevator->sock.originator_name, 0);

	            //RCN TEST
	            //this_Building_Elevator->sock.socket.result_content_retrieve(this_Building_Elevator->sock.originator_name, 0);
                
	            //this_Building_Elevator->start_thread();
            }
        }
    }
}

int main()
{
    try
    {
    	DTServer digital_twin_server;
    	digital_twin_server.Running();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}