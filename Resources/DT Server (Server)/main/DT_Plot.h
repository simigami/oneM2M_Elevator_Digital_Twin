#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <numeric>
#include <map>
#include <unordered_map>
#include "FileSystem.h"

enum column_name {
	COL_BLDG,
	COL_EV,
	COL_DATETIME,
	COL_YEAR,
	COL_MONTH,
	COL_DAY,
	COL_HOUR,
	COL_MINUTE,
	COL_SECOND,
	COL_OPTIME,
	COL_VELOCITY,
	COL_ALTIMETER,
	COL_ERD,
	COL_ESD,
	COL_ED,
	COL_LABELS
};

class dt_plot {
public:
	std::map<std::string, std::unordered_map<std::string, std::vector<double>>> extractED(const std::vector<std::vector<std::string>>& data);
	void run();
	void ReadFile(std::string CSVFilePath);

private:
	FileSystem fs;
	std::vector<std::string> colors = { "red", "green", "blue", "cyan", "magenta", "yellow" };
	int color_idx = 0;
};