#include "elevator.h"

Elevator::Elevator(elevator_resource_status sc, Wparsed_struct parsed_struct, vector<wstring> ACP_NAMES, int algorithmNumber, std::chrono::system_clock::time_point this_building_creation_time)
{
	this->sock = new socket_oneM2M(sc, parsed_struct, ACP_NAMES);
	this->p = new physics(parsed_struct);
	this->UEsock = new socket_UnrealEngine();

	this->thisElevatorAlgorithmSingle = new elevatorAlgorithmSingle(parsed_struct.building_name, parsed_struct.device_name, this_building_creation_time);
	this->thisElevatorAlgorithmMultiple = new elevatorAlgorithmMultiple(parsed_struct.building_name, parsed_struct.device_name, this_building_creation_time);

	this->algorithmNumber = algorithmNumber;
}

void Elevator::runElevator()
{
	if(algorithmNumber == 1)
	{
		this->thisElevatorAlgorithmSingle->set_physical_information(p);
		this->thisElevatorAlgorithmSingle->startThread(sock , UEsock, p);
	}
	else
	{
		this->thisElevatorAlgorithmMultiple->set_physical_information(p);
		this->thisElevatorAlgorithmMultiple->startThread(sock , UEsock, p);
	}

}

void Elevator::setNotificationContent(notificationContent* sendTo)
{
	switch (algorithmNumber)
	{
		case 1:
			this->thisElevatorAlgorithmSingle->thisNotificationContent = sendTo;
			break;
		case 2:
			this->thisElevatorAlgorithmMultiple->thisNotificationContent = sendTo;
			break;
		default:
			this->thisElevatorAlgorithmSingle->thisNotificationContent = sendTo;
			break;
	}
}

void Elevator::setUpdateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{
	switch (algorithmNumber)
	{
		case 1:
			this->thisElevatorAlgorithmSingle->updateElevatorTick(ueSock, phy);
		break;

		case 2:
			this->thisElevatorAlgorithmMultiple->updateElevatorTick(ueSock, phy);
		break;

		default:
			this->thisElevatorAlgorithmSingle->updateElevatorTick(ueSock, phy);
		break;
	}
}

elevatorStatus* Elevator::getElevatorStatus()
{
	switch (this->algorithmNumber)
	{
		case 1:
		return this->thisElevatorAlgorithmSingle->getElevatorStatus();
		break;

		case 2:
		return this->thisElevatorAlgorithmMultiple->getElevatorStatus();
		break;
		
		default:
		break;
	}
	return nullptr;
}

double Elevator::getAltimeterFromFloor(int floor)
{
	vector<double> altimeter_of_each_floor = this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_each_floor_altimeter();	
	int ground_floor = this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_ground_floor();
	int underground_floor = this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_underground_floor();

	if (floor >= 0)
	{
		return altimeter_of_each_floor[underground_floor + floor - 1];
	}
	else
	{
		return altimeter_of_each_floor[underground_floor + floor];
	}
}

wstring Elevator::getBuildingName() const
{
	return this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_building_name();	
}

wstring Elevator::getDeviceName() const
{
	return this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_device_name();

}
