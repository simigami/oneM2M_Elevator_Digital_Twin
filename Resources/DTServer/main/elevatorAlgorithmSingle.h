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

	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stop(physics* p) override;
	virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy) override;

	virtual void appendLogToLogList(int code, ...) override;
	virtual void writeLog() override;

	virtual void printThisElevatorEnergyConsumptionInfos() override;

	virtual int printTimeDeltaNow() override;

	virtual void rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy) override;
};
