#include <nlohmann/json.hpp>
#include "elevatorAlgorithmDefault.h"
#include <mutex>

elevatorAlgorithmDefault::elevatorAlgorithmDefault(wstring buildingName, wstring deviceName)
{
	thisNotificationContent = new notificationContent;

	thisElevatorStatus = new elevatorStatus;
	thisElevatorStatus->set_names(buildingName, deviceName);

	thisFlags = new flags;

	worldClock = std::chrono::system_clock::now();
	RETRIEVE_interval_millisecond = 10;
}

elevatorAlgorithmDefault::~elevatorAlgorithmDefault()
{

}

void elevatorAlgorithmDefault::set_physical_information(physics* p)
{
	thisElevatorStatus->set_physical_information(p);
}

void elevatorAlgorithmDefault::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
}

void elevatorAlgorithmDefault::stopThread()
{
}

void elevatorAlgorithmDefault::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* p)
{
	char timeString[128];
	time_t timeNow = std::chrono::system_clock::to_time_t(this->worldClock);

	struct tm tstruct;
	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration)%1000;

	std::wcout << L"IN " <<  this->thisElevatorStatus->get_building_name() << L" -> " << this->thisElevatorStatus->get_device_name() << L" Spawned At TIME : ";

	sprintf_s(timeString, "%04d/%02d/%02d - %02d:%02d:%02d.%03lld\n",
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour , tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count()
		);
	std::puts(timeString);
}

void elevatorAlgorithmDefault::stop(physics* p)
{
	this->thisNotificationContent = new notificationContent;
	this->thisFlags = new flags;
	this->log_list = vector<wstring>{};

	this->thisElevatorStatus->current_goTo_floor_vector_info = new vector<vector<double>>{};
	this->thisElevatorStatus->current_goTo_Floor_single_info = vector<double>{};

	p->s = new simulation();
}

void elevatorAlgorithmDefault::appendLogToLogList(int code, ...)
{
	va_list args;
	va_start(args, code);

	switch (code)
	{
	case IDLE:
		this->IDLELog(args);
		break;
	case CALL:
		this->CALLLog(args);
		break;
	case PRESS:
		this->PRESSLog(args);
		break;
	case UNPRESS:
		this->UNPRESSLog(args);
		break;
	case STOP:
		this->STOPLog(args);
		break;
	case MOV:
		this->MOVLog(args);
		break;
	default:
		wcerr << L"LOG CODE ERROR" << endl;
		break;
	}
}

void elevatorAlgorithmDefault::IDLELog(va_list args)
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
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	std::wstring buildingName = va_arg(args, const wstring);
	std::wstring deviceName = va_arg(args, const wstring);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"IDLE " + buildingName + L" " + deviceName + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::CALLLog(va_list args)
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
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	std::wstring buildingName = va_arg(args, const wstring);
	std::wstring deviceName = va_arg(args, const wstring);
	int calledFloor = va_arg(args, int);
	wstring calledFloorDirection = va_arg(args, int) == 1 ? L"Up" : L"Down";
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"CALL " + buildingName + L" " + deviceName + L" " + std::to_wstring(calledFloor) + L" " + calledFloorDirection + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::PRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Pressed Floor List
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const wstring);
	const std::wstring deviceName = va_arg(args, const wstring);
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
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::UNPRESSLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Unpressed Floor List
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const wstring);
	const std::wstring deviceName = va_arg(args, const wstring);
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
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::STOPLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Stopped Floor
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);
	
	if (expectedArgCount != actualArgCount) {
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const wstring);
	const std::wstring deviceName = va_arg(args, const wstring);
	int stoppedFloor = va_arg(args, int);
	const int timestamp = va_arg(args, const int);

	// 로그 메시지 생성
	std::wstring logMessage = L"STOP " + buildingName + L" " + deviceName + L" " + std::to_wstring(stoppedFloor) + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::MOVLog(va_list args)
{
	// arg[0] = arg count
	// arg[1] = Building Name
	// arg[2] = Device Name
	// arg[3] = Moving Floor
	// arg[4] = Timestamp

	int expectedArgCount = 4;
	int actualArgCount = va_arg(args, int);

	if (expectedArgCount != actualArgCount) {
		std::cerr << "IDLELog Argument Count Does Not Match " << std::endl;
		std::cerr << "Expected : " << expectedArgCount << ", Actual : " << actualArgCount << std::endl;
		return;
	}

	// 각 인자 형변환 및 wstring으로 변환
	const std::wstring buildingName = va_arg(args, const wstring);
	const std::wstring deviceName = va_arg(args, const wstring);
	const int movingFloor = va_arg(args, int);
	const int timestamp = va_arg(args, const int);
	va_end(args);

	// 로그 메시지 생성
	std::wstring logMessage = L"MOV " + buildingName + L" " + deviceName + L" " + std::to_wstring(movingFloor) + L" " + std::to_wstring(timestamp);

	// 로그 리스트에 추가
	this->log_list.push_back(logMessage);
}

