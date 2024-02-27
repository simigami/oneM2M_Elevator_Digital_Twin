#pragma once
#include "elevatorAlgorithmDefault.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

class elevatorAlgorithmMultiple : public elevatorAlgorithmDefault
{
public:
	elevatorAlgorithmMultiple(string buildingName, string deviceName);
	~elevatorAlgorithmMultiple();

	virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stopThread() override;

	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void rearrangeVector(socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy) override;

	int ReverseIntervalMovingDeltaMaximum = 2;
	bool bFirstMoveFlag = true;
};
