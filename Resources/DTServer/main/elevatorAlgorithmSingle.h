#pragma once
#include "elevatorAlgorithmDefault.h"
#include "socket_UnrealEngine.h"
#include "socket_oneM2M.h"
#include "simulation.h"

class elevatorAlgorithmSingle : public elevatorAlgorithmDefault
{
public:
	elevatorAlgorithmSingle(string buildingName, string deviceName);
	~elevatorAlgorithmSingle();

	virtual void startThread(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void stopThread() override;

	virtual void run(socket_oneM2M* sock, socket_UnrealEngine* ueSock, physics* phy) override;
	virtual void updateElevatorTick(socket_UnrealEngine* ueSock, physics* phy) override;
};
