#include "socket_oneM2M.h"

socket_oneM2M::socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES)
{
	system_clock::time_point start = system_clock::now();
    send_oneM2M s(parsed_struct);

    string building_name = parsed_struct.building_name;
    string ACOR_NAME = parsed_struct.building_name;

    string originator_name = "C" + parsed_struct.device_name;
    string device_name = parsed_struct.device_name;

    try
    {
        std::cout << "Updating This Building ACP\n"  << std::endl;
        s.acp_update(parsed_struct, ACP_NAMES, 0);
        s.ae_create(parsed_struct);

        vector<string> Default_CNTs;
        vector<string> Default_PHYSICS_CNTs;
        vector<string> Default_INSIDE_CNTs;
        vector<string> Default_OUTSIDE_CNTs;

        Default_CNTs.emplace_back("Elevator_physics");
        Default_CNTs.emplace_back("Elevator_button_inside");
        Default_CNTs.emplace_back("Elevator_button_outside");

        Default_PHYSICS_CNTs.emplace_back("Velocity");
        Default_PHYSICS_CNTs.emplace_back("Altimeter");
        Default_PHYSICS_CNTs.emplace_back("Temperature");
        Default_PHYSICS_CNTs.emplace_back("Trip");
        Default_PHYSICS_CNTs.emplace_back("Distance");

        s.cnt_create(parsed_struct, 1, device_name);
        for(const string& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(parsed_struct, 2, device_name, CNT_NAME);
        }
        for(const string& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(int i = parsed_struct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
        }
        for(int i = 1 ; i<=parsed_struct.ground_floor ; i++)
        {
            s.cnt_create(parsed_struct, 3, device_name, Default_CNTs[2], std::to_string(i));
        }

		// Create or Update Velocity, Altimeter, Temperature
        string originator = "C" + parsed_struct.building_name;
        if(parsed_struct.velocity != -1)
        {
            string payload = std::to_string(parsed_struct.velocity);
            s.cin_create(originator, "velocity", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[0]);
        }
        if(parsed_struct.altimeter != -1)
        {
            string payload = std::to_string(parsed_struct.altimeter);
            s.cin_create(originator, "altimeter", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[1]);
        }
        if(parsed_struct.temperature != -1)
        {
            string payload = std::to_string(parsed_struct.temperature);
            s.cin_create(originator, "temperature", payload, 3, device_name, Default_CNTs[0], Default_PHYSICS_CNTs[2]);
        }
        if(!parsed_struct.button_inside.empty())
        {
            string payload = "";
            for(string temp : parsed_struct.button_inside)
            {
                payload += " " + temp;
            }
            s.cin_create(originator, "button_inside", payload, 2, device_name, Default_CNTs[1]);
        }

        std::chrono::duration<double> delta = system_clock::now() - start;

        std::cout << "Time Spend on DT_SERVER is : "  << delta.count() << " s"<< std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
    }
    //Check Each AE Exists based on DB, if no AE then make AE (in DEV)

    //Check Latest CIN timestamp based on latest log, if new CIN then get new CIN and calculate logic312262
}

socket_oneM2M::~socket_oneM2M()
{
}
