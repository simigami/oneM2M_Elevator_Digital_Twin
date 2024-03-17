#include "elevator.h"

Elevator::Elevator(elevator_resource_status sc, Wparsed_struct parsed_struct, vector<wstring> ACP_NAMES, int algorithmNumber)
{
	this->sock = new socket_oneM2M(sc, parsed_struct, ACP_NAMES);
	this->p = new physics(parsed_struct);
	this->UEsock = new socket_UnrealEngine();

	this->thisElevatorAlgorithmSingle = new elevatorAlgorithmSingle(parsed_struct.building_name, parsed_struct.device_name);
	this->thisElevatorAlgorithmMultiple = new elevatorAlgorithmMultiple(parsed_struct.building_name, parsed_struct.device_name);

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

wstring Elevator::getBuildingName() const
{
	return this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_building_name();	
}

wstring Elevator::getDeviceName() const
{
	return this->thisElevatorAlgorithmSingle->thisElevatorStatus->get_device_name();

}
