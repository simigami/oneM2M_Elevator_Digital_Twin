#include "main.h"
#include "parse_json.h"
#include "send_oneM2M.h"

#define _WIN32_WINNT_WIN10 0x0A00
#define oneM2M_CSE_Server "192.168.0.178"
#define oneM2M_CSE_Port "10051"

using namespace boost::asio;
using ip::tcp;
using std::string;

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

DTServer::DTServer(parse_json::parsed_struct parsed_struct) //Initialize DT Server 
{
    send_oneM2M s(parsed_struct);

    string building_name = parsed_struct.building_name;
    string originator_name = "C" + parsed_struct.device_name;
    string ACP_NAME = building_name + "ACP";

    //Check ACP, if no ACP then make ACP
    std::cout << "Checking This Building ACP\n"  << std::endl;
    try
    {
        if(!s.acp_validate())
        {
	        std::cout << "No ACP Found, Create ACP, AE, CNT, CIN Based on This JSON\n"  << std::endl;
    		s.acp_create(parsed_struct);
    		s.ae_create(parsed_struct);

            string device_name = parsed_struct.device_name;
        	s.cnt_create(parsed_struct, 1, device_name);
        	s.cnt_create(parsed_struct, 2, device_name, "Elevator_physics");
        	s.cnt_create(parsed_struct, 2, device_name, "Elevator_button_inside");
        	s.cnt_create(parsed_struct, 2, device_name, "Elevator_button_outside");
    		//s.cin_create(parsed_struct);
        }
        else
        {
	        std::cout << "ACP Found, Create New CIN Based on This JSON,,,\n"  << std::endl;
            s.cin_create(parsed_struct);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
    }
    //Check Each AE Exists based on DB, if no AE then make AE (in DEV)

    //Check Latest CIN timestamp based on latest log, if new CIN then get new CIN and calculate logic312262
}

int main()
{
    try
    {
    	boost::asio::io_service io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 10050));
        std::thread timerThread(print_elapsed_time);

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

            if(parsed_struct.button_inside.empty())
			{
				std::cout << "Received Elevator Outside Data" << json_data << std::endl;

                // Make ACP for Originator name Device_name
			}
			else if(parsed_struct.button_outside.empty())
			{
				std::cout << "Received Elevator Inside Data" << json_data << std::endl;

                string originator;

                DTServer DTServer(parsed_struct);
			}

        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}