#pragma once
#include <ShlObj.h>
#include <commdlg.h>
#include "socket_oneM2M.h"
#include "parse_json.h"

using namespace std;

class dt_simulation
{
public:

    OPENFILENAME ofn;
    socket_oneM2M* oneM2MSocket;
    wchar_t* fileLocation;

    // 1st dimension will become each Building Name, 2nd will be each Elevator Name
    //vector<vector<wstring>> thisTestCaseResourceInfo;
    vector<wstring> TCNames;
    map<wstring, map<wstring, int>> thisTestCaseResourceInfo;

    dt_simulation();

    void setSimulatedFileLocation();
    wchar_t getSimulatedFileLocation() const;

    void run();
    void readFileAndCreateoneM2MResource(wstring TCName);
    void putSimulatedStringTooneM2M(Wparsed_struct parsedStruct);
};