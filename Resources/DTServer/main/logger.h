#pragma once
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <mutex>

namespace fs = boost::filesystem;

class logger {
public:
	logger();

	std::mutex log_mutex;

	void write_logs(std::vector<std::wstring> strings);
	void write_log(std::wstring string);

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
	std::wstring log_file_name_for_building_logs = L"";

};