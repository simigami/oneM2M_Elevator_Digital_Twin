#pragma once
#include "FileSystem.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

enum LabelRepresentation {
	BUILDINGNAME = 1,
	ELEVATORNAME,
	TIMESTAMP,
	VALUE,
	FLOOR,
	PHYSICS,
	TEMPERATURE
};

class DTLabeling
{
public:
	DTLabeling();
	void run();
	void GetSortKeyStartToEnd(string CSVPath, int SortId);

	FileSystem fs;
	std::map<int, std::string> ColMaps;

private:
    std::map<std::string, std::string> LabelMaps{
        {"1", "Morning"},
        {"2", "Afternoon"},
        {"3", "Evening"},
        {"4", "Spring"},
        {"5", "Summer"},
        {"6", "Fall"},
        {"7", "Winter"},
        {"8", "Night"},
        {"9", "Weekday"},
        {"10", "Weekend"},
        {"11", "High Energy Usage"},
        {"12", "Average Energy Usage"},
        {"13", "Low Energy Usage"},
        {"14", "High Traffic"},
        {"15", "Average Traffic"},
        {"16", "Low Traffic"},
        {"17", "Event"},
        {"18", "High Temperature"},
        {"19", "Average Temperature"},
        {"20", "Low Temperature"},
        {"21", "High Humidity"},
        {"22", "Average Humidity"},
        {"23", "Low Humidity"},
        {"24", "High CO2"},
        {"25", "Average CO2"},
        {"26", "Low CO2"},
        {"27", "High Light"},
        {"28", "Average Light"},
        {"29", "Low Light"},
        {"30", "High Noise"},
        {"31", "Average Noise"},
        {"32", "Low Noise"}
    };
    bool CheckTimeRange(vector<int> Start, vector<int> End, vector<int> Time);
};