void elevatorAlgorithmDefault::writeLog()
{
	const auto logs = this->log_list;
	const auto logSize = logs.size();
	const auto wirtePath = LOGFILEPATH;
	const auto temp = this->getElevatorStatus();
	const wstring buildingName = temp->get_building_name();
	const wstring deviceName = temp->get_device_name();

	std::mutex logMutex;

	std::wifstream logFile(wirtePath);
	if (logFile) {
		std::wstring fileContent;
		std::getline(logFile, fileContent);

		size_t lastLogEndPos = fileContent.find(L"LOG END " + buildingName + L" " + deviceName);
		logFile.close();

		std::wofstream logFile(wirtePath, ios::app);
		if (lastLogEndPos == std::string::npos) {
			const auto underground_floor = temp->get_underground_floor();
			const auto ground_floor = temp->get_ground_floor();
			const auto each_floor_altimeter = temp->get_each_floor_altimeter();

			logFile << L"LOG START " << buildingName << L" " << deviceName << L" " << underground_floor << L" " << ground_floor << L" [";
			for (int i = 0; i < each_floor_altimeter.size(); i++) {
				logFile << each_floor_altimeter[i];

				if (i != each_floor_altimeter.size() - 1) {
					logFile << L",";
				}
			}
			logFile << L"]" << std::endl;

			for (const auto& log : logs) {
				logFile << log << std::endl;
			}
			logFile << L"LOG END " << buildingName << L" " << deviceName << std::endl;

			logFile.close();
		}
		else {
			fileContent.insert(lastLogEndPos + 8, L"\n");
			for (const auto& log : logs) {
				fileContent.insert(lastLogEndPos + 9, log + L"\n");
			}
			logFile.seekp(0, std::ios::beg);
			logFile << fileContent;
			logFile.close();
		}
	}
}

void elevatorAlgorithmDefault::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* p)
{
	simulation* sim = p->s;

	//CHANGE V_T Graph
	stats->current_goTo_floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);
	
	stats->direction = p->current_direction;
	stats->tta = p->t_to_max_velocity;
	stats->ttm = p->t_constant_speed;
	stats->ttd = p->t_max_to_zero_deceleration;
	stats->go_to_floor = sim->main_trip_list[0][0];

	it = stats->current_goTo_floor_vector_info->begin();
	stats->current_goTo_Floor_single_info = *it;

	printTimeDeltaWhenRearrange();
	//SEND MODIFY goTo Floor Data To Unreal Engine
	this->set_elevator_Status_JSON_STRING();
	ueSock->send_data_to_UE5(this->elevatorStatus_JSON_STRING);

	this->thisFlags->IDLEFlag = false;
}

