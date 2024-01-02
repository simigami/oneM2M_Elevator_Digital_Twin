#include "socket_oneM2M.h"

socket_oneM2M::socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) : socket(parsed_struct)
{
	system_clock::time_point start = system_clock::now();

    string building_name = parsed_struct.building_name;
    string ACOR_NAME = parsed_struct.building_name;

    string originator_name = "C" + parsed_struct.device_name;
    string device_name = parsed_struct.device_name;

    try
    {
        this->socket_name = building_name;

        std::cout << "Updating This Building ACP\n"  << std::endl;
        socket.acp_update(parsed_struct, ACP_NAMES, 0);
        socket.ae_create(parsed_struct);

        Default_CNTs.emplace_back("Elevator_physics");
        Default_CNTs.emplace_back("Elevator_button_inside");
        Default_CNTs.emplace_back("Elevator_button_outside");

        auto res = create_oneM2M_under_device_name(parsed_struct);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception caught on socket_oneM2M::socket_oneM2M() " << e.what() << std::endl;
    }
    //Check Each AE Exists based on DB, if no AE then make AE (in DEV)

    //Check Latest CIN timestamp based on latest log, if new CIN then get new CIN and calculate logic312262
}

socket_oneM2M::~socket_oneM2M()
{
}

bool socket_oneM2M::create_oneM2M_under_device_name(parse_json::parsed_struct parsed_struct)
{
    try
    {
        system_clock::time_point start = system_clock::now();

        send_oneM2M s = this->socket;

        string building_name = parsed_struct.building_name;
		string originator_name = "C" + parsed_struct.building_name;
        string device_name = parsed_struct.device_name;

        Default_PHYSICS_CNTs.emplace_back(device_name +"_Velocity");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Altimeter");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Temperature");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Trip");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Distance");

        Default_INSIDE_CNTs.emplace_back(device_name +"_Button_List");
        Default_INSIDE_CNTs.emplace_back(device_name +"_GoTo");

        socket.cnt_create(parsed_struct, 1, device_name);

	    for(const string& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(parsed_struct, 2, device_name, CNT_NAME);
        }

        for(const string& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(const string& CNT_NAME : Default_INSIDE_CNTs)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[1], CNT_NAME);
        }
        for(const string& CNT_NAME : Default_OUTSIDE_CNTs)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[2], CNT_NAME);
        }

        string payload = "None";
        for(int i = parsed_struct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
            s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
        }
        for(int i = 1 ; i<=parsed_struct.ground_floor ; i++)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[2], std::to_string(i));
            s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], std::to_string(i));
        }

		// Create or Update Velocity, Altimeter, Temperature
        if(parsed_struct.velocity != -1)
        {
            payload = std::to_string(parsed_struct.velocity);
            s.cin_create(originator_name, "velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
        }
        if(parsed_struct.altimeter != -1)
        {
            payload = std::to_string(parsed_struct.altimeter);
            s.cin_create(originator_name, "altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
        }
        if(parsed_struct.temperature != -1)
        {
            payload = std::to_string(parsed_struct.temperature);
            s.cin_create(originator_name, "temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
        }
        if(!parsed_struct.button_inside.empty())
        {
            payload = "";
            for(string temp : parsed_struct.button_inside)
            {
                payload += " " + temp;
            }
            s.cin_create(originator_name, "button_inside", payload, 3, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
        }

        if(!parsed_struct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parsed_struct.button_outside)
	        {
                string floor = each_floor[0] < 0 ? "B"+std::to_string((each_floor[0]*-1)) : std::to_string(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    string payload = "Up";
	                s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], floor);
                }
                else
                {
	                string payload = "Down";
                    s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], floor);
                }
	        }
        }

        std::chrono::duration<double> delta = system_clock::now() - start;
        std::cout << "Time Spend on DT_SERVER is : "  << delta.count() << " s"<< std::endl;

		Default_PHYSICS_CNTs .clear();
		Default_INSIDE_CNTs .clear();
		Default_OUTSIDE_CNTs .clear();

        return true;
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
        return false;
    }
}

bool socket_oneM2M::create_oneM2M_under_CNTs(parse_json::parsed_struct parsed_struct)
{
	try
	{
        system_clock::time_point start = system_clock::now();

	    send_oneM2M s = this->socket;

	    string building_name = parsed_struct.building_name;
		string originator_name = "C" + parsed_struct.building_name;
	    string device_name = parsed_struct.device_name;

	    Default_PHYSICS_CNTs.emplace_back(device_name +"_Velocity");
	    Default_PHYSICS_CNTs.emplace_back(device_name +"_Altimeter");
	    Default_PHYSICS_CNTs.emplace_back(device_name +"_Temperature");
	    Default_PHYSICS_CNTs.emplace_back(device_name +"_Trip");
	    Default_PHYSICS_CNTs.emplace_back(device_name +"_Distance");

	    Default_INSIDE_CNTs.emplace_back(device_name +"_Button_List");
	    Default_INSIDE_CNTs.emplace_back(device_name +"_GoTo");

	    string payload = "None";
		// Create or Update Velocity, Altimeter, Temperature
	    if(parsed_struct.velocity != -1)
	    {
	        payload = std::to_string(parsed_struct.velocity);
	        s.cin_create(originator_name, "velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
	    }
	    if(parsed_struct.altimeter != -1)
	    {
	        payload = std::to_string(parsed_struct.altimeter);
	        s.cin_create(originator_name, "altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
	    }
	    if(parsed_struct.temperature != -1)
	    {
	        payload = std::to_string(parsed_struct.temperature);
	        s.cin_create(originator_name, "temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
	    }

	    if(!parsed_struct.button_inside.empty())
	    {
	        payload = "";
	        for(string temp : parsed_struct.button_inside)
	        {
	            payload += " " + temp;
	        }
	        s.cin_create(originator_name, "button_inside", payload, 3, device_name, Default_CNTs[1], Default_INSIDE_CNTs[0]);
	    }

        if(!parsed_struct.button_outside.empty())
        {
	        for(const vector<int> each_floor : parsed_struct.button_outside)
	        {
                string floor = each_floor[0] < 0 ? "B"+std::to_string((each_floor[0]*-1)) : std::to_string(each_floor[0]);
                int direction = each_floor[1];

                if(direction)
                {
                    string payload = "Up";
	                s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], floor);
                }
                else
                {
	                string payload = "Down";
                    s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], floor);
                }
	        }
        }
	    std::chrono::duration<double> delta = system_clock::now() - start;
	    std::cout << "Time Spend on DT_SERVER is : "  << delta.count() << " s"<< std::endl;

		Default_PHYSICS_CNTs .clear();
		Default_INSIDE_CNTs .clear();
		Default_OUTSIDE_CNTs .clear();
        return true;
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
        return false;
	}
}