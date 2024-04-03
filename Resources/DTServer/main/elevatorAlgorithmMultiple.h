#pragma once
#include "elevatorAlgorithmDefault.h"

class elevatorAlgorithmMultiple : public elevatorAlgorithmDefault
{
public:
	elevatorAlgorithmMultiple(wstring buildingName, wstring deviceName, std::chrono::system_clock::time_point this_building_creation_time);
	~elevatorAlgorithmMultiple();

	virtual string getJSONString() override;

	virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stopThread() override;

	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void rearrangeVector(elevatorStatus* stats, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy) override;

	virtual void appendLogToLogList(int code, ...) override;
	virtual void writeLog() override;

	virtual int printTimeDeltaNow() override;
	void moveToDeterminedFloor(socket_UnrealEngine* ueSock, physics* phy);

	int ReverseIntervalMovingDeltaMaximum = 2;
	bool bFirstMoveFlag = true;
};
