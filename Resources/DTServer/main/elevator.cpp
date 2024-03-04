#include "elevator.h"

Elevator::Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES, int algorithmNumber)
{
	this->sock = new socket_oneM2M(parsed_struct, ACP_NAMES);
	this->p = new physics(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    });
	this->UEsock = new socket_UnrealEngine(parsed_struct.building_name, parsed_struct.device_name, parsed_struct.underground_floor, parsed_struct.ground_floor, this->p->info.altimeter_of_each_floor, this->p->info.acceleration, this->p->info.max_velocity);

	this->thisElevatorAlgorithmSingle = new elevatorAlgorithmSingle(parsed_struct.building_name, parsed_struct.device_name);
	this->thisElevatorAlgorithmMultiple = new elevatorAlgorithmMultiple(parsed_struct.building_name, parsed_struct.device_name);

	this->building_name = parsed_struct.building_name;
	this->device_name = parsed_struct.device_name;

	this->algorithmNumber = algorithmNumber;
}

void Elevator::runElevator()
{
	if(algorithmNumber == 1)
	{
		this->thisElevatorAlgorithmSingle->startThread(sock , UEsock, p);
	}
	else
	{
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
