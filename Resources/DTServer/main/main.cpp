#include "main.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"

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
    // Check Default ACP Exists
    if(!this->ACP_Validation_Socket.acp_validate(0))
    {
        std::cout << "NO Default ACP Found, Creating New Default ACP..." << std::endl;
	    // Make Default ACP at CSE BASE
        ACP_Validation_Socket.acp_create(0);
    }
    else
    {
	    std::cout << "Default ACP Found..." << std::endl;
    }
}

void DTServer::Running()
{
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

        // CHECK THIS ACP Exists
        if(!ACP_Validation_Socket.acp_validate(1, ACOR_NAME))
        {
	        std::cout << "NO Building ACP Socket For Building : " << parsed_struct.building_name << std::endl;
	        std::cout << "Creating New oneM2M Socket For this Building" << std::endl;

            this->ACP_NAMES.push_back(ACOR_NAME);
            socket_oneM2M oneM2M_socket(parsed_struct, this->ACP_NAMES);
			this->oneM2M_sockets.push_back(oneM2M_socket);
        }
        else
        {
<<<<<<< HEAD
            // CHECK THIS CNT(Device Name) Exists
            string AE_NAME = parsed_struct.building_name;
            string CNT_NAME = parsed_struct.device_name;
            socket_oneM2M This_AE_Socket = this->get_oneM2M_socket_based_on_AE_ID(this->oneM2M_sockets, AE_NAME);

        	std::cout << "Building Socket Exists, Check this CNT Exists..." << CNT_NAME << std::endl;
        	if(!This_AE_Socket.socket.cnt_validate(parsed_struct, 1, AE_NAME))
            {
	            // Create New CNT
                std::cout << "CNT Not Found, Creating New CNT..." << CNT_NAME << std::endl;
                This_AE_Socket.create_oneM2M_under_device_name(parsed_struct);
            }
            else
            {
	            // Update CIN Value
                std::cout << "CNT Found, Updating CIN based on this CNT..." << CNT_NAME << std::endl;
                This_AE_Socket.create_oneM2M_under_CNTs(parsed_struct);
            }
        }
    }
}

socket_oneM2M DTServer::get_oneM2M_socket_based_on_AE_ID(vector<socket_oneM2M> socket_array, const string AE_ID)
{
    for(auto socket : socket_array)
    {
	    if(socket.socket_name == AE_ID)
	    {
		    std::cout << "Found Socket Name : " << AE_ID << std::endl;
            return socket;
	    }
    }
    std::cout << "Error occured in Class DTServer::get_oneM2M_socket_based_on_AE_ID -> AE_ID Not Found : " << AE_ID << std::endl;
    exit(0);
=======
            // CHECK THIS AE(Device Name) Exists
	        std::cout << "Building Socket Exists, Update CIN Based on Socket..." << json_data << std::endl;
        }
    }
>>>>>>> origin/Prototype_Beta
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