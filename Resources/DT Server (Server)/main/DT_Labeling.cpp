#include "DT_Labeling.h"
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <set>

using namespace std;

DTLabeling::DTLabeling()
{
}

void DTLabeling::run()
{
	const std::string CSVPath = fs.chooseLog();

	// OPEN CSV FILE
	std::ifstream ifs(CSVPath);
	if (!ifs.is_open())
	{
		std::cerr << "Error: File not found" << std::endl;
		return;
	}

	std::string line;
	std::getline(ifs, line);
	std::istringstream ss(line);
	std::string token;

	int index = 0;
	while (std::getline(ss, token, ','))
	{
		ColMaps[index] = token;
		index++;
	}

	ifs.close();

	int ColNum = -1;
	std::cout << "Select A Column Number to Get Specific Label Range: " << endl;
	for (auto &col : ColMaps)
	{
		std::cout << col.first << " : " << col.second << endl;
	}

	cin >> ColNum;
	while (ColMaps.find(ColNum) == ColMaps.end())
	{
		std::cout << "Invalid Column Number. Please Enter Again: " << endl;
		cin >> ColNum;
	}

	int RepresentNumber = -1;
	std::cout << "Select A Label Representation Number: " << endl;
	std::cout << "1 : BUILDING NAME" << endl;
	std::cout << "2 : ELEVATOR NAME" << endl;
	std::cout << "3 : TIMESTAMP" << endl;
	std::cout << "4 : VALUE" << endl;
	std::cout << "5 : FLOOR" << endl;
	std::cout << "6 : PHYSICS" << endl;
	std::cout << "7 : TEMPERATURE" << endl;
	cin >> RepresentNumber;
	while (RepresentNumber < 1 || RepresentNumber > 7)
	{
		std::cout << "Invalid Label Representation Number. Please Enter Again: " << endl;
		cin >> RepresentNumber;
	}

	GetSortKeyStartToEnd(CSVPath, RepresentNumber);
}

