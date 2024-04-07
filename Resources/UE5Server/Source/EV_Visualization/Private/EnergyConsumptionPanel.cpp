// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyConsumptionPanel.h"

TArray<UEachElevatorBarWidgetPanel*> UEnergyConsumptionPanel::GetSortedElevatorBarWidgetPanels(
	bool updown, int sortType)
{
	// Get All uobject name UEachElevatorBarWidgetPanel
	
	// if sortType is 0, sort by building name
	if (sortType == 0)
	{
		// if updown is true, sort by building name in ascending order
		if (updown)
		{
			// sort by building name in ascending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				// if there is multiple panel that has same building name, sort by elevator name in ascending order
				if(A.BuildingNameString == B.BuildingNameString)
				{
					return A.ElevatorNameString < B.ElevatorNameString;
				}
				
				return A.BuildingNameString < B.BuildingNameString;
			});
		}

		// if updown is false, sort by building name in descending order
		else
		{
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				// if there is multiple panel that has same building name, sort by elevator name in ascending order
				if(A.BuildingNameString == B.BuildingNameString)
				{
					return A.ElevatorNameString > B.ElevatorNameString;
				}
				
				return A.BuildingNameString > B.BuildingNameString;
			});
		}
	}

	// if sortType is 1, sort by elevator name
	else if (sortType == 1)
	{
		// if updown is true, sort by elevator name in ascending order
		if (updown)
		{
			// sort by elevator name in ascending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				if(A.ElevatorNameString == B.ElevatorNameString)
				{
					return A.BuildingNameString < B.BuildingNameString;
				}
				
				return A.ElevatorNameString < B.ElevatorNameString;
			});
		}

		// if updown is false, sort by elevator name in descending order
		else
		{
			// sort by elevator name in descending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				if(A.ElevatorNameString == B.ElevatorNameString)
				{
					return A.BuildingNameString > B.BuildingNameString;
				}
				
				return A.ElevatorNameString > B.ElevatorNameString;
			});
		}
	}

	// if sortType is 2, sort by erd double value
	else if (sortType == 2)
	{
		// if updown is true, sort by erd double value in ascending order
		if (updown)
		{
			// sort by erd double value in ascending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.ErdDouble < B.ErdDouble;
			});
		}

		// if updown is false, sort by erd double value in descending order
		else
		{
			// sort by erd double value in descending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.ErdDouble > B.ErdDouble;
			});
		}
	}

	// if sortType is 3, sort by esd double value
	else if (sortType == 3)
	{
		// if updown is true, sort by esd double value in ascending order
		if (updown)
		{
			// sort by esd double value in ascending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.EsdDouble < B.EsdDouble;
			});
		}

		// if updown is false, sort by esd double value in descending order
		else
		{
			// sort by esd double value in descending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.EsdDouble > B.EsdDouble;
			});
		}
	}

	// if sortType is 4, sort by ed double value
	else if (sortType == 4)
	{
		// if updown is true, sort by ed double value in ascending order
		if (updown)
		{
			// sort by ed double value in ascending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.EdDouble < B.EdDouble;
			});
		}

		// if updown is false, sort by ed double value in descending order
		else
		{
			// sort by ed double value in descending order
			this->ElevatorBarWidgetPanels.Sort([](const UEachElevatorBarWidgetPanel& A, const UEachElevatorBarWidgetPanel& B)
			{
				return A.EdDouble > B.EdDouble;
			});
		}
	}

	// else do - nothing
	else
	{
		// do nothing
	}

	return this->ElevatorBarWidgetPanels;
}

UEachElevatorBarWidgetPanel* UEnergyConsumptionPanel::GetElevatorBarWidgetPanel(FString buildingName,
	FString elevatorName)
{
	// Returns UEachElevatorBarWidgetPanel that holds specific building name and elevator name
	for (const auto& EachElevatorBarWidgetPanel : this->ElevatorBarWidgetPanels)
	{
		if (EachElevatorBarWidgetPanel->BuildingNameString == buildingName &&
			EachElevatorBarWidgetPanel->ElevatorNameString == elevatorName)
		{
			return EachElevatorBarWidgetPanel;
		}
	}

	return nullptr;
}

void UEnergyConsumptionPanel::SetElevatorBarWidgetPanelMembers(FDecisionStruct decisionStruct,
	UEachElevatorBarWidgetPanel* elevatorBarWidgetPanel)
{
	// Set building name, elevator name, erd double, esd double, ed double
	elevatorBarWidgetPanel->BuildingNameString = decisionStruct.building_name;
	elevatorBarWidgetPanel->ElevatorNameString = decisionStruct.device_name;
	elevatorBarWidgetPanel->ErdDouble = decisionStruct.erd;
	elevatorBarWidgetPanel->EsdDouble = decisionStruct.esd;
	elevatorBarWidgetPanel->EdDouble = decisionStruct.ed;
}
