#pragma once
#include "elevatorAlgorithmDefault.h"

class elevatorAlgorithmSingle : public elevatorAlgorithmDefault
{
public:
	elevatorAlgorithmSingle(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time);
	~elevatorAlgorithmSingle();

	virtual string getJSONString() override;

	virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stopThread() override;

	virtual vector<int> getNewButtonInsideList(vector<int> prev, vector<int> current) override;
	virtual vector<int> getRemoveButtonInsideList(vector<int> prev, vector<int> current) override;
	
	virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void insideLogic(elevatorStatus* status, physics* phy, vector<int> new_elem, vector<int> remove_elem) override;
	virtual void outsideLogic(elevatorStatus* status, physics* phy, int outside_button, bool outside_direction) override;
	
	virtual void updateMainTripList(elevatorStatus* status, physics* phy, int elem) override;

	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stop(physics* p, flags* this_flag) override;

	virtual void appendLogToLogList(int code, ...) override;
	virtual void writeLog() override;

	virtual void printThisElevatorEnergyConsumptionInfos() override;

	virtual int printTimeDeltaNow() override;

	virtual void rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy) override;
};
