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
            // CHECK THIS AE(Device Name) Exists
	        std::cout << "Building Socket Exists, Update CIN Based on Socket..." << json_data << std::endl;
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