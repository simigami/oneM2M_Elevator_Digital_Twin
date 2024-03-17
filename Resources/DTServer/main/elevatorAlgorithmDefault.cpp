#include <nlohmann/json.hpp>
#include "elevatorAlgorithmDefault.h"

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
	this->go_To_Floor = 0;

	this->thisElevatorStatus->current_goTo_floor_vector_info = new vector<vector<double>>{};
	this->thisElevatorStatus->current_goTo_Floor_single_info = vector<double>{};

	p->s = new simulation();
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

	if (this->thisElevatorStatus->get_building_name() == L"EV2") {
		cout << "ASD" << endl;
	}

	std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Rearranged At TIME : ";
	sprintf_s(timeString, "%04d/%02d/%02d - %02d:%02d:%02d.%03lld, Time Delta is %lld Seconds\n",
		tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_hour , tstruct.tm_min,
		tstruct.tm_sec, milliseconds.count(), diffSeconds
		);
	puts(timeString);
}

void elevatorAlgorithmDefault::printTimeDeltaWhenStop()
{
	char timeString[128];
	struct tm tstruct;
	struct tm tstructDelta;

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