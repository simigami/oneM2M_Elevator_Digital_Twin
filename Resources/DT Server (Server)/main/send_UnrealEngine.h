/*
#pragma once
#include <vector>
#include <string>
#include <boost/serialization/serialization.hpp>

using namespace std;
//DATA THAT CONTAINS UE5 To MAKE V_T Graph
struct UE_Info
{
	string building_name;
	string device_name;

	int underground_floor;
	int ground_floor;

	vector<double> each_floor_altimeter;

	int goToFloor = 0;

	double acceleration;
	double max_velocity;

	double tta;
	double ttm;
	double ttd;

	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive& ar, const unsigned int version);
};

class send_UnrealEngine
{
public:
	string struct_to_json(const UE_Info& info);
	void set_UE_info(const UE_Info& info);

private:
	UE_Info UE_info;
};

template<typename Archive>
inline void UE_Info::serialize(Archive& ar, const unsigned int version)
{
	ar & building_name;
	ar & device_name;
	ar & underground_floor;
	ar & ground_floor;
	ar & max_velocity;
	ar & acceleration;
	ar & tta;
	ar & ttm;
	ar & ttd;
}*/