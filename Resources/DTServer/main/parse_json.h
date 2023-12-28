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

		float altimeter;
		float temperature;

		bool inFlag = true;
		bool outFlag = true;

		vector<std::string> button_inside;
		vector<std::string> button_outside;
	};

	parsed_struct parsing(string json_data);
	void modify_inside_data_to_oneM2M(parsed_struct p);
	void modify_outside_data_to_oneM2M(parsed_struct p);

};