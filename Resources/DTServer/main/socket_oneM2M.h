#pragma once
#include <map>
#include <chrono>

#include "send_oneM2M.h"
#include "parse_json.h"

class socket_oneM2M
{
public:
	send_oneM2M socket;

    wstring TCName;
    wstring originator_name;
	wstring building_name;
    wstring device_name;

	vector<wstring> Default_CNTs;
    vector<wstring> Default_PHYSICS_CNTs;
    vector<wstring> Default_INSIDE_CNTs;
    vector<wstring> Default_OUTSIDE_CNTs;

	socket_oneM2M(elevator_resource_status sc, Wparsed_struct parseStruct, vector<wstring> ACP_NAMES);
	socket_oneM2M(elevator_resource_status sc, Wparsed_struct parseStruct);
	~socket_oneM2M();

	void init(Wparsed_struct parseStruct);

	bool create_oneM2M_under_CNTs(Wparsed_struct parseStruct);

	bool create_oneM2M_CNTs(Wparsed_struct parseStruct);

	bool create_oneM2M_SUBs(Wparsed_struct parseStruct, map<wstring, wstring>* mapper);

	bool write_subscription_mapper_to_file(map<wstring, wstring> mapper);
	bool modify_mapper_to_file(wstring search_string, wstring change_string);

	bool create_oneM2M_CINs(Wparsed_struct parseStruct);

	bool createNewData(Wparsed_struct parseStruct);

	bool create_oneM2M_CIN_EnergyConsumption(Wparsed_struct parseSstruct);

	bool create_oneM2M_CIN_Except_Button_Outside(Wparsed_struct parseSstruct);

	bool create_oneM2M_CIN_Only_Button_Outside(vector<vector<int>> button_outside);

	bool check_oneM2M_CNT(Wparsed_struct parseStruct);

	vector<double> retrieveEachFloorAltimeter();

	vector<vector<wstring>> retrieve_oneM2M_cins(vector<int> floor_info);

	vector<double> retrieve_oneM2M_Energy_CIN(Wparsed_struct parseSstruct);
};