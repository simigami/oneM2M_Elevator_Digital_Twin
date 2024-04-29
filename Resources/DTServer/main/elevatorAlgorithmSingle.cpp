#include "elevatorAlgorithmSingle.h"

elevatorAlgorithmSingle::elevatorAlgorithmSingle(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time) : elevatorAlgorithmDefault(buildingName, deviceName, this_building_creation_time)
{

}

elevatorAlgorithmSingle::~elevatorAlgorithmSingle()
{

}

string elevatorAlgorithmSingle::getJSONString()
{
	return elevatorAlgorithmDefault::getJSONString();
}

void elevatorAlgorithmSingle::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::startThread(sock , ueSock, phy);

	thread temp(&elevatorAlgorithmSingle::run, this, sock , ueSock, phy);
	temp.detach();
}

void elevatorAlgorithmSingle::stopThread()
{
	elevatorAlgorithmDefault::stopThread();
}

vector<int> elevatorAlgorithmSingle::getNewButtonInsideList(vector<int> prev, vector<int> current)
{
	return elevatorAlgorithmDefault::getNewButtonInsideList(prev, current);
}

vector<int> elevatorAlgorithmSingle::getRemoveButtonInsideList(vector<int> prev, vector<int> current)
{
	return elevatorAlgorithmDefault::getRemoveButtonInsideList(prev, current);
}

void elevatorAlgorithmSingle::insideLogic(elevatorStatus* status, physics* phy, vector<int> new_elem, vector<int> remove_elem)
{
	elevatorAlgorithmDefault::insideLogic(status, phy, new_elem, remove_elem);
}

void elevatorAlgorithmSingle::outsideLogic(elevatorStatus* status, physics* phy, int outside_button, bool outside_direction)
{
	elevatorAlgorithmDefault::outsideLogic(status, phy, outside_button, outside_direction);
}

void elevatorAlgorithmSingle::updateMainTripList(elevatorStatus* status, physics* phy, int elem)
{
	elevatorAlgorithmDefault::updateMainTripList(status, phy, elem);
}

void elevatorAlgorithmSingle::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::printTimeDeltaWhenSpawn();
	elevatorAlgorithmDefault::run(sock, ueSock, phy);
}

void elevatorAlgorithmSingle::stop(physics* p, flags* this_flag)
{
	elevatorAlgorithmDefault::stop(p, this_flag);
}

void elevatorAlgorithmSingle::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::updateElevatorTick(ueSock, phy);
}

void elevatorAlgorithmSingle::writeEnergyLog()
{
	elevatorAlgorithmDefault::writeEnergyLog();
}

void elevatorAlgorithmSingle::write_logs(std::vector<std::wstring> strings)
{
	elevatorAlgorithmDefault::write_logs(strings);
}

void elevatorAlgorithmSingle::write_log(std::wstring string)
{
	elevatorAlgorithmDefault::write_log(string);
}

void elevatorAlgorithmSingle::printThisElevatorEnergyConsumptionInfos()
{
	elevatorAlgorithmDefault::printThisElevatorEnergyConsumptionInfos();
}

int elevatorAlgorithmSingle::printTimeDeltaNow()
{
	return elevatorAlgorithmDefault::printTimeDeltaNow();
}

void elevatorAlgorithmSingle::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::rearrangeVector(stats, ueSock, phy);
}