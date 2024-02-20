#include "send_UnrealEngine.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string send_UnrealEngine::struct_to_json(const UE_Info& info)
{
	json jsonObj;

	jsonObj["building_name"] = info.building_name;
    jsonObj["device_name"] = info.device_name;

    jsonObj["underground_floor"] = info.underground_floor;
    jsonObj["ground_floor"] = info.ground_floor;

    jsonObj["each_floor_altimeter"] = info.each_floor_altimeter;
    jsonObj["goToFloor"] = info.goToFloor;

    jsonObj["tta"] = info.tta;
    jsonObj["ttm"] = info.ttm;
    jsonObj["ttd"] = info.ttd;

    jsonObj["acceleration"] = info.acceleration;
    jsonObj["max_velocity"] = info.max_velocity;

    return jsonObj.dump();
}