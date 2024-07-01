#include "elevatorAlgorithmMultiple.h"

elevatorAlgorithmMultiple::elevatorAlgorithmMultiple(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time) : elevatorAlgorithmDefault(buildingName, deviceName, this_building_creation_time)
{

}

elevatorAlgorithmMultiple::~elevatorAlgorithmMultiple()
{

}

string elevatorAlgorithmMultiple::getJSONString()
{
	return elevatorAlgorithmDefault::getJSONString();
}

void elevatorAlgorithmMultiple::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::startThread(sock, ueSock, phy);

	thread temp(&elevatorAlgorithmMultiple::run, this, sock , ueSock, phy);
	temp.detach();
}

void elevatorAlgorithmMultiple::stopThread()
{
	elevatorAlgorithmDefault::stopThread();
}

void elevatorAlgorithmMultiple::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::run(sock, ueSock, phy);
}

void elevatorAlgorithmMultiple::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::updateElevatorTick(ueSock, phy);
}

void elevatorAlgorithmMultiple::rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy)
{
	elevatorAlgorithmDefault::rearrangeVector(stats, ueSock, phy);
}

void elevatorAlgorithmMultiple::writeEnergyLog()
{
	elevatorAlgorithmDefault::writeEnergyLog();
}

int elevatorAlgorithmMultiple::printTimeDeltaNow()
{
	return elevatorAlgorithmDefault::printTimeDeltaNow();
}

void elevatorAlgorithmMultiple::moveToDeterminedFloor(socket_UnrealEngine* ueSock, physics* phy)
{
	if(!this->thisFlags->IDLEFlag)
	{
		std::wcout << "IN " <<  this->thisElevatorStatus->get_building_name() << " -> " << this->thisElevatorStatus->get_device_name() << " Error : moveToDeterminedFloor is called when not IDLE" << std::endl;
		return;
	}


}
