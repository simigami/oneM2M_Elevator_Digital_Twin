#pragma once
#include <chrono>

#include "send_oneM2M.h"
#include "parse_json.h"

using std::chrono::system_clock;

class socket_oneM2M
{
public:
	string socket_name = "";

	socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
	~socket_oneM2M();

};
