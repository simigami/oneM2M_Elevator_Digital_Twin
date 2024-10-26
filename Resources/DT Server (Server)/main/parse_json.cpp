#include "parse_json.h"

#include <codecvt>
#include <locale>
#include <sstream>
#include <regex>

using namespace std;
using json = nlohmann::json;

Wparsed_struct parse_json::parsingWithBulidingAlgorithms(string httpRequestBody, int algorithm_number)
{
	switch (algorithm_number)
	{
		case 0:
			return parsingOnlyBuildingName(httpRequestBody);
		case 1:
			return parsingCrowdControlButtonBuliding(httpRequestBody);
		default:
			return parsingOnlyBuildingName(httpRequestBody);
	}
}

Wparsed_struct parse_json::parsingOnlyBuildingName(string json_data)
{
	nlohmann::json parsed_json = nlohmann::json::parse(json_data);
	try
	{
		p.building_name = stringToWstring(parsed_json["building_name"]);
		p.device_name = stringToWstring(parsed_json["device_name"]);

		// Parse each array element to p.each_floor_altimeter
		if (parsed_json["each_floor_altimeter"] != nullptr)
		{
			json altimeterArray = parsed_json["each_floor_altimeter"];
			for (const nlohmann::basic_json<>& element : altimeterArray) {
				p.each_floor_altimeter.push_back(element);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
	return p;
}

Wparsed_struct parse_json::parsingDedicatedButtonBuilding(string json_data)
{
	nlohmann::json parsed_json = nlohmann::json::parse(json_data);
	try
	{
		p.building_name = stringToWstring(parsed_json["building_name"]);
		p.device_name = stringToWstring(parsed_json["device_name"]);
		p.underground_floor = stoi(parsed_json.at("underground_floor").get<string>());
		p.ground_floor = stoi(parsed_json.at("ground_floor").get<string>());
		p.timestamp = stringToWstring(parsed_json["timestamp"]);
		p.velocity = stod(parsed_json.at("velocity").get<string>());
		p.altimeter = stod(parsed_json.at("altimeter").get<string>());
		p.temperature = stod(parsed_json.at("temperature").get<string>());


		if (parsed_json["button_outside"] != nullptr)
		{
			vector<vector<int>> temp = parsed_json.at("button_outside").get<vector<vector<int>>>();

			for (const auto& elem : temp)
			{
				p.button_outside.push_back(elem);
			}
		}

		if (parsed_json["button_inside"] != nullptr)
		{
			vector<int> temp = parsed_json.at("button_inside").get<vector<int>>();

			for (const auto& elem : temp)
			{
				wstring temp = to_wstring(elem);
				if (elem < 0)
				{
					temp[0] = L'B';
				}
				p.button_inside.push_back(temp);
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
	return p;
}

Wparsed_struct parse_json::parsingCrowdControlButtonBuliding(string json_data)
{
	json parsed_json = json::parse(json_data);
	Wparsed_struct p;
	try
	{
		const auto p_building_name = parsed_json["building name"];
		const auto p_ev_name = parsed_json["elevator name"];
		const auto p_underground_floor = parsed_json["underground floor"];
		const auto p_init_floor = parsed_json["init floor"];
		const auto p_ground_floor = parsed_json["ground floor"];
		const auto p_timestamp = parsed_json["timestamp"];
		const auto p_max_velocity = parsed_json["max velocity"];
		const auto p_acceleration = parsed_json["acceleration"];
		const auto p_velocity = parsed_json["velocity"];
		const auto p_altimeter = parsed_json["altimeter"];
		const auto p_temperature = parsed_json["temperature"];
		const auto p_button_inside = parsed_json["button inside"];
		const auto p_button_outside = parsed_json["button outside"];
		const auto p_button_outside_direction = parsed_json["button outside direction"];
		const auto p_each_floor_altimeter = parsed_json["each floor altimeter"];
		const auto p_E_IDLE = parsed_json["idle energy"];
		const auto p_E_Standby = parsed_json["standby energy"];
		const auto p_E_Ref = parsed_json["ref energy"];

		if(p_building_name != nullptr)
		{
			p.building_name = stringToWstring(p_building_name.get<string>());
		}
		if (p_ev_name != nullptr)
		{
			p.device_name = stringToWstring(p_ev_name.get<string>());
		}

		if (p.device_name == L"OUT")
		{
			p.timestamp = stringToWstring(parsed_json["timestamp"]);

			if (parsed_json["button outside"] != nullptr)
			{
				json buttonOutsideArray = parsed_json["button outside"];

				// Iterate over the elements of the JSON array and store them
				for (const nlohmann::basic_json<>& element : buttonOutsideArray) {
					// Extract individual elements within the nested array
					vector<int> temp = { element[0], element[1] };
					p.button_outside.push_back(temp);
				}
			}
		}
		else 
		{
			if (p_underground_floor != nullptr)
			{
				p.underground_floor = p_underground_floor.get<int>();
			}
			if (p_ground_floor != nullptr)
			{
				p.ground_floor = p_ground_floor.get<int>();
			}
			if (p_init_floor != nullptr)
			{
				p.init_floor = p_init_floor.get<int>();
			}
			if (p_timestamp != nullptr)
			{
				p.timestamp = stringToWstring(p_timestamp.get<string>());
			}
			if (p_velocity != nullptr)
			{
				p.velocity = p_velocity.get<double>();
			}
			if (p_altimeter != nullptr)
			{
				p.altimeter = p_altimeter.get<double>();
			}
			if (p_temperature != nullptr)
			{
				p.temperature = p_temperature.get<double>();
			}
			if (p_max_velocity != nullptr)
			{
				p.max_velocity = p_max_velocity.get<double>();
			}
			if (p_acceleration != nullptr)
			{
				p.acceleration = p_acceleration.get<double>();
			}
			if (p_E_IDLE != nullptr)
			{
				p.idle_power = p_E_IDLE.get<double>();
			}
			if (p_E_Standby != nullptr)
			{
				p.standby_power = p_E_Standby.get<double>();
			}
			if (p_E_Ref != nullptr)
			{
				p.iso_power = p_E_Ref.get<double>();
			}
		}

		if (p_button_inside != nullptr)
		{
			vector<int> temp = p_button_inside.get<vector<int>>();
			for (const auto& elem : temp)
			{
				wstring temp = to_wstring(elem);
				if (elem < 0)
				{
					temp[0] = L'B';
				}
				p.button_inside.push_back(temp);
			}
		}
		if (p_each_floor_altimeter != nullptr)
		{
			vector<double> temp = p_each_floor_altimeter.get<vector<double>>();
			for (const auto& elem : temp)
			{
				p.each_floor_altimeter.push_back(elem);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error parsing JSON: " << e.what() << std::endl;
	}
	return p;
}

Wparsed_struct parse_json::parsingText(wstring text)
{
	Wparsed_struct retStruct;
	vector<wstring> eachLineToken = splitText(text, L' ');

	if(eachLineToken.size() >= 8)
	{
		retStruct.building_name = eachLineToken[0];
        retStruct.device_name = eachLineToken[1];

        retStruct.underground_floor = stoi(eachLineToken[2]);
        retStruct.ground_floor = stoi(eachLineToken[3]);

        retStruct.timestampOffset = stoi(eachLineToken[4]);

        retStruct.velocity = stod(eachLineToken[5]);
        retStruct.altimeter = stod(eachLineToken[6]);
        retStruct.temperature = stod(eachLineToken[7]);
	}

	if(eachLineToken.size() > 9)
	{
		if(eachLineToken[8] != L"[]")
		{
			wstring bi = eachLineToken[8].substr(1, eachLineToken[8].length() - 2); // Remove '[' and ']'
	        if (!bi.empty()) 
			{
	            vector<wstring> bi_tokens = splitText(bi, L',');
	            for (auto& t : bi_tokens)
				{
	                retStruct.button_inside.push_back(t);
	            }
	        }
		}

		if(eachLineToken[9] != L"[]")
		{
			// Parsing button_outside
	        wstring bo = eachLineToken[9].substr(1, eachLineToken[9].length() - 2); // Remove '[[', ']]'
	        if (!bo.empty()) 
			{
	            vector<wstring> bo_tokens = splitText(bo, L'.');
	            if (!bo_tokens.empty()) 
				{
					for(const auto& elem : bo_tokens)
					{
						wstring bo_sub = elem.substr(1, elem.length() - 2); // Remove '[' and ']'
						vector<wstring> eachBoTokens = splitText(bo_sub, L',');
						vector<int> temp;

						temp.push_back(stoi(eachBoTokens[0]));
						temp.push_back(eachBoTokens[1] == L"True" ? 1 : 0);

						retStruct.button_outside.push_back(temp);
					}
	            }
	        }
		}
	}

	return retStruct;
}

wstring parse_json::stringToWstring(const string& str) const
{
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

	return converter.from_bytes(str);
}

vector<wstring> parse_json::splitText(const wstring& s, wchar_t delimiter)
{
	vector<wstring> tokens;
    wstring token;
    wistringstream tokenStream(s);

    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void parse_json::modify_outside_data_to_oneM2M(Wparsed_struct p)
{
		std::wcout << L"Device_name: " << p.device_name << std::endl;
        std::wcout << L"Timestamp: " << p.timestamp << std::endl;
        std::wcout << L"Altimeter: " << p.altimeter << std::endl;
        std::wcout << L"Temperature: " << p.temperature << std::endl;

		std::wcout << L"Button Outside: ";
        std::wcout << std::endl;
}

vector<int> parse_json::parse_content_type(const std::string& content_type)
{
	std::regex tyRegex(R"(ty=(\d+))");
	std::regex enRegex(R"(en=(\d+))");

	std::smatch match;

	vector<int> result;

	if (std::regex_search(content_type, match, tyRegex)) {
		if (match.size() == 2) { // match[0] is the whole match, match[1] is ty, match[2] is en
			result.push_back(std::stoi(match.str(1)));
		}
	}

	if (std::regex_search(content_type, match, enRegex)) {
		if (match.size() == 2) { // match[0] is the whole match, match[1] is ty, match[2] is en
			result.push_back(std::stoi(match.str(1)));
		}
	}

	return result;
}

int parse_json::parse_M2M_RI(const std::string& content_type)
{
	std::regex RIRegex(R"(^X-M2M-RI:\s*(\w+))");

	std::smatch match;

	vector<int> result;

	if (std::regex_search(content_type, match, RIRegex)) {
		if (match.size() == 2) { // match[0] is the whole match, match[1] is ty, match[2] is en
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void parse_json::modify_inside_data_to_oneM2M(Wparsed_struct p)
{
		std::wcout << L"Device_name: " << p.device_name << std::endl;
        std::wcout << L"Timestamp: " << p.timestamp << std::endl;
        std::wcout << L"Altimeter: " << p.altimeter << std::endl;
        std::wcout << L"Temperature: " << p.temperature << std::endl;

        std::wcout << "Button Inside: ";
        for (const auto &button : p.button_inside) {
            std::wcout << button << " ";
        }
		std::wcout << std::endl;
}
