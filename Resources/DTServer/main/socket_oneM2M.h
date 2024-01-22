#pragma once
#include <map>

#include <chrono>

#include "send_oneM2M.h"
#include "parse_json.h"

using std::chrono::system_clock;
using std::map;

class socket_oneM2M
{
public:
	string socket_name = "";

	send_oneM2M socket;

	string ACOR_NAME;
    string originator_name;
	string building_name;
    string device_name;

	vector<string> Default_CNTs;
    vector<string> Default_PHYSICS_CNTs;
    vector<string> Default_INSIDE_CNTs;
    vector<string> Default_OUTSIDE_CNTs;

	map<string, string> RI_Dict_Velocity;
	map<string, string> RI_Dict_Altimeter;
	map<string, string> RI_Dict_Temperature;
	map<string, string> RI_Dict_Button_outside;
	map<string, string> RI_Dict_Button_inside_panel;
	map<string, string> RI_Dict_Button_inside_goto;

	socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
	~socket_oneM2M();

	bool create_oneM2M_under_device_name(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_under_CNTs(parse_json::parsed_struct parsed_struct);

	vector<vector<string>> retrieve_oneM2M_cins(vector<int> floor_info);

};