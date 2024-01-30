#pragma once
#include <boost/asio.hpp>
#include "send_UnrealEngine.h"

using namespace std;

#define UE5_IP "127.0.0.1"
#define UE5_PORT 10052

class socket_UnrealEngine
{
public:
	socket_UnrealEngine(string building_name, string device_name, int underground_floor, int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity);

	send_UnrealEngine sock;

	vector<double> set_sock_altimeter_offsets();

	void send_data_to_UE5(const UE_Info& info);
	void set_goTo_Floor(int floor);
	void accept_con();
};
