#pragma once
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <mutex>

namespace fs = boost::filesystem;

class CSVHeader {
public:
	std::vector<std::wstring> SimHeader;
	std::vector<std::wstring> RTSHeader;

	CSVHeader() {
		SimHeader.push_back(L"Building Name");
		SimHeader.push_back(L"Elevator Name");
		SimHeader.push_back(L"Datetime");
		SimHeader.push_back(L"Year");
		SimHeader.push_back(L"Month");
		SimHeader.push_back(L"Day");
		SimHeader.push_back(L"Hour");
		SimHeader.push_back(L"Minute");
		SimHeader.push_back(L"Second");
		SimHeader.push_back(L"OpTime");
		SimHeader.push_back(L"Velocity");
		SimHeader.push_back(L"Altimeter");
		SimHeader.push_back(L"ERD");
		SimHeader.push_back(L"ESD");
		SimHeader.push_back(L"ED");
		SimHeader.push_back(L"Labels");

		RTSHeader.push_back(L"Building Name");
		RTSHeader.push_back(L"Elevator Name");
		RTSHeader.push_back(L"Datetime");
		RTSHeader.push_back(L"Year");
		RTSHeader.push_back(L"Month");
		RTSHeader.push_back(L"Day");
		RTSHeader.push_back(L"Hour");
		RTSHeader.push_back(L"Minute");
		RTSHeader.push_back(L"Second");
		RTSHeader.push_back(L"OpTime");
		RTSHeader.push_back(L"Command");
		RTSHeader.push_back(L"Current Floor");
		RTSHeader.push_back(L"Velocity");
		RTSHeader.push_back(L"Altimeter");
		RTSHeader.push_back(L"Distance");
		RTSHeader.push_back(L"ERD");
		RTSHeader.push_back(L"ESD");
		RTSHeader.push_back(L"ED");
		RTSHeader.push_back(L"Labels");
	};
};

class logger {
public:
	logger();

	std::mutex log_mutex;

	void write_logs(std::vector<std::wstring> strings);
	void write_log(std::wstring string);
	void write_csv(std::wstring string);
	void write_sim_csv_header();
	void write_rts_csv_header();
	void write_rts_csv_body();

	std::wstring IDLELog(va_list args);
	std::wstring CALLLog(va_list args);
	std::wstring PRESSLog(va_list args);
	std::wstring UNPRESSLog(va_list args);
	std::wstring STOPLog(va_list args);
	std::wstring MOVLog(va_list args);

	void set_log_directory(const std::wstring& dirPath);
	void set_log_directory_RTS() { this->log_directory = this->log_directory_RTS; };
	void set_log_directory_StateCode() { this->log_directory = this->log_directory_StateCode; };
	void set_log_directory_Simulation() { this->log_directory = this->log_directory_Simulation; };

	void set_log_file_name(const std::wstring& fileName);

	std::wstring get_file_name_as_timestamp();
	
	std::wstring current_path = fs::current_path().wstring();
	std::wstring parent_path = fs::current_path().parent_path().wstring();

#ifdef _WIN32
	std::wstring log_directory = L"";
	std::wstring log_directory_RTS = parent_path + L"\\Log\\RTS";
	std::wstring log_directory_StateCode = parent_path + L"\\Log\\StateCode";
	std::wstring log_directory_Simulation = parent_path + L"\\Log\\Simulation";
#else
	std::wstring log_directory = L"";
	std::wstring log_directory_RTS = parent_path + L"/Log/RTS";
	std::wstring log_directory_Simulation = parent_path + L"/Log/Simulation";
#endif // WIN32
	std::wstring log_file_name = L"";
	std::wstring csv_file_name = L"";
	std::wstring log_file_name_for_building_logs = L"";

};