#include "elevatorAlgorithmDefault.h"

elevatorAlgorithmDefault::elevatorAlgorithmDefault(string buildingName, string deviceName)
{
	this->thisNotificationContent = new notificationContent;

	this->thisElevatorStatus = new elevatorStatus;

	this->thisFlags = new flags;

	this->current_goTo_Floor_vector_info = new vector<vector<double>>;
	this->current_goTo_Floor_single_info = vector<double>{};
	this->RETRIEVE_interval_millisecond = 10;
	this->go_To_Floor = 0;

	this->building_name = buildingName;
	this->device_name = deviceName;
}

elevatorAlgorithmDefault::~elevatorAlgorithmDefault()
{

}

void elevatorAlgorithmDefault::startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy)
{
}

void elevatorAlgorithmDefault::stopThread()
{
}

void elevatorAlgorithmDefault::run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* p)
{

}

void elevatorAlgorithmDefault::rearrangeVector(socket_UnrealEngine* ueSock, physics* p)
{
	simulation* sim = p->s;

	//CHANGE V_T Graph
	current_goTo_Floor_vector_info = p->draw_vt_on_single_floor(sim->main_trip_list[0][0]);

	this->go_To_Floor = sim->main_trip_list[0][0];

	it = current_goTo_Floor_vector_info->begin();
	current_goTo_Floor_single_info = *it;

	//SEND MODIFY goTo Floor Data To Unreal Engine
	ueSock->sock.UE_info = ueSock->wrap_for_UE_socket(this->building_name, this->device_name, p->info.underground_floor, p->info.ground_floor, {-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1}, p->info.acceleration, p->info.max_velocity);
	ueSock->set_goTo_Floor(sim->main_trip_list[0][0], p->t_to_max_velocity, p->t_constant_speed, p->t_max_to_zero_deceleration);
	ueSock->send_data_to_UE5(ueSock->sock.UE_info);
}

void elevatorAlgorithmDefault::updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy)
{

}

elevatorStatus* elevatorAlgorithmDefault::getElevatorStatus()
{
	return this->thisElevatorStatus;
}

flags* elevatorAlgorithmDefault::getElevatorFlag()
{
	return this->thisFlags;
}