void DTLabeling::GetSortKeyStartToEnd(string CSVPath, int SortId)
{
	std::string labelInfo = "";
	std::cout << "Enter a Label Number To Put Label On" << endl;
	std::cout << "Enter 'help' or 'h' to See All Label Info : ";

	std::cin >> labelInfo;

	if(labelInfo == "help" || labelInfo == "h")
	{
		for(auto &col : LabelMaps)
		{
			std::cout << col.first << " : " << col.second << std::endl;
		}
		std::cout << "Enter a Label Number To Put Label On" << endl;
		std::cout << "Enter 'help' or 'h' to See All Label Info : ";

		std::cin >> labelInfo;
	}

	// Input Again if labelInfo is not in LabelMaps
	while (LabelMaps.find(labelInfo) == LabelMaps.end())
	{
		std::cout << "Invalid Label Number. Please Enter Again: " << std::endl;
		std::cin >> labelInfo;
	}

	switch (SortId)
	{
		case BUILDINGNAME:
		{
			std::string BldgName = "";
			std::cout << "Enter a Single Building Name To Put This Label On: ";
			std::cin >> BldgName;

			// GET A index of SortId using ColNum
			int ColNum = -1;
			for (auto &col : ColMaps)
			{
				if (col.second == "Building Name")
				{
					ColNum = col.first;
					break;
				}
			}
			
			// Open CSV File and Remember all the line number that eachline[ColNum] == BldgName
			std::ifstream ifs(CSVPath);
			if (!ifs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			vector<int> MatchedLineNumbers;
			int LineNumber = 1;

			std::string line;
			std::string FirstLine;
			vector<std::string> lines;
			std::getline(ifs, line);
			FirstLine = line;

			vector<string> EachLineData;

			while (std::getline(ifs, line))
			{
				EachLineData.clear();
				lines.push_back(line);
				std::istringstream ss(line);
				std::string token;

				while (std::getline(ss, token, ','))
				{
					EachLineData.push_back(token);
				}
				if (EachLineData[ColNum] == BldgName)
				{
					MatchedLineNumbers.push_back(LineNumber);
				}
				LineNumber++;
			}

			ifs.close();
			
			std::cout << "Total Matched Line Numbers: " << MatchedLineNumbers.size() << endl;
	
			LineNumber = 1;

			// ERASE All the Data in CSV File
			line = "";
			std::ofstream ofs(CSVPath);
			if (!ofs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			ofs << FirstLine << std::endl;

			if (!ofs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			for (std::string elem : lines)
			{
				if(MatchedLineNumbers.size() != 0 &&  MatchedLineNumbers[0] == LineNumber)
				{
					while (elem[elem.size()-1] != ',')
					{
						elem.pop_back();
					}

					ofs << elem << LabelMaps[labelInfo] << std::endl;
					MatchedLineNumbers.erase(MatchedLineNumbers.begin());
				}
				else
				{
					ofs << elem << std::endl;
				}
				LineNumber++;
			}

			ofs.close();
			cout << "Labeling Complete\n\n";
			break;
		}
		case ELEVATORNAME:
		{
			break;
		}
		case TIMESTAMP:
		{
			int StartTimestampYear = -1;
			int StartTimestampMonth = -1;
			int StartTimestampDay = -1;
			int StartTimestampHour = -1;
			int StartTimestampMinute = -1;

			int EndTimestampYear = -1;
			int EndTimestampMonth = -1;
			int EndTimestampDay = -1;
			int EndTimestampHour = -1;
			int EndTimestampMinute = -1;

			std::string StartTimestamp;
			std::string EndTimestamp;

			std::cout << "Enter a Start Timestamp To Put This Label On\nEX)1999 02 08 20 15 = 1999.02.08 20:15\n";
			std::getline(std::cin, StartTimestamp);

			// Split to empty space and check each element can convert to number
			bool Flag = true;
			while (Flag)
			{
				std::istringstream ss(StartTimestamp);
				std::string token;
				std::vector<std::string> tokens;
				while (std::getline(ss, token, ' '))
				{
					tokens.push_back(token);
				}

				if (StartTimestamp == "")
				{
					std::getline(std::cin, StartTimestamp);
					continue;
				}

				else if (tokens.size() != 5)
				{
					std::cout << "Invalid Timestamp Format. Please Enter Again: " << std::endl;
					std::getline(std::cin, StartTimestamp);
				}
				else
				{
					try
					{
						StartTimestampYear = std::stoi(tokens[0]);
						StartTimestampMonth = std::stoi(tokens[1]);
						StartTimestampDay = std::stoi(tokens[2]);
						StartTimestampHour = std::stoi(tokens[3]);
						StartTimestampMinute = std::stoi(tokens[4]);
						Flag = false;
					}
					catch (std::exception &e)
					{
						std::cout << "Invalid Timestamp Format. Please Enter Again: " << std::endl;
						std::getline(std::cin, StartTimestamp);
					}
				}
			}

			std::cout << "Enter a End Timestamp To Put This Label On\nEX)1999 02 08 20 15 = 1999.02.08 20:15\n";
			std::getline(std::cin, EndTimestamp);

			Flag = true;
			while (Flag)
			{
				std::istringstream ss(EndTimestamp);
				std::string token;
				std::vector<std::string> tokens;
				while (std::getline(ss, token, ' '))
				{
					tokens.push_back(token);
				}

				if (EndTimestamp == "")
				{
					std::getline(std::cin, EndTimestamp);
					continue;
				}

				else if (tokens.size() != 5)
				{
					std::cout << "Invalid Timestamp Format. Please Enter Again: " << std::endl;
					std::getline(std::cin, EndTimestamp);
				}
				else
				{
					try
					{
						EndTimestampYear = std::stoi(tokens[0]);
						EndTimestampMonth = std::stoi(tokens[1]);
						EndTimestampDay = std::stoi(tokens[2]);
						EndTimestampHour = std::stoi(tokens[3]);
						EndTimestampMinute = std::stoi(tokens[4]);
						Flag = false;
					}
					catch (std::exception& e)
					{
						std::cout << "Invalid Timestamp Format. Please Enter Again: " << std::endl;
						std::getline(std::cin, EndTimestamp);
					}
				}
			}

			// GET A index of SortId using ColNum
			int ColNumYear = -1;
			int ColNumMonth = -1;
			int ColNumDay = -1;
			int ColNumHour = -1;
			int ColNumMinute = -1;
			for (auto& col : ColMaps)
			{
				if (col.second == "Year")
				{
					ColNumYear = col.first;
				}
				else if (col.second == "Month")
				{
					ColNumMonth = col.first;
				}
				else if (col.second == "Day")
				{
					ColNumDay = col.first;
				}
				else if (col.second == "Hour")
				{
					ColNumHour = col.first;
				}
				else if (col.second == "Minute")
				{
					ColNumMinute = col.first;
				}
			}

			// Open CSV File and Remember all the line number that eachline[ColNum] == BldgName
			std::ifstream ifs(CSVPath);
			if (!ifs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			vector<int> MatchedLineNumbers;
			int LineNumber = 1;

			std::string line;
			std::string FirstLine;
			vector<std::string> lines;
			std::getline(ifs, line);
			FirstLine = line;

			vector<string> EachLineData;

			while (std::getline(ifs, line))
			{
				EachLineData.clear();
				lines.push_back(line);
				std::istringstream ss(line);
				std::string token;

				while (std::getline(ss, token, ','))
				{
					EachLineData.push_back(token);
				}

				int ThisLineYear = std::stoi(EachLineData[ColNumYear]);
				int ThisLineMonth = std::stoi(EachLineData[ColNumMonth]);
				int ThisLineDay = std::stoi(EachLineData[ColNumDay]);
				int ThisLineHour = std::stoi(EachLineData[ColNumHour]);
				int ThisLineMinute = std::stoi(EachLineData[ColNumMinute]);

				if(CheckTimeRange(
					{StartTimestampYear, StartTimestampMonth, StartTimestampDay, StartTimestampHour, StartTimestampMinute},
					{EndTimestampYear, EndTimestampMonth, EndTimestampDay, EndTimestampHour, EndTimestampMinute},
					{ThisLineYear, ThisLineMonth, ThisLineDay, ThisLineHour, ThisLineMinute}))
				{
					MatchedLineNumbers.push_back(LineNumber);
				}
				LineNumber++;
			}

			ifs.close();

			std::cout << "Total Matched Line Numbers: " << MatchedLineNumbers.size() << endl;

			LineNumber = 1;

			// ERASE All the Data in CSV File
			line = "";
			std::ofstream ofs(CSVPath);
			if (!ofs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			ofs << FirstLine << std::endl;

			if (!ofs.is_open())
			{
				std::cerr << "Error: File not found" << std::endl;
				return;
			}

			for (std::string elem : lines)
			{
				if (MatchedLineNumbers.size() != 0 && MatchedLineNumbers[0] == LineNumber)
				{
					while (elem[elem.size() - 1] != ',')
					{
						elem.pop_back();
					}

					ofs << elem << LabelMaps[labelInfo] << std::endl;
					MatchedLineNumbers.erase(MatchedLineNumbers.begin());
				}
				else
				{
					ofs << elem << std::endl;
				}
				LineNumber++;
			}

			ofs.close();
			cout << "Labeling Complete\n\n";
			break;
			break;
		}
		case VALUE:
		{
			break;
		}
		case FLOOR:
		{
			break;
		}
		case PHYSICS:
		{
			break;
		}
		case TEMPERATURE:
		{
			break;
		}
	default:
		break;
	}
}

bool DTLabeling::CheckTimeRange(vector<int> Start, vector<int> End, vector<int> Time)
{
	// Check if three argument vector length is same
	if (Start.size() != End.size() || End.size() != Time.size())
	{
		return false;
	}

	for (int i = 0; i < Start.size(); i++)
	{
		if (Time[i] < Start[i])
		{
			return false;
		}
		else if (Time[i] > Start[i])
		{
			break;
		}
		else if (i == Start.size() - 1 && Time[i] == Start[i])
		{
			break;
		}
	}

	for (int i = 0; i < End.size(); i++)
	{
		if (Time[i] > End[i])
		{
			return false;
		}
	}

	return true;
}
