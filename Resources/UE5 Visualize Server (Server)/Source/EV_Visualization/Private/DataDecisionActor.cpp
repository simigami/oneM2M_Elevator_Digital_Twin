// Fill out your copyright notice in the Description page of Project Settings.


#include "DataDecisionActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADataDecisionActor::ADataDecisionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADataDecisionActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADataDecisionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if new decision struct is changed
	if(isDecisionStructChanged())
	{
		// if changed, set latest decision struct to decision struct
		setLatestDecisionStruct(this->decisionStruct);
		// Execute Blueprint Implementable Function
		Execute(this->decisionStruct);
	}
}

FDecisionStruct ADataDecisionActor::deserialJSON(const FString& ReceivedJSON)
{
	FDecisionStruct newStruct;
	
	// Deserialize the JSON string 1. Make Reader
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ReceivedJSON);
	
	// Deserialize the JSON string 2. Make Object That Saves Deserialized JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// Deserialize the JSON string 3. Do Deserialization
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		newStruct.building_name = JsonObject->GetStringField(TEXT("building_name"));
		newStruct.device_name = JsonObject->GetStringField(TEXT("device_name"));

		// Set Map Path using content dir + building name + device name + umap
		newStruct.map_path = FPaths::Combine(FPaths::ProjectContentDir(), newStruct.building_name, "EV_VIS/Maps/", newStruct.building_name + ".umap");

		newStruct.goToFloor = JsonObject->GetIntegerField(TEXT("goToFloor"));

		newStruct.tta = JsonObject->GetNumberField(TEXT("tta"));
		newStruct.ttm = JsonObject->GetNumberField(TEXT("ttm"));
		newStruct.ttd = JsonObject->GetNumberField(TEXT("ttd"));
		
		newStruct.underground_floor = JsonObject->GetIntegerField(TEXT("underground_floor"));
		newStruct.ground_floor = JsonObject->GetIntegerField(TEXT("ground_floor"));
		
		newStruct.acceleration = JsonObject->GetNumberField(TEXT("acceleration"));
		newStruct.max_velocity = JsonObject->GetNumberField(TEXT("max_velocity"));
		
		if(newStruct.each_floor_altimeter.IsEmpty())
		{
			TArray<TSharedPtr<FJsonValue>> temp = JsonObject->GetArrayField(TEXT("each_floor_altimeter"));
			for (const TSharedPtr<FJsonValue>& JsonValue : temp)
			{
				// Ensure the JsonValue is valid and its type is number
				if (JsonValue.IsValid() && JsonValue->Type == EJson::Number)
				{
					// Get the number value as a float and add it to the DecimalsArray
					float DecimalValue = JsonValue->AsNumber();
					newStruct.each_floor_altimeter.Add(DecimalValue);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON data"));
	}

	return newStruct;
}

void ADataDecisionActor::InitDecisionStruct()
{
	this->decisionStruct = FDecisionStruct();
}

void ADataDecisionActor::setLatestDecisionStruct(FDecisionStruct& thisStruct)
{
	this->latestDecisionStruct = thisStruct;
}

void ADataDecisionActor::setDecisionStruct(FDecisionStruct& thisStruct)
{
	this->decisionStruct = thisStruct;
}

FDecisionStruct ADataDecisionActor::getDecisionStruct()
{
	return this->decisionStruct;
}

bool ADataDecisionActor::isDecisionStructChanged()
{
	// if decisionStruct is not empty, and value changed from latest decisionStruct

	// if building name empty = false
	if(decisionStruct.building_name.IsEmpty())
	{
		return false;
	}
	
	// if building name == latest decisionStruct building name  -> Check goTo Floor
	if(decisionStruct.building_name == latestDecisionStruct.building_name)
	{
		// if goToFloor == latest decisionStruct goToFloor -> return false
		if(decisionStruct.goToFloor == latestDecisionStruct.goToFloor)
		{
			return false;
		}
		else
		{
			// if goToFloor != latest decisionStruct goToFloor -> return true
			return true;
		}
	}
	else
	{
		// if building name != latest decisionStruct building name -> return true
		return true;
	}
}

TArray<double> ADataDecisionActor::getEachFloorTimeLines(FDecisionStruct this_status, const int current_floor_index, const int goTo_floor_index)
{
	FCriticalSection Mutex;
	FScopeLock ScopeLock(&Mutex);

	int dilation = this_status.dilation;
	
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
	const double ttm = this_status.ttm;
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

	// log if length of eachFloorAltDiff is 1
	if(eachFloorAltDiff.Num() == 1)
	{
		UE_LOG(LogTemp, Log, TEXT("Each Floor Alt Diff: %f"), eachFloorAltDiff[0]);
	}

	// check if length of eachFloorAltDiff is 1 and eachFloorAltDiff[0] 's distance is less than 2 * max_velocity * max_velocity / acc
	if(eachFloorAltDiff.Num() == 1 and eachFloorAltDiff[0] < acc * FMath::Pow(max_velocity / acc, 2))
	{
		eachFloorTime.Add((tta+ttd)/dilation);
	}

	else if(eachFloorAltDiff.Num() == 1)
	{
		eachFloorTime.Add((tta+ttm+ttd)/dilation);
	}

	else
	{
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
				double v0 = acc * tta_temp;
				// Get Rest Time to rach maxvel and round to second floating point
				double rest_time = FMath::RoundToDouble((max_velocity - v0) / acc * 100.0f) / 100.0f;

				// Get distance using v0, maxvel, and rest_time
				double distance_left_tta = (FMath::Pow(max_velocity, 2) -  FMath::Pow(v0, 2)) / (2*acc);

				// Check if distance_left_tta is less than each_alt_diff
				if(distance_left_tta < each_alt_diff)
				{
					// Get rest alt
					double rest_alt = each_alt_diff - distance_left_tta;

					// divide this to maxvel to get time and round to second floating point
					double rest_time_ttm = FMath::RoundToDouble(rest_alt / max_velocity * 100.0f) / 100.0f;
					eachFloorTime.Add((rest_time + rest_time_ttm)/dilation);

					tta_temp += rest_time + rest_time_ttm;
					ttm_temp += rest_time_ttm;

					eachfloorAlt.Add((tta+rest_time_ttm)/dilation);
				}
				else
				{
					double time = FMath::Sqrt((2*each_alt_diff)/acc);
					double rounded_time = FMath::RoundToDouble(time * 100.0f) / 100.0f;
					tta_temp += rounded_time;

					eachfloorAlt.Add(rounded_time/dilation);
				}
			}

			else if(ttm_temp < ttm)
			{
				// Get Distance from ttm-ttm_temp
				double distance = max_velocity * (ttm - ttm_temp);

				// LOG
				if(current_floor_index == 13 and goTo_floor_index == 3)
				{
					//UE_LOG(LogTemp, Log, TEXT("Distance: %f"), distance);
				}
				// Compare distance to each_alt_diff
				if(each_alt_diff < distance)
				{
					// Get Time to reach each_alt_diff
					double time = each_alt_diff / max_velocity;
					double rounded_time = FMath::RoundToDouble(time * 100.0f) / 100.0f;
					ttm_temp += rounded_time;

					eachFloorTime.Add(rounded_time/dilation);
				}
				else
				{
					double rest_distance = each_alt_diff - distance;
					double rest_time_ttm = FMath::RoundToDouble(distance / max_velocity * 100.0f) / 100.0f; 

					double t_temp = FMath::Abs( max_velocity * max_velocity + 2 * (-1 * acc) * rest_distance);
					double rest_time_ttd = FMath::Abs((-1 * max_velocity + FMath::Sqrt((t_temp))) / acc);

					
					// LOG -1 * max_velocity + FMath::Sqrt((max_velocity * max_velocity + 2 * (-1 * acc) * rest_distance))
					//UE_LOG(LogTemp, Log, TEXT("Rest Time TTD: %f"), (rest_time_ttd));

					ttd_start_v0 = max_velocity - acc * rest_time_ttd;
					// Round Rest Time to .2f
					double rounded_time = FMath::RoundToDouble((rest_time_ttm + rest_time_ttd) * 100.0f) / 100.0f;

					ttm_temp += rounded_time;
					ttd_temp += rest_time_ttd;

					eachFloorTime.Add(rounded_time/dilation);
				}
			}

			else if(ttd_temp < ttd)
			{
				// Get Distance from ttd-ttd_temp
				double t_temp = FMath::Abs( ttd_start_v0 * ttd_start_v0 + 2 * (-1 * acc) * each_alt_diff);
				double rest_time_ttd = FMath::Abs((-1 * ttd_start_v0 + FMath::Sqrt((t_temp))) / acc);
				
				ttd_temp += rest_time_ttd;
				eachFloorTime.Add(rest_time_ttd/dilation);

				ttd_start_v0 = max_velocity - acc * rest_time_ttd;
			}
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
