#include <cstdarg>
#include <codecvt>
#include <unordered_set>
#include <cpprest/http_client.h>

#include "socket_oneM2M.h"

using namespace std;

socket_oneM2M::socket_oneM2M(elevator_resource_status sc, Wparsed_struct parseStruct, vector<wstring> ACP_NAMES) : socket(parseStruct)
{
    building_name = parseStruct.building_name;
    originator_name = L"C" + parseStruct.building_name;
    device_name = parseStruct.device_name;

    try
    {
        switch (sc)
        {
        case ACP_NOT_FOUND:
            // FIRST CHECK THERE IS ACPS IN oneM2M CSE SERVER
            socket.acp_create_one_ACP(ACP_NAMES, 0);
            socket.ae_create(building_name);
            socket.cnt_create(1, device_name);
            Default_CNTs.emplace_back(L"Elevator_physics");
            Default_CNTs.emplace_back(L"Elevator_button_inside");
            Default_CNTs.emplace_back(L"Elevator_button_outside");

            create_oneM2M_CNTs(parseStruct);
            break;

        case THIS_ACP_NOT_FOUND:
			socket.acp_update(ACP_NAMES, 0);
			socket.ae_create(building_name);
			socket.cnt_create(1, device_name);

			Default_CNTs.emplace_back(L"Elevator_physics");
			Default_CNTs.emplace_back(L"Elevator_button_inside");
			Default_CNTs.emplace_back(L"Elevator_button_outside");

			create_oneM2M_CNTs(parseStruct);
			break;

        case BUILDING_NOT_FOUND:
            socket.acp_update(ACP_NAMES, 0);
            socket.ae_create(building_name);
            break;

        case ELEVATOR_NOT_FOUND:
            socket.cnt_create(1, device_name);

            Default_CNTs.emplace_back(L"Elevator_physics");
            Default_CNTs.emplace_back(L"Elevator_button_inside");
            Default_CNTs.emplace_back(L"Elevator_button_outside");

            create_oneM2M_CNTs(parseStruct);

            break;

        case BUILDING_DATA_NOT_FOUND:

            break;

        case ELEVATOR_DATA_NOT_FOUND:
            Default_CNTs.emplace_back(L"Elevator_physics");
            Default_CNTs.emplace_back(L"Elevator_button_inside");
            Default_CNTs.emplace_back(L"Elevator_button_outside");

            Default_PHYSICS_CNTs.emplace_back(device_name + L"_Velocity");
            Default_PHYSICS_CNTs.emplace_back(device_name + L"_Altimeter");
            Default_PHYSICS_CNTs.emplace_back(device_name + L"_Temperature");
            Default_PHYSICS_CNTs.emplace_back(device_name + L"_Trip");
            Default_PHYSICS_CNTs.emplace_back(device_name + L"_Distance");

            Default_INSIDE_CNTs.emplace_back(device_name + L"_Button_List");
            Default_INSIDE_CNTs.emplace_back(device_name + L"_GoTo");

            break;

        case ELEVATOR_FOUND:
            break;

        default:
            break;
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception caught on socket_oneM2M::socket_oneM2M() " << e.what() << std::endl;
    }
}

socket_oneM2M::socket_oneM2M(elevator_resource_status sc, Wparsed_struct parseStruct) : socket(parseStruct)
{
    TCName = parseStruct.TCName;
    originator_name = L"C" + parseStruct.TCName;

    building_name = parseStruct.building_name;
    device_name = parseStruct.device_name;
}

socket_oneM2M::~socket_oneM2M()
{
}

void socket_oneM2M::init(Wparsed_struct parseStruct)
{
    TCName = parseStruct.TCName;
    originator_name = L"C" + parseStruct.TCName;

    building_name = parseStruct.building_name;
    device_name = parseStruct.device_name;
}

bool socket_oneM2M::createBuilding(Wparsed_struct parseStruct)
{
	try
    {
        send_oneM2M s = this->socket;

        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        const wstring building_name = this->building_name;
        const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;
        const wstring timestamp = to_wstring(parseStruct.timestampOffset);

        s.cnt_create(1, building_name);
        s.cnt_create(2, building_name, device_name);

        Default_CNTs.emplace_back(L"Elevator_physics");
        Default_CNTs.emplace_back(L"Elevator_button_inside");
        Default_CNTs.emplace_back(L"Elevator_button_outside");

        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Velocity");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Altimeter");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Temperature");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Trip");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Distance");

        Default_INSIDE_CNTs.emplace_back(device_name +L"_Button_List");
        Default_INSIDE_CNTs.emplace_back(device_name +L"_GoTo");

	    for(const wstring& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(3, building_name, device_name, CNT_NAME);
        }

        for(const wstring& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(const wstring& CNT_NAME : Default_INSIDE_CNTs)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[1], CNT_NAME);
        }

        for(int i = parseStruct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[2], L"B"+to_wstring(i));
        }
        for(int i = 1 ; i<=parseStruct.ground_floor ; i++)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[2], to_wstring(i));
        }

        wstring payload;
		// Create or Update Velocity, Altimeter, Temperature
        if(parseStruct.velocity != -1)
        {
            payload = to_wstring(parseStruct.velocity) + L" " + timestamp;
            s.cin_create(L"velocity", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
        }
        if(parseStruct.altimeter != -1)
        {
            payload = to_wstring(parseStruct.altimeter) + L" " + timestamp;
            s.cin_create( L"altimeter", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
        }
        if(parseStruct.temperature != -1)
        {
            payload = to_wstring(parseStruct.temperature) + L" " + timestamp;
            s.cin_create( L"temperature", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
        }
        if(!parseStruct.button_inside.empty())
        {
            payload = L"";
            for(const wstring& temp : parseStruct.button_inside)
            {
                payload += L" " + temp;
            }
            payload += L" " + timestamp;

            s.cin_create( L"button_inside", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
        }

        if(!parseStruct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parseStruct.button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+ to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    payload = L"Up " + timestamp;
	                s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
                else
                {
	                payload = L"Down " + timestamp;
                    s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
	        }
        }
        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
        return false;
    }
}

bool socket_oneM2M::createElevator(Wparsed_struct parseStruct)
{
	try
    {
        device_name = parseStruct.device_name;

        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;
        const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;
        const wstring timestamp = to_wstring(parseStruct.timestampOffset);

        s.cnt_create(2, building_name, device_name);

	    for(const wstring& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(3, building_name, device_name, CNT_NAME);
        }

        for(const wstring& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(const wstring& CNT_NAME : Default_INSIDE_CNTs)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[1], CNT_NAME);
        }

        for(int i = parseStruct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[2], L"B"+to_wstring(i));
        }
        for(int i = 1 ; i<=parseStruct.ground_floor ; i++)
        {
            s.cnt_create(4, building_name, device_name, Default_CNTs[2], to_wstring(i));
        }

        wstring payload;
		// Create or Update Velocity, Altimeter, Temperature
        if(parseStruct.velocity != -1)
        {
            payload = to_wstring(parseStruct.velocity) + L" " + timestamp;
            s.cin_create(L"velocity", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
        }
        if(parseStruct.altimeter != -1)
        {
            payload = to_wstring(parseStruct.altimeter) + L" " + timestamp;
            s.cin_create( L"altimeter", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
        }
        if(parseStruct.temperature != -1)
        {
            payload = to_wstring(parseStruct.temperature) + L" " + timestamp;
            s.cin_create( L"temperature", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
        }
        if(!parseStruct.button_inside.empty())
        {
            payload = L"";
            for(const wstring& temp : parseStruct.button_inside)
            {
                payload += L" " + temp;
            }
            payload += L" " + timestamp;

            s.cin_create( L"button_inside", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
        }

        if(!parseStruct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parseStruct.button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+ to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    payload = L"Up " + timestamp;
	                s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
                else
                {
	                payload = L"Down " + timestamp;
                    s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
	        }
        }
        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
        return false;
    }
}

bool socket_oneM2M::create_oneM2M_under_CNTs(Wparsed_struct parseStruct)
{
	try
	{
	    send_oneM2M s = this->socket;

        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        const wstring building_name = this->building_name;
		const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;

	    Default_PHYSICS_CNTs.emplace_back(device_name +L"_Velocity");
	    Default_PHYSICS_CNTs.emplace_back(device_name +L"_Altimeter");
	    Default_PHYSICS_CNTs.emplace_back(device_name +L"_Temperature");
	    Default_PHYSICS_CNTs.emplace_back(device_name +L"_Trip");
	    Default_PHYSICS_CNTs.emplace_back(device_name +L"_Distance");

	    Default_INSIDE_CNTs.emplace_back(device_name +L"_Button_List");
	    Default_INSIDE_CNTs.emplace_back(device_name +L"_GoTo");

	    wstring payload = L"None";
		// Create or Update Velocity, Altimeter, Temperature
	    if(parseStruct.velocity != -1)
	    {
	        payload = to_wstring(parseStruct.velocity);
	        s.cin_create(L"velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
	    }
	    if(parseStruct.altimeter != -1)
	    {
	        payload = to_wstring(parseStruct.altimeter);
	        s.cin_create( L"altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
	    }
	    if(parseStruct.temperature != -1)
	    {
	        payload = to_wstring(parseStruct.temperature);
	        s.cin_create( L"temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
	    }

	    if(!parseStruct.button_inside.empty())
	    {
	        payload = L"";
	        for(const wstring& temp : parseStruct.button_inside)
	        {
	            payload += L" " + temp;
	        }
	        s.cin_create( L"button_inside", payload, 3, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
	    }

        payload = L"None";
        for(int i = parseStruct.underground_floor ; i>=1 ; i--)
        {
            bool flag = true;
            for(const vector<int> each_floor : parseStruct.button_outside)
            {
	            int floor = each_floor[0];
                if(i == floor*-1)
                {
                    //DO NOT MAKE NONE CIN
                    flag = false;
                    break;
                }
            }
            if(flag)
            {
            	s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], L"B"+to_wstring(i));
            }
        }

        for(int i = 1 ; i<=parseStruct.ground_floor ; i++)
        {
            bool flag = true;
            for(const vector<int> each_floor : parseStruct.button_outside)
            {
	            int floor = each_floor[0];
                if(i == floor)
                {
                    //DO NOT MAKE NONE CIN
                    flag = false;
                    break;
                }
            }
            if(flag)
            {
				s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], to_wstring(i));
            }
        }

        if(!parseStruct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parseStruct.button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    wstring payload = L"Up";
	                s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
                else
                {
	                wstring payload = L"Down";
                    s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
	        }
        }
        return true;
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
        return false;
	}
}

bool socket_oneM2M::create_oneM2M_CNTs(Wparsed_struct parseStruct)
{
    try
    {
        send_oneM2M s = this->socket;

        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        const wstring building_name = this->building_name;
        const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;

        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Velocity");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Altimeter");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Temperature");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Trip");
        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Distance");

        Default_INSIDE_CNTs.emplace_back(device_name +L"_Button_List");
        Default_INSIDE_CNTs.emplace_back(device_name +L"_GoTo");

	    for(const wstring& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(2, device_name, CNT_NAME);
        }
        for(const wstring& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(3, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(const wstring& CNT_NAME : Default_INSIDE_CNTs)
        {
            s.cnt_create(3, device_name, Default_CNTs[1], CNT_NAME);
        }

        for(int i = parseStruct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(3, device_name, Default_CNTs[2], L"B"+to_wstring(i));
        }
        for(int i = 1 ; i<=parseStruct.ground_floor ; i++)
        {
            s.cnt_create(3, device_name, Default_CNTs[2], to_wstring(i));
        }
        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on socket_oneM2M::create_oneM2M_CNTs: " << e.what() << std::endl;
        exit(0);
    }
}

bool socket_oneM2M::create_oneM2M_SUBs(Wparsed_struct parseStruct, map<wstring, wstring>* mapper)
{
    try
    {
        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;
        const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;

        for(const wstring& CNT_NAME : Default_PHYSICS_CNTs)
        {
            const wstring sub_name = L"Sub_" + CNT_NAME;
            s.sub_create(mapper, originator_name, 4, device_name, Default_CNTs[0], CNT_NAME, sub_name);
        }
        for(const wstring& CNT_NAME : Default_INSIDE_CNTs)
        {
            const wstring sub_name = L"Sub_" + CNT_NAME;
            s.sub_create(mapper, originator_name, 4, device_name, Default_CNTs[1], CNT_NAME, sub_name);
        }
        for(int i = parseStruct.underground_floor ; i>=1 ; i--)
        {
            const wstring sub_name = L"Sub_" + device_name + L"_" + L"B"+to_wstring(i);
            s.sub_create(mapper, originator_name, 4, device_name, Default_CNTs[2], L"B"+to_wstring(i), sub_name);
        }
        for(int i = 1 ; i<=parseStruct.ground_floor ; i++)
        {
            const wstring sub_name = L"Sub_" + device_name + L"_" + to_wstring(i);
            s.sub_create(mapper, originator_name, 4, device_name, Default_CNTs[2], to_wstring(i), sub_name);
        }

#ifdef oneM2M_ACME
        write_subscription_mapper_to_file(*mapper);
#endif

        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on socket_oneM2M::create_oneM2M_CNTs: " << e.what() << std::endl;
        exit(0);
    }
}

bool socket_oneM2M::write_subscription_mapper_to_file(map<wstring, wstring> mapper)
{
#ifdef  oneM2M_ACME
    // Iterate through the map and write each key-value pair to the file
    for (const auto& entry : mapper) {
        wstring sub_ri = entry.first;
        wstring rn_path = entry.second;

        if (!modify_mapper_to_file(rn_path, sub_ri)) {
            std::wofstream outFile(DEFAULT_SUB_RI_FILE_NAME, std::ios::app);
            if (!outFile) {
                std::cerr << "Error: Unable to open file " << DEFAULT_SUB_RI_FILE_NAME << " for writing." << std::endl;
                return false;
            }
            outFile << entry.first << " " << entry.second << '\n';
            outFile.close();
        }
    }

    // Close the file
#endif //  oneM2M_ACME
    return true;
}

bool socket_oneM2M::modify_mapper_to_file(wstring search_string, wstring change_string)
{
#ifdef oneM2M_ACME
    string file_path;
    file_path.append(DEFAULT_SUB_RI_FILE_NAME);

    std::wifstream file(DEFAULT_SUB_RI_FILE_NAME, std::ios::app);
    if (!file.is_open()) {
        cerr << "파일을 열 수 없습니다: " << DEFAULT_SUB_RI_FILE_NAME << endl;
        return false;
    }

    // 수정된 내용을 저장할 임시 파일을 생성합니다.
    string temp_file_path;
    temp_file_path.append(DEFAULT_SUB_RI_FILE_NAME);
    temp_file_path.append(".tmp");

    wofstream output_file(temp_file_path);
    if (!output_file.is_open()) {
        cerr << "임시 파일을 생성할 수 없습니다: " << temp_file_path << endl;
        return false;
    }

    wstring line;
    bool modified = false;

    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string findString = converter.to_bytes(search_string);
    string changeString = converter.to_bytes(change_string);

    // 원본 파일을 읽고 수정된 내용을 임시 파일에 기록합니다.
    while (getline(file, line)) {
        wstringstream wiss(line);
        std::vector<std::wstring> values;
        std::wstring item;

        while (getline(wiss, item, L' '))
        {
            values.push_back(item);
        }

        if (search_string == values[1]) {
            //cout << "OVERLAP FOUND" << endl;
            output_file << change_string << L" " << search_string << endl;
            modified = true;
        }
        else {
            output_file << line << endl;
        }
    }

    // 찾을 문자열을 포함하는 줄이 없으면 추가합니다.
    if (!modified) {
        return false;
    }

    // 임시 파일을 원본 파일로 변경합니다.
    file.close();
    output_file.close();

    if (remove(file_path.c_str()) != 0) {
        cerr << "원본 파일을 삭제할 수 없습니다: " << file_path << endl;
        return false;
    }
    if (rename(temp_file_path.c_str(), file_path.c_str()) != 0) {
        cerr << "임시 파일을 원본 파일로 변경할 수 없습니다: " << temp_file_path << " -> " << file_path << endl;
        return false;
    }
#endif // oneM2M_ACME
    return true;
}

bool socket_oneM2M::create_oneM2M_CINs(Wparsed_struct parseStruct)
{
	try
	{
        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;
		const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;
        const wstring timestamp = parseStruct.timestamp;

	    wstring payload = L"None";
		// Create or Update Velocity, Altimeter, Temperature
	    if(parseStruct.velocity != -1)
	    {
	        payload = to_wstring(parseStruct.velocity);
	        s.cin_create(L"velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
	    }
	    if(parseStruct.altimeter != -1)
	    {
	        payload = to_wstring(parseStruct.altimeter);
	        s.cin_create( L"altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
	    }
	    if(parseStruct.temperature != -1)
	    {
	        payload = to_wstring(parseStruct.temperature);
	        s.cin_create( L"temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
	    }

	    if(!parseStruct.button_inside.empty())
	    {
	        payload = L"";
	        for(const wstring& temp : parseStruct.button_inside)
	        {
	            payload += L" " + temp;
	        }
            payload += L" ";
	        s.cin_create( L"button_inside", payload, 3, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
	    }

        if(!parseStruct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parseStruct.button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    payload = L"Up ";
	                s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
                else
                {
	                payload = L"Down ";
                    s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
	        }
        }
        return true;
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
        return false;
	}
}

bool socket_oneM2M::createNewData(Wparsed_struct parseStruct)
{
	try
    {
        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;;
		const wstring originator_name = this->originator_name;
        const wstring device_name = this->device_name;
		const wstring timestamp = to_wstring(parseStruct.timestampOffset);

        wstring payload;
		// Create or Update Velocity, Altimeter, Temperature
        if(parseStruct.velocity != -1)
        {
            payload = to_wstring(parseStruct.velocity) + L" " + timestamp;
            s.cin_create(L"velocity", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
        }
        if(parseStruct.altimeter != -1)
        {
            payload = to_wstring(parseStruct.altimeter) + L" " + timestamp;
            s.cin_create( L"altimeter", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
        }
        if(parseStruct.temperature != -1)
        {
            payload = to_wstring(parseStruct.temperature) + L" " + timestamp;
            s.cin_create( L"temperature", payload, 4, building_name, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
        }
        if(!parseStruct.button_inside.empty())
        {
            payload = L"";
            for(const wstring& temp : parseStruct.button_inside)
            {
                payload += L" " + temp;
            }
            payload += L" " + timestamp;

            s.cin_create( L"button_inside", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
        }

        if(!parseStruct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parseStruct.button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+ to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    payload = L"Up " + timestamp;
	                s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
                else
                {
	                payload = L"Down " + timestamp;
                    s.cin_create(L"status", payload, 4, building_name, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
                }
	        }
        }
        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
        return false;
    }
}

bool socket_oneM2M::create_oneM2M_CIN_EnergyConsumption(Wparsed_struct parseStruct)
{
    try
    {
        send_oneM2M s = this->socket;

        building_name = parseStruct.building_name;
        device_name = parseStruct.device_name;

        const wstring building_name = this->building_name;
        const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;

        const wstring energy_cnt_name = device_name + L"_Energy";

        s.cnt_create(2, device_name, energy_cnt_name);

        // get idle, standby, iso reference energy to wstring
        const wstring idle_power = to_wstring(parseStruct.idle_power);
        const wstring standby_power = to_wstring(parseStruct.standby_power);
        const wstring iso_reference_cycle_energy = to_wstring(parseStruct.iso_power);

        const wstring payload = idle_power + L" " + standby_power + L" " + iso_reference_cycle_energy;

        s.cin_create(L"power_info", payload, 2, device_name, energy_cnt_name);

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught on socket_oneM2M::create_oneM2M_CNTs: " << e.what() << std::endl;
        exit(0);
    }
}

bool socket_oneM2M::create_oneM2M_CIN_Except_Button_Outside(Wparsed_struct parseStruct)
{
	try
	{
        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;
		const wstring originator_name = L"C" + this->building_name;
        const wstring device_name = this->device_name;

	    wstring payload = L"None";
		// Create or Update Velocity, Altimeter, Temperature
	    if(parseStruct.velocity != -1)
	    {
	        payload = to_wstring(parseStruct.velocity);
	        s.cin_create(L"velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
	    }
	    if(parseStruct.altimeter != -1)
	    {
	        payload = to_wstring(parseStruct.altimeter);
	        s.cin_create( L"altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
	    }
	    if(parseStruct.temperature != -1)
	    {
	        payload = to_wstring(parseStruct.temperature);
	        s.cin_create( L"temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
	    }

	    if(!parseStruct.button_inside.empty())
	    {
	        payload = L"";
	        for(wstring temp : parseStruct.button_inside)
	        {
	            payload += L" " + temp;
	        }
	        s.cin_create( L"button_inside", payload, 3, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
	    }
        return true;
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
        return false;
	}
}

bool socket_oneM2M::create_oneM2M_CIN_Only_Button_Outside(vector<vector<int>> button_outside)
{
	try
	{
        send_oneM2M s = this->socket;

        const wstring building_name = this->building_name;
		const wstring originator_name = L"C" + building_name;
        const wstring device_name = this->device_name;

	    wstring payload;
		// Create or Update Velocity, Altimeter, Temperature
        if(!button_outside.empty())
        {
	        for(const vector<int> each_floor : button_outside)
	        {
                wstring floor = each_floor[0] < 0 ? L"B"+to_wstring((each_floor[0]*-1)) : to_wstring(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    payload = L"Up";
	                s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
                else
                {
	                payload = L"Down";
                    s.cin_create(L"status", payload, 3, device_name, Default_CNTs[2], floor);
                }
	        }
        }
        return true;
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
        return false;
	}
}

bool socket_oneM2M::check_oneM2M_CNT(Wparsed_struct parseStruct)
{
    try
    {
        send_oneM2M s = this->socket;

        return s.cnt_validate(1, device_name);
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on socket_oneM2M::create_oneM2M_CNTs: " << e.what() << std::endl;
        exit(0);
    }
}

vector<vector<wstring>> socket_oneM2M::retrieve_oneM2M_cins(vector<int> floor_info)
{
    send_oneM2M s = this->socket;

    vector<vector<wstring>> ret; // ret  = {velocity, altimeter, button_inside, button_outside}
    vector<wstring> ret_vel, ret_alt, ret_bin, ret_bout; // EACH CON STRING

    wstring building_name = this->building_name;
    wstring originator_name = L"C" + building_name;
    wstring device_name = this->device_name;

    web::http::http_response res;
    web::json::value response_json;
    std::wstring con;
    std::wstring con_string;
	try
	{
        if(this->Default_CNTs.empty())
        {
	        std::cerr << "ERROR OCCURED ON socket_oneM2M :  Default_CNTs is Empty" << std::endl;
            
            return vector<vector<wstring>>();
        }
        else
        {
            Default_PHYSICS_CNTs.emplace_back(device_name +L"_Velocity");
	        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Altimeter");
	        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Temperature");
	        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Trip");
	        Default_PHYSICS_CNTs.emplace_back(device_name +L"_Distance");

	        Default_INSIDE_CNTs.emplace_back(device_name +L"_Button_List");
	        Default_INSIDE_CNTs.emplace_back(device_name +L"_GoTo"); //IN_DEV

            //CIN RETRIEVE OF VELOCITY
            res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[0], this->Default_PHYSICS_CNTs[0]);

            response_json = res.extract_json().get();
			con = response_json[U("m2m:cin")][U("con")].as_string();

            con_string.assign(con.begin(), con.end());
            ret_vel.push_back(con_string);

            //std::cout << "VELOCITY CON IS : " << con_string << std::endl;

            //CIN RETRIEVE OF Altimeter
        	res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[0], this->Default_PHYSICS_CNTs[1]);

            response_json = res.extract_json().get();
			con = response_json[U("m2m:cin")][U("con")].as_string();

            con_string.assign(con.begin(), con.end());
            ret_alt.push_back(con_string);

            //std::cout << "ALTIMETER CON IS : " << con_string << std::endl;

            //CIN RETRIEVE OF BUTTON INSIDE
            res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[1], this->Default_INSIDE_CNTs[0]);
            response_json = res.extract_json().get();
            //CHECK IF BUTTON INSIDE IS EMPTY
            if(response_json.has_field(U("m2m:cin")) && response_json[U("m2m:cin")].has_field(U("con")))
            {
	            //DISASSEMBLE CON
				con = response_json[U("m2m:cin")][U("con")].as_string();

	            con_string.assign(con.begin(), con.end());
	            //std::cout << "BUTTON INSIDE CON IS : " << con_string << std::endl;

	            wstringstream iss(con_string);
	            wstring value;
	            while(iss >> value)
	            {
		            ret_bin.push_back(value);
	            }

            }
            else
            {
            	ret_bin = {};
            }

            for(int i = floor_info[0] ; i>=1 ; i--)
	        {
                wstring floor = L"B"+to_wstring(i);
				res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[2], floor);

	            //DISASSEMBLE CON
	            response_json = res.extract_json().get();
				con = response_json[U("m2m:cin")][U("con")].as_string();

                if(con != L"None")
                {
	                con_string.assign(con.begin(), con.end());
                    wstring result_string = floor + L":" + con_string;

					ret_bout.push_back(result_string);
                }
	        }

	        for(int i = 1 ; i<=floor_info[1] ; i++)
	        {
                wstring floor = to_wstring(i);
                res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[2], floor);

	            //DISASSEMBLE CON
	            response_json = res.extract_json().get();
				con = response_json[U("m2m:cin")][U("con")].as_string();

                if(con != L"None")
                {
	                con_string.assign(con.begin(), con.end());
                    wstring result_string = floor + L":" + con_string;

                    //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : " << result_string;
					ret_bout.push_back(result_string);
                }

                //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : ";
	            //std::std::cout << con << std::endl;
	        }

            ret.push_back(ret_vel);
            ret.push_back(ret_alt);
            ret.push_back(ret_bin);
            ret.push_back(ret_bout);
        }
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
	}
    return ret;
}

vector<double> socket_oneM2M::retrieve_oneM2M_Energy_CIN(Wparsed_struct parseSstruct)
{
    send_oneM2M s = this->socket;

    wstring building_name = this->building_name;
    wstring originator_name = L"C" + building_name;
    wstring device_name = this->device_name;

    web::http::http_response res;
    web::json::value response_json;
    std::wstring con;

    vector<double> energy_info;
    try
    {
        //CIN RETRIEVE
        res = s.cin_retrieve_la(originator_name, 2, device_name, device_name + L"_Energy");

        response_json = res.extract_json().get();
        con = response_json[U("m2m:cin")][U("con")].as_string();

        // con info is [idle] [standby] [iso_reference_cycle_energy]
        // move this string to vector<double> energy_info
        wstringstream iss(con);
        wstring value;
        while (iss >> value)
        {
            energy_info.push_back(stod(value));
		}
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception Caught : " << e.what() << std::endl;
    }

    return energy_info;
}
