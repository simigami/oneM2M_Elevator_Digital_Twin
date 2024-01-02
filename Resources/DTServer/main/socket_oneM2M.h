#pragma once
<<<<<<< HEAD
#include <map>
=======
>>>>>>> origin/Prototype_Beta
#include <chrono>

#include "send_oneM2M.h"
#include "parse_json.h"

using std::chrono::system_clock;
<<<<<<< HEAD
using std::map;
=======
>>>>>>> origin/Prototype_Beta

class socket_oneM2M
{
public:
	string socket_name = "";
<<<<<<< HEAD
	send_oneM2M socket;

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
=======
>>>>>>> origin/Prototype_Beta

	socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
	~socket_oneM2M();

<<<<<<< HEAD
	bool create_oneM2M_under_device_name(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_under_CNTs(parse_json::parsed_struct parsed_struct);

};
=======
};
>>>>>>> origin/Prototype_Beta
