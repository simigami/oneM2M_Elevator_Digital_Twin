#pragma once
#include <iostream>
#include <cpprest/http_client.h>
#include <chrono>
#include <cpprest/filestream.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "parse_json.h"

#define host_protocol "http://"
#define host_ip "192.168.0.178"
#define host_port "10051"
#define CSE_NAME "tinyIoT"
#define UI "UserInput"
#define SO "SensorOutput"


using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

class DTServer
{
public:
	DTServer(parse_json::parsed_struct parsed_struct);

private:
	bool check_ACP();
	bool check_AE(const std::string& device_name);

	parse_json::parsed_struct paresed_struct;

	std::string device_name;
};