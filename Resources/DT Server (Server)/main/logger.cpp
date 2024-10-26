#include "logger.h"
#include <iostream>
#include <__msvc_chrono.hpp>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>


static std::string wstring_to_string(const std::wstring& wstr)
{
	return std::string(wstr.begin(), wstr.end());
}

std::wstring string_to_wstring(const std::string& str) {
	return std::wstring(str.begin(), str.end());
}

logger::logger()
{

}

void logger::set_log_directory(const std::wstring& dirPath)
{
	// CHECK IF THIS IS A VALID DIRECTORY
	if (dirPath.empty())
	{
		std::wcerr << L"Invalid directory path" << std::endl;
		return;
	}
	log_directory = dirPath;
}

void logger::set_log_file_name(const std::wstring& fileName)
{
	log_file_name = fileName;
}

std::wstring logger::get_file_name_as_timestamp()
{
	// Get current time
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	struct std::tm now_tm;
	localtime_s(&now_tm, &now_time_t);

	// Format time
	std::wstringstream ss;
	ss << std::put_time(&now_tm, L"%Y-%m-%d");
	return ss.str();
}

void logger::write_logs(std::vector<std::wstring> strings)
{
	std::wstring path;
	// CHECK OS
	#ifdef _WIN32
	path = log_directory + L"\\" + get_file_name_as_timestamp() + L"_" + log_file_name;
	#else
	path = log_directory + L"/" + get_file_name_as_timestamp() + L"_" + log_file_name;
	#endif

	std::wofstream file(path, std::ios::app);

	if (!file.is_open()) {
		std::wcerr << L"Failed to open log file: " << path << std::endl;
		return;
	}
	
	// MUTEX LOCK
	log_mutex.lock();

	for (const auto& str : strings)
	{
		file << str << std::endl;
	}
	file.close();

	// MUTEX UNLOCK
	log_mutex.unlock();
}

void logger::write_log(std::wstring string)
{
	std::wstring path;
	// CHECK OS
#ifdef _WIN32
	path = log_directory + L"\\" + log_file_name;
#else
	path = log_directory + L"/"  + log_file_name;
#endif

	std::wofstream file(path, std::ios::app);

	if (!file.is_open()) {
		std::wcerr << L"Failed to open log file: " << path << std::endl;
		return;
	}

	// MUTEX LOCK
	log_mutex.lock();

	file << string << std::endl;
	file.close();

	// MUTEX UNLOCK
	log_mutex.unlock();
}

void logger::write_csv(std::wstring string)
{
	std::wstring path;
	// CHECK OS
#ifdef _WIN32
	path = log_directory + L"\\" + csv_file_name;
#else
	path = log_directory + L"/" + csv_file_name;
#endif

	std::wofstream file(path, std::ios::app);

	if (!file.is_open()) {
		std::wcerr << L"Failed to open log file: " << path << std::endl;
		return;
	}

	// MUTEX LOCK
	log_mutex.lock();

	file << string;
	file.close();

	// MUTEX UNLOCK
	log_mutex.unlock();
}

void logger::write_sim_csv_header()
{
	std::wstring path;
	// CHECK OS
#ifdef _WIN32
	path = log_directory + L"\\" + csv_file_name;
#else
	path = log_directory + L"/" + csv_file_name;
#endif

	std::wofstream file(path, std::ios::app);

	if (!file.is_open()) {
		std::wcerr << L"Failed to open log file: " << path << std::endl;
		return;
	}

	const auto temp = new CSVHeader();

	for (const auto& str : temp->SimHeader)
	{
		file << str;

		if (str == temp->SimHeader.back())
		{
			file << std::endl;
		}
		else
		{
			file << L",";
		}
	}
}

void logger::write_rts_csv_header()
{
	std::wstring path;
	// CHECK OS
#ifdef _WIN32
	path = log_directory + L"\\" + csv_file_name;
#else
	path = log_directory + L"/" + csv_file_name;
#endif

	std::wofstream file(path, std::ios::app);

	if (!file.is_open()) {
		std::wcerr << L"Failed to open log file: " << path << std::endl;
		return;
	}

	const auto temp = new CSVHeader();

	for (const auto& str : temp->RTSHeader)
	{
		file << str;

		if (str == temp->RTSHeader.back())
		{
			file << std::endl;
		}
		else
		{
			file << L",";
		}
	}
}

std::wstring logger::IDLELog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Timestamp

	// 인자 개수 검사
	int expectedArgCount = 3;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		// 에러 메시지 출력
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);

		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	std::wstring buildingName = va_arg(args, const std::wstring);
	std::wstring deviceName = va_arg(args, const std::wstring);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"IDLE " + buildingName + L" " + deviceName + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	return logMessage;
}

std::wstring logger::CALLLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Called Floor
	// arg[4] = Called Floor Direction
	// arg[5] = Timestamp

	const int expectedArgCount = 5;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);
		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	std::wstring buildingName = va_arg(args, const std::wstring);
	std::wstring deviceName = va_arg(args, const std::wstring);
	int calledFloor = va_arg(args, int);
	std::wstring calledFloorDirection = va_arg(args, int) == 1 ? L"Up" : L"Down";
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"CALL " + buildingName + L" " + deviceName + L" " + std::to_wstring(calledFloor) + L" " + calledFloorDirection + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	return logMessage;
}

std::wstring logger::PRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Pressed Floor List
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);
		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const std::wstring);
	const std::wstring deviceName = va_arg(args, const std::wstring);
	const std::vector<int> pressedFloorList = va_arg(args, std::vector<int>);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"PRESS " + buildingName + L" " + deviceName + L" [";

	for (int i = 0; i < pressedFloorList.size(); i++) {
		logMessage += std::to_wstring(pressedFloorList[i]);
		if (i != pressedFloorList.size() - 1) {
			logMessage += L",";
		}
	}

	logMessage.append(L"] " + std::to_wstring(timestamp));

	// 로그 리스트에 추가
	return logMessage;
}

std::wstring logger::UNPRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Unpressed Floor List
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);
		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const std::wstring);
	const std::wstring deviceName = va_arg(args, const std::wstring);
	std::vector<int> unpressedFloorList = va_arg(args, std::vector<int>);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"UNPRESS " + buildingName + L" " + deviceName + L" [";

	for (int i = 0; i < unpressedFloorList.size(); i++) {
		logMessage += std::to_wstring(unpressedFloorList[i]);
		if (i != unpressedFloorList.size() - 1) {
			logMessage += L", ";
		}
	}

	logMessage += L"] " + timestamp;

	// 로그 리스트에 추가
	return logMessage;
}

std::wstring logger::STOPLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Stopped Floor
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);
		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const std::wstring);
	const std::wstring deviceName = va_arg(args, const std::wstring);
	int stoppedFloor = va_arg(args, int);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"STOP " + buildingName + L" " + deviceName + L" " + std::to_wstring(stoppedFloor) + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	return logMessage;
}

std::wstring logger::MOVLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Moving Floor
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::wstring error_msg = L"IDLELog Argument Count Does Not Match -> Expected : " + std::to_wstring(expectedArgCount) + L", Actual : " + std::to_wstring(actualArgCount);
		return error_msg;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const std::wstring);
	const std::wstring deviceName = va_arg(args, const std::wstring);
	const int movingFloor = va_arg(args, int);
	const int timestamp = va_arg(args, const int);
	va_end(args);

	// 로그 메시지 생성
	std::wstring logMessage = L"MOV " + buildingName + L" " + deviceName + L" " + std::to_wstring(movingFloor) + L" " + std::to_wstring(timestamp);

	return logMessage;
}
