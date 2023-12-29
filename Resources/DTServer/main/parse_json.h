#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

using std::string;
using std::vector;

class parse_json
{
private:

public:
	struct parsed_struct
	{
		string building_name;
		string device_name;
		string timestamp;

		int underground_floor = -1;
		int ground_floor = -1;
		int button_outside_floor;

		float velocity = -1;
		float altimeter = -1;
		float temperature = -1;

		bool button_outside_direction;
		bool inFlag = true;
		bool outFlag = true;

		vector<string> button_inside;
	};

	parsed_struct parsing(string json_data);
	void modify_inside_data_to_oneM2M(parsed_struct p);
	void modify_outside_data_to_oneM2M(parsed_struct p);

};