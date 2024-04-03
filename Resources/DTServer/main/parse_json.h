#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

struct Wparsed_struct
{
	wstring TCName = L"";

	wstring building_name = L"";
	wstring device_name = L"";
	wstring timestamp = L"";

	int underground_floor = 0;
	int ground_floor = 0;

	int timestampOffset = 0;

	double velocity = 0.0;
	double altimeter = 0.0;
	double temperature = 0.0;

	double idle_power = 0.0;
	double standby_power = 0.0;
	double iso_power = 0.0;

	bool button_outside_direction;
	bool inFlag = true;
	bool outFlag = true;

	vector<double> each_floor_altimeter = {
	-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
	};

	vector<wstring> button_inside = vector<wstring>{};
	vector<vector<int>> button_outside = vector<vector<int>>{};
};

class parse_json
{
public:
	Wparsed_struct p;

	Wparsed_struct parsingWithBulidingAlgorithms(wstring json_data, int algorithm_number);

	Wparsed_struct parsingOnlyBuildingName(wstring json_data);
	Wparsed_struct parsingDedicatedButtonBuilding(wstring json_data);
	Wparsed_struct parsingCCButtonBuliding(wstring json_data);

	Wparsed_struct parsingText(wstring text);

	wstring stringToWstring(const std::string& str)const;

	vector<wstring> splitText(const wstring& s, wchar_t delimiter);

	void modify_inside_data_to_oneM2M(Wparsed_struct p);
	void modify_outside_data_to_oneM2M(Wparsed_struct p);

};