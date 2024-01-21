#pragma once

#include <boost/asio.hpp>
#include <cpprest/http_client.h>

#include "elevator.h"
#include "parse_json.h"
#include "socket_oneM2M.h"

#define host_protocol "http://"
#define host_ip "192.168.0.178"
#define host_port "10051"

#define DEFAULT_ACP_NAME "DT_SERVER"
#define DEFAULT_ORIGINATOR "CAdmin"
#define DEFAULT_CSE_NAME "tinyIoT"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

struct class_of_one_Building
{
	vector<Elevator> class_of_all_Elevators;
	string ACP_NAME;
};

class DTServer
{
public:
	DTServer();
	void Running();

<<<<<<< HEAD
	bool exists_elevator(class_of_one_Building class_of_one_building, string device_name);

	socket_oneM2M get_oneM2M_socket_based_on_AE_ID(vector<Elevator> elevator_array, const string AE_ID);
	class_of_one_Building get_building_vector(vector<class_of_one_Building> class_of_all_Buildings, string ACOR_NAME);
	Elevator get_elevator(class_of_one_Building class_of_one_building, string device_name);
=======
	socket_oneM2M get_oneM2M_socket_based_on_AE_ID(vector<socket_oneM2M> socket_array, const string AE_ID);
>>>>>>> origin/Prototype_Beta

	vector<string> ACP_NAMES;

	vector<thread> one_building_threads;

	vector<class_of_one_Building> class_of_all_Buildings; //THIS WILL BE USED
	vector<Elevator> class_of_all_Elevators; //THIS WILL BE USED

	vector<socket_oneM2M> oneM2M_sockets; //TBD
	send_oneM2M ACP_Validation_Socket;

	boost::asio::io_service io;
	parse_json::parsed_struct parsed_struct;

private:


};