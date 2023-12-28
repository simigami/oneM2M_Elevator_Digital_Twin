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
		p.timestamp = parsed_json["timestamp"];
		p.altimeter = parsed_json["altimeter"];
		p.temperature = parsed_json["temperature"];

		if(parsed_json["button_inside"] != nullptr)
		{
			p.button_inside = parsed_json["button_inside"];
		}
		if(parsed_json["button_outside"] != nullptr)
		{
			p.button_outside = parsed_json["button_outside"];
		}

		// Print the parsed values
		return p;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
}

void parse_json::modify_inside_data_to_oneM2M(parsed_struct p)
{
		std::cout << "Device_name: " << p.device_name << std::endl;
        std::cout << "Timestamp: " << p.timestamp << std::endl;
        std::cout << "Altimeter: " << p.altimeter << std::endl;
        std::cout << "Temperature: " << p.temperature << std::endl;

		std::cout << "Button Outside: ";
		for (const auto &button : p.button_outside) {
            std::cout << button << " ";
        }
        std::cout << std::endl;
}

void parse_json::modify_outside_data_to_oneM2M(parsed_struct p)
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
