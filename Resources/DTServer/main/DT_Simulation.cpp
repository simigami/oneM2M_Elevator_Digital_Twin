#include "DT_Simulation.h"
#include "config.h"
#include "socket_oneM2M.h"
#include "send_oneM2M.h"
#include <Shlwapi.h>
#include <algorithm>
#include <fstream>
#include <string>

dt_simulation::dt_simulation()
{
}

void dt_simulation::setSimulatedFileLocation()
{
	wchar_t szFile[MAXFILEPATHLENGTH] = { 0 };
    bool isTXTFlag = false;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    std::wcout << "IN SERVER -> Choose a txt file that has Data for Elevator..." << std::endl;
    Sleep(1000);

    while(!isTXTFlag)
    {
	    // 파일 선택 대화 상자 열기
	    if (GetOpenFileName(&ofn) == TRUE) 
	    {
            if(PathMatchSpecW(ofn.lpstrFile, L"*.txt"))
	        // 사용자가 파일을 선택한 경우 선택된 파일의 경로 출력
            {
	            isTXTFlag = true;
                ofn.lpstrFile = szFile;
            }
            else 
		    {
		        // 사용자가 취소한 경우 메시지 출력
		        std::wcout << L"IN SERVER -> Selected File is not txt, Please Choose a txt file..." << std::endl;
            	Sleep(500);
		    }
	    }
    }

    return;
}

wchar_t dt_simulation::getSimulatedFileLocation() const
{
    return *(this->ofn.lpstrFile);
}

void dt_simulation::run()
{
    vector<wstring> TCs;
    wstring TCName;

    std::wcout << L"Please Enter a TestCase Name, This Name Will Be Your AE Name : ";
    wcin >> TCName;

	std::wcout << L"Your TestCase Name is \"" << TCName << "\"" << std::endl;

    bool flag = false;
    while(!flag)
    {
        if(find(this->TCNames.begin(), this->TCNames.end(), TCName) == TCNames.end())
        {
            TCNames.push_back(TCName);
            oneM2MSocket->socket.acp_create_one_ACP(TCNames, 0);
            oneM2MSocket->socket.ae_create(TCName);
            flag = true;
        }
        else
        {
	        std::wcout << L"Please Enter a TestCase Name, This Name Will Be Your AE Name : ";
			wcin >> TCName;
			std::wcout << L"Your TestCase Name is \"" << TCName << "\"" << std::endl;
        }
    }

    this->setSimulatedFileLocation();
	this->readFileAndCreateoneM2MResource(TCName);

    std::wcout << L"Test Data Complete" << std::endl;
}

void dt_simulation::readFileAndCreateoneM2MResource(wstring TCName)
{
	wchar_t path = getSimulatedFileLocation();
    parse_json parsingClass;
	wstring wline;
    wifstream wfile;

    wfile.open(L"E:\\ML\\Elevator Git\\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\\Resources\\Sensors\\testlog2.txt");

    if (!wfile.is_open()) 
    {
        std::wcerr << L"IN SERVER -> dt_simulation ERROR : Unable to open file." << std::endl;
        return;
    }

    while (getline(wfile, wline)) // Read each line from the file
    {
        Wparsed_struct eachLineParsedStruct = parsingClass.parsingText(wline);
        eachLineParsedStruct.TCName = TCName;

    	oneM2MSocket->init(eachLineParsedStruct);

        auto it = thisTestCaseResourceInfo.find(eachLineParsedStruct.building_name);

        if(it == thisTestCaseResourceInfo.end())
        {
            oneM2MSocket->createBuilding(eachLineParsedStruct);
            thisTestCaseResourceInfo.emplace(make_pair(eachLineParsedStruct.building_name, map<wstring, int>()));
            thisTestCaseResourceInfo[eachLineParsedStruct.building_name].emplace(make_pair(eachLineParsedStruct.device_name, 1));
        }
        else
        {
            auto itit = thisTestCaseResourceInfo[eachLineParsedStruct.building_name].find(eachLineParsedStruct.device_name);
            if(itit == thisTestCaseResourceInfo[eachLineParsedStruct.building_name].end())
			{
            	oneM2MSocket->createElevator(eachLineParsedStruct);
	            thisTestCaseResourceInfo[eachLineParsedStruct.building_name].emplace(make_pair(eachLineParsedStruct.device_name, 1));
			}
            else
            {
            	oneM2MSocket->createNewData(eachLineParsedStruct);
            }
        }
    }

    wfile.close(); // Close the file
}

void dt_simulation::putSimulatedStringTooneM2M(Wparsed_struct parsedStruct)
{
    /*
    send_oneM2M oneM2MSocket;

	int ACP_FLAG;

    wstring ACOR_NAME = parsedStruct.building_name;
    wstring AE_NAME = parsedStruct.building_name;
    wstring CNT_NAME = parsedStruct.device_name;

    ACP_FLAG = oneM2MSocket.acp_validate(ACOR_NAME, 0);

    // CHECK THIS ACP Exists
    if(ACP_FLAG == 0 || ACP_FLAG == 1) // oneM2M에 빌딩 ACP가 존재하지 않는 경우
    {
    	std::wcout << L"IN SERVER : " << L" ACP : C" << parsedStruct.building_name << L"Not Exists" << std::endl;

    	Building* newBuilding;
        this->ACP_NAMES.push_back(ACOR_NAME);

        // Create THIS CNT(Building's Elevator) Class
        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

        newBuilding->buildingElevatorInfo->classOfAllElevators.push_back(thisBuildingElevator);
        newBuilding->buildingElevatorInfo->ACP_NAME = ACOR_NAME;

        allBuildingInfo.push_back(newBuilding);

        thisBuildingElevator->runElevator();
	    thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct);

    	thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
    }
    else // oneM2M에 빌딩 ACP가 존재하는 경우
    {
        std::wcout << "IN SERVER -> Building ACP Exists, Check Building " << AE_NAME << " Exists In Server" << std::endl;

        // GET THIS BUILDING CLASS and CHECK THIS CNT(Device Name) Exists
        Building* thisBuildingFlag = this->get_building_vector(ACOR_NAME);

        // if TEMP == NULL, It means oneM2M Building Resource Exists, But Server Restarted

        std::wcout << "IN SERVER -> Building Resource Exists, Check Elevator " << CNT_NAME << " Resource Exists IN Server" << std::endl;

        bool flag = this->existsElevator(thisBuildingFlag, CNT_NAME);

        if(flag) // oneM2M과 서버에 해당 엘리베이터 클래스가 모두 존재하는 경우
        {
            // THIS CNT Exists
            Elevator* thisBuildingElevator = this->getElevator(thisBuildingFlag, CNT_NAME);

            // Update CIN Value
            std::wcout << "IN SERVER -> Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << std::endl;

        	thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
        }
        else // oneM2M에 해당 엘리베이터 클래스가 존재하지 않는 경우
        {
            //CHECK THIS CNT EXISTS in oneM2M
            string originator_name = "C" + AE_NAME;

        	ACP_Validation_Socket.URL_TO_AE = ACP_Validation_Socket.URL_TO_CSE + "/" + AE_NAME;

        	bool flag = ACP_Validation_Socket.cnt_validate(originator_name, 1, CNT_NAME); //수정 필요, 엘리베이터 CNT 존재하는 지 확인

            std::wcout << "IN SERVER -> Elevator Resource Not Exist in DT Server... Create New CNT " << CNT_NAME << " Resource" << std::endl;
	        this->ACP_NAMES.push_back(ACOR_NAME);

            int algNumber;

        	thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
		}
    }*/
}
