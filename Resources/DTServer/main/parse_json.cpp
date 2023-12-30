#include "parse_json.h"

using json = nlohmann::json;

parse_json::parsed_struct parse_json::parsing(string json_data)
{
	try
	{
		json parsed_json = json::parse(json_data);
		parsed_struct p;

		p.building_name = parsed_json["building_name"];
		p.device_name = parsed_json["device_name"];

		p.underground_floor = parsed_json["underground_floor"];
		p.ground_floor = parsed_json["ground_floor"];

		p.timestamp = parsed_json["timestamp"];

		p.velocity = parsed_json["velocity"];
		p.altimeter = parsed_json["altimeter"];
		p.temperature = parsed_json["temperature"];

		if(parsed_json["button_inside"] != nullptr)
		{
			p.button_inside = parsed_json["button_inside"];
		}
		if(parsed_json["button_outside"] != nullptr)
		{
			json buttonOutsideArray = parsed_json["button_outside"];

			 // Iterate over the elements of the JSON array and store them
			for (const nlohmann::basic_json<>& element : buttonOutsideArray) {
            // Extract individual elements within the nested array
	            vector<int> temp = {element[0], element[1]};
				p.button_outside.push_back(temp);
			}
		}
		// Print the parsed values
		return p;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
}

void parse_json::modify_outside_data_to_oneM2M(parsed_struct p)
{
		std::cout << "Device_name: " << p.device_name << std::endl;
        std::cout << "Timestamp: " << p.timestamp << std::endl;
        std::cout << "Altimeter: " << p.altimeter << std::endl;
        std::cout << "Temperature: " << p.temperature << std::endl;

		std::cout << "Button Outside: ";
        std::cout << std::endl;
}

void parse_json::modify_inside_data_to_oneM2M(parsed_struct p)
{
		std::cout << "Device_name: " << p.device_name << std::endl;
        std::cout << "Timestamp: " << p.timestamp << std::endl;
        std::cout << "Altimeter: " << p.altimeter << std::endl;
        std::cout << "Temperature: " << p.temperature << std::endl;

        std::cout << "Button Inside: ";
        for (const auto &button : p.button_inside) {
            std::cout << button << " ";
        }
		std::cout << std::endl;
}
