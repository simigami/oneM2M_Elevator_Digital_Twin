// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "EachElevatorActor.h"
#include "SocketThread.h"
#include <Windows.h>
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AMyActor::AMyActor()
{
	InitStatus();
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMyActor::BeginDestroy()
{
	if(ReceiveThread)
	{
		delete ReceiveThread;
		ReceiveThread = nullptr;
	}
	Super::BeginDestroy();
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AMyActor::BuildingNameExists(const FString& BuildingName)
{
	return BuildingLevels.Contains(BuildingName);
}

void AMyActor::RunSocket()
{
	ReceiveThread = new FSocketThread(this);
}

void AMyActor::printAllStats()
{
	/*
	UE_LOG(LogTemp, Log, TEXT("Building Name: %s"), *status.building_name);
	UE_LOG(LogTemp, Log, TEXT("Device Name: %s"), *status.device_name);
	UE_LOG(LogTemp, Log, TEXT("Ground Floor: %d"), status.ground_floor);
	UE_LOG(LogTemp, Log, TEXT("Underground Floor: %d"), status.underground_floor);
	UE_LOG(LogTemp, Log, TEXT("Go To Floor: %d"), status.goToFloor);
	UE_LOG(LogTemp, Log, TEXT("Acceleration: %.2f"), status.acceleration);
	UE_LOG(LogTemp, Log, TEXT("Max Velocity: %.2f"), status.max_velocity);
	UE_LOG(LogTemp, Log, TEXT("TTA: %.2f"), status.tta);
	UE_LOG(LogTemp, Log, TEXT("TTM: %.2f"), status.ttm);
	UE_LOG(LogTemp, Log, TEXT("TTD: %.2f"), status.ttd);
	*/
}

void AMyActor::spawnEachElevatorActor(FString ThisElevatorName, FStatus this_status)
{
	// Get All Class of Actor EachElevator Actor
	for(const auto& elem : EachsubElevatorActorClass)
	{
		AEachElevatorActor* Instance = elem.GetDefaultObject();

		if(Instance->ElevatorName == ThisElevatorName && Instance->BuildingName == this_status.building_name)
		{
			// Set this Class
			Instance->SetThisElevatorStatus(this_status);
			return;
		}
	}

	// If not found, make new instance to spawn
	AEachElevatorActor* NewInstance = GetWorld()->SpawnActor<AEachElevatorActor>(AEachElevatorActor::StaticClass(), FVector(0, 0, 0), FRotator(0, 0, 0));
	NewInstance->ElevatorName = ThisElevatorName;
	NewInstance->BuildingName = this_status.building_name;
	
	NewInstance->SetThisElevatorStatus(this_status);
	return;
}

const UObject* AMyActor::GetWorldContextObjectFromPath(const FString& MapPath)
{
	// Load the level object from the provided path
	ULevel* Level = Cast<ULevel>(StaticLoadObject(ULevel::StaticClass(), nullptr, *MapPath));
    
	// Check if the level was loaded successfully
	if(Level)
	{
		// Get the world context object from the level
		return Level->GetWorld();
	}
    
	// If the level was not loaded successfully, return nullptr
	return nullptr;
}

void AMyActor::ChangeEachFloorAltimeterToABS()
{
	// Get Lowest Altitude
	double lowest_alt = this->status.each_floor_altimeter[0];

	// Set Each Floor Altimeter to ABS
	for(int i = 0; i < this->status.each_floor_altimeter.Num(); i++)
	{
		this->status.each_floor_altimeter[i] = FMath::Abs(this->status.each_floor_altimeter[i] - lowest_alt);
	}
}

TArray<double> AMyActor::getEachFloorTimeLines(FStatus this_status, const int current_floor_index, const int goTo_floor_index)
{
	FCriticalSection Mutex;
	FScopeLock ScopeLock(&Mutex);
	
	//get velocity, acc, tta, ttm, ttd, each floor alt from stats
	const double max_velocity = this_status.max_velocity;
	const double acc = this_status.acceleration;
	TArray<double> eachfloorAlt = this_status.each_floor_altimeter;

	// LOG current_floor_index
	//UE_LOG(LogTemp, Log, TEXT("Current Floor Index: %d"), current_floor_index);

	//push to Tarray each altimeter difference between current floor to goTo floor
	TArray<double> eachFloorTime;
	TArray<double> eachFloorAltDiff;
	const double altimeter_diff = FMath::Abs(eachfloorAlt[goTo_floor_index] - eachfloorAlt[current_floor_index]);

	const double tta = this_status.tta;
	const double ttm = altimeter_diff - this_status.tta * max_velocity > 0 ? altimeter_diff - this_status.tta * max_velocity : 0.0;
	const  double ttd = this_status.ttd;

	if(current_floor_index == goTo_floor_index)
	{
		//ERROR
		UE_LOG(LogTemp, Error, TEXT("Current Floor Index and GoTo Floor Index is Same"));
		Mutex.Unlock();
		return eachFloorTime;
	}
	
	//to get altimeter difference between each floor = subtract each floor index to next floor index
	if(current_floor_index < goTo_floor_index)
	{
		for(int i = current_floor_index; i < goTo_floor_index; i++)
		{
			eachFloorAltDiff.Add(FMath::Abs(eachfloorAlt[i+1] - eachfloorAlt[i]));
		}
	}
	else
	{
		for(int i = current_floor_index; i > goTo_floor_index; i--)
		{
			eachFloorAltDiff.Add(FMath::Abs(eachfloorAlt[i] - eachfloorAlt[i-1]));
		}
	}

	double tta_temp = 0.0;
	double ttm_temp = 0.0;
	double ttd_temp = 0.0;
	double ttd_start_v0 = max_velocity;

	if(current_floor_index == 13 and goTo_floor_index == 3)
	{
		// LOG eachFloorAltDiff
		for(const auto& elem : eachFloorAltDiff)
		{
			//UE_LOG(LogTemp, Log, TEXT("Each Floor Alt Diff: %f"), elem);
		}
	}

	for(const auto& each_alt_diff : eachFloorAltDiff)
	{
		if(current_floor_index == 13 and goTo_floor_index == 3)
		{
			//UE_LOG(LogTemp, Log, TEXT("TTA Temp: %f"), tta_temp);
			//UE_LOG(LogTemp, Log, TEXT("TTM Temp: %f"), ttm_temp);
			//UE_LOG(LogTemp, Log, TEXT("TTD Temp: %f"), ttd_temp);
		}
		// LOG tta_temp, ttm_temp, ttd_temp
		//UE_LOG(LogTemp, Log, TEXT("TTA Temp: %f"), tta_temp);
		//UE_LOG(LogTemp, Log, TEXT("TTM Temp: %f"), ttm_temp);
		//UE_LOG(LogTemp, Log, TEXT("TTD Temp: %f"), ttd_temp);
		
		if(tta_temp < tta)
		{
			// Get V0 at this moment
			const double v0 = acc * tta_temp;
			// Get Rest Time to rach maxvel and round to second floating point
			const double rest_time = FMath::RoundToDouble((max_velocity - v0) / acc * 100.0f) / 100.0f;

			// Get distance using v0, maxvel, and rest_time
			const double distance_left_tta = (FMath::Pow(max_velocity, 2) -  FMath::Pow(v0, 2)) / (2*acc);

			// Check if distance_left_tta is less than each_alt_diff
			if(distance_left_tta < each_alt_diff)
			{
				// Get rest alt
				const double rest_alt = each_alt_diff - distance_left_tta;

				// divide this to maxvel to get time and round to second floating point
				const double rest_time_ttm = FMath::RoundToDouble(rest_alt / max_velocity * 100.0f) / 100.0f;
				eachFloorTime.Add(rest_time + rest_time_ttm);

				tta_temp += rest_time + rest_time_ttm;
				ttm_temp += rest_time_ttm;

				eachfloorAlt.Add(tta+rest_time_ttm);
			}
			else
			{
				const double time = FMath::Sqrt((2*each_alt_diff)/acc);
				const double rounded_time = FMath::RoundToDouble(time * 100.0f) / 100.0f;
				tta_temp += rounded_time;

				eachfloorAlt.Add(rounded_time);
			}
		}

		else if(ttm_temp < ttm)
		{
			// Get Distance from ttm-ttm_temp
			const double distance = max_velocity * (ttm - ttm_temp);

			// LOG
			if(current_floor_index == 13 and goTo_floor_index == 3)
			{
				UE_LOG(LogTemp, Log, TEXT("Distance: %f"), distance);
			}
			// Compare distance to each_alt_diff
			if(each_alt_diff < distance)
			{
				// Get Time to reach each_alt_diff
				const double time = each_alt_diff / max_velocity;
				const double rounded_time = FMath::RoundToDouble(time * 100.0f) / 100.0f;
				ttm_temp += rounded_time;

				eachFloorTime.Add(rounded_time);
			}
			else
			{
				const double rest_distance = each_alt_diff - distance;
				const double rest_time_ttm = FMath::RoundToDouble(distance / max_velocity * 100.0f) / 100.0f; 

				const double t_temp = FMath::Abs( max_velocity * max_velocity + 2 * (-1 * acc) * rest_distance);
				const double rest_time_ttd = FMath::Abs((-1 * max_velocity + FMath::Sqrt((t_temp))) / acc);

				
				// LOG -1 * max_velocity + FMath::Sqrt((max_velocity * max_velocity + 2 * (-1 * acc) * rest_distance))
				//UE_LOG(LogTemp, Log, TEXT("Rest Time TTD: %f"), (rest_time_ttd));

				ttd_start_v0 = max_velocity - acc * rest_time_ttd;
				// Round Rest Time to .2f
				const double rounded_time = FMath::RoundToDouble((rest_time_ttm + rest_time_ttd) * 100.0f) / 100.0f;

				ttm_temp += rounded_time;
				ttd_temp += rest_time_ttd;

				eachFloorTime.Add(rounded_time);
			}
		}

		else if(ttd_temp < ttd)
		{
			// Get Distance from ttd-ttd_temp
			const double t_temp = FMath::Abs( ttd_start_v0 * ttd_start_v0 + 2 * (-1 * acc) * each_alt_diff);
			const double rest_time_ttd = FMath::Abs((-1 * ttd_start_v0 + FMath::Sqrt((t_temp))) / acc);
			
			ttd_temp += rest_time_ttd;
			eachFloorTime.Add(rest_time_ttd);

			ttd_start_v0 = max_velocity - acc * rest_time_ttd;
		}
	}

	// LOG Elevator name
	UE_LOG(LogTemp, Log, TEXT("Elevator Name: %s, Start Floor Index %d -> End Floor Index %d / tta : %f ttm : %f, ttd : %f"), *FString(this_status.device_name), current_floor_index, goTo_floor_index, tta, ttm, ttd);
		
	// LOG All elem of eachFloorTime
	const int count = current_floor_index < goTo_floor_index ? 1 : -1;
	int start = current_floor_index;
	for(const auto& elem : eachFloorTime)
	{
		UE_LOG(LogTemp, Log, TEXT("Each Floor Time of Elevator [%d -> %d] = %f"), start, start+count, elem);
		start += count;
	}
	return eachFloorTime;
}

void AMyActor::SetMapPath()
{
	this->status.map_path = FPaths::ProjectContentDir() + "EV_VIS/Maps/" + this->status.building_name + ".umap";
}

bool AMyActor::CheckAndAppendMapList()
{
	if(this->loaded_building_list.Find(this->status.building_name) != nullptr)
	{
		return false;
	}
	else
	{
		this->loaded_building_list.Add(this->status.building_name, true);
		return true;
	}
}

void AMyActor::emptyMapList()
{
	this->loaded_building_list.Empty();
}

void AMyActor::FilpFlopMapList(FString building_name)
{
	// FIND VALUE OF BULIDGING NAME
	bool* value = this->loaded_building_list.Find(building_name);

	// CHANGE VALUE
	*value = !(*value);
}
