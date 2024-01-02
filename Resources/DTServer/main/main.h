#pragma once

#include <cpprest/http_client.h>
#include <boost/asio.hpp>

#include "socket_oneM2M.h"
#include "parse_json.h"

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

class DTServer
{
public:
	DTServer();
	void Running();
	socket_oneM2M get_oneM2M_socket_based_on_AE_ID(vector<socket_oneM2M> socket_array, const string AE_ID);

	vector<string> ACP_NAMES;
	vector<socket_oneM2M> oneM2M_sockets;
	send_oneM2M ACP_Validation_Socket;

	boost::asio::io_service io;
	parse_json::parsed_struct parsed_struct;
	std::string device_name;

private:


};