void elevatorAlgorithmDefault::set_elevator_Status_JSON_STRING()
{
	nlohmann::json jsonObj;

	jsonObj["building_name"] = wstringToString(thisElevatorStatus->get_building_name());
	jsonObj["device_name"] = wstringToString(thisElevatorStatus->get_device_name());

	jsonObj["acceleration"] = thisElevatorStatus->get_acceleration();
	jsonObj["max_velocity"] = thisElevatorStatus->get_max_velocity();

	jsonObj["underground_floor"] = thisElevatorStatus->get_underground_floor();
	jsonObj["ground_floor"] = thisElevatorStatus->get_ground_floor();

	auto revised_altimeter = thisElevatorStatus->get_each_floor_altimeter();
	double lowest_altimeter = abs(revised_altimeter[0]);

	if (revised_altimeter[0] > 0)
	{
		for (auto& elem : revised_altimeter)
		{
			elem -= lowest_altimeter;
		}
	}
	else
	{
		for (auto& elem : revised_altimeter)
		{
			elem += lowest_altimeter;
		}
	}

	jsonObj["each_floor_altimeter"] = revised_altimeter;
	jsonObj["goToFloor"] = thisElevatorStatus->go_to_floor;

	jsonObj["tta"] = thisElevatorStatus->tta;
	jsonObj["ttm"] = thisElevatorStatus->ttm;
	jsonObj["ttd"] = thisElevatorStatus->ttd;

	this->elevatorStatus_JSON_STRING = jsonObj.dump();
}

void elevatorAlgorithmDefault::printTimeDeltaWhenRearrange()
{
	char timeString[128];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::duration<double> delta = now - this->worldClock;

	time_t timeNow = std::chrono::system_clock::to_time_t(now);
	auto diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();

	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration)%1000;

	std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Rearranged At TIME : ";
	sprintf_s(timeString, "%04d/%02d/%02d - %02d:%02d:%02d.%03lld, Time Delta is %lld Seconds\n",
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour , tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), diffSeconds
		);
	puts(timeString);
}

int elevatorAlgorithmDefault::printTimeDeltaNow()
{
	char timeString[128];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	int diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();

	return diffSeconds;
}

void elevatorAlgorithmDefault::printTimeDeltaWhenStop()
{
	char timeString[128];
	struct tm tstruct;

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::duration<double> delta = now - this->worldClock;

	time_t timeNow = std::chrono::system_clock::to_time_t(now);
	time_t timeDelta = std::chrono::duration_cast<std::chrono::seconds>(delta).count();

	auto diffSeconds = chrono::duration_cast<chrono::seconds>(now - this->worldClock).count();
	localtime_s(&tstruct, &timeNow);

	auto duration = worldClock.time_since_epoch();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration)%1000;

	std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Stopped At TIME : ";
	sprintf_s(timeString, "%04d/%02d/%02d - %02d:%02d:%02d.%03lld, Time Delta is %lld Seconds\n",
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour , tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), diffSeconds
		);
	puts(timeString);
}

void elevatorAlgorithmDefault::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{

}

string elevatorAlgorithmDefault::wstringToString(const std::wstring& wstr) const
{
	std::string s(wstr.begin(), wstr.end());
	return s;
}

wstring elevatorAlgorithmDefault::stringToWstring(const std::string& str) const
{
	std::wstring s(str.begin(), str.end());
	return s;
}

elevatorStatus* elevatorAlgorithmDefault::getElevatorStatus()
{
	return this->thisElevatorStatus;
}

flags* elevatorAlgorithmDefault::getElevatorFlag()
{
	return this->thisFlags;
}

void elevatorStatus::set_names(wstring buildingName, wstring deviceName)
{
	this->building_name = buildingName;
	this->device_name = deviceName;
}

void elevatorStatus::set_physical_information(physics* p)
{
	this->tta = p->t_to_max_velocity;
	this->ttm = p->t_constant_speed;
	this->ttd = p->t_max_to_zero_deceleration;

	this->underground_floor = p->info.underground_floor;
	this->ground_floor = p->info.ground_floor;

	this->each_floor_altimeter = p->info.altimeter_of_each_floor;
	this->max_velocity = p->info.max_velocity;	
	this->acceleration = p->info.acceleration;

	this->current_goTo_floor_vector_info = new vector<vector<double>>{};
	this->current_goTo_Floor_single_info = vector<double>{};
}

wstring elevatorStatus::get_building_name()
{
	return building_name;
}
wstring elevatorStatus::get_device_name()
{
	return device_name;
}
int elevatorStatus::get_underground_floor()
{
	return underground_floor;
}
int elevatorStatus::get_ground_floor()
{
	return ground_floor;
}
vector<double> elevatorStatus::get_each_floor_altimeter()
{
	return each_floor_altimeter;
}
double elevatorStatus::get_max_velocity()
{
	return max_velocity;
}

double elevatorStatus::get_acceleration()
{
	return acceleration;
}