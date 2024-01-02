#include <cstdarg>
#include <unordered_set>
#include <cpprest/http_client.h>

#include "socket_oneM2M.h"

using namespace std;

socket_oneM2M::socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) : socket(parsed_struct)
{
	system_clock::time_point start = system_clock::now();

	ACOR_NAME = parsed_struct.building_name;
    building_name = parsed_struct.building_name;

    originator_name = "C" + parsed_struct.device_name;
    device_name = parsed_struct.device_name;

    try
    {
        this->socket_name = building_name;

        cout << "Updating This Building ACP\n"  << endl;
        socket.acp_update(parsed_struct, ACP_NAMES, 0);
        socket.ae_create(this->building_name, this->device_name);

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

void socket_oneM2M::check_cin_difference_between_previous_RETRIEVE(vector<vector<string>> previous, vector<vector<string>> current)
{
    vector<double> velocity_comparison;
    vector<double> altimeter_comparison;

    std::unordered_set<std::string> previous_set(previous[2].begin(), previous[2].end());
    std::unordered_set<std::string> current_set(current[2].begin(), current[2].end());

    //CHECK VELOCITY DIFFERENCE
    velocity_comparison.push_back(std::stod(previous[0][0]));
    velocity_comparison.push_back(std::stod(current[0][0]));

	//CHECK ALTIMETER DIFFERENCE
    altimeter_comparison.push_back(std::stod(previous[1][0]));
    altimeter_comparison.push_back(std::stod(current[1][0]));

	//CHECK BUTTON INSIDE DIFFERENCE

    // Find added elements
    for (const auto& element : current[2]) {
        if (previous_set.find(element) == previous_set.end()) {
            std::cout << "ADD FLOOR : " << element << std::endl;
        }
    }

    // Find deleted elements
    for (const auto& element : previous[2]) {
        if (current_set.find(element) == current_set.end()) {
            std::cout << "DELETED FLOOR : " << element << std::endl;
        }
    }

    //CHECK BUTTON OUTSIDE DIFFERENCE
}

bool socket_oneM2M::create_oneM2M_under_device_name(parse_json::parsed_struct parsed_struct)
{
    try
    {
        system_clock::time_point start = system_clock::now();

        send_oneM2M s = this->socket;

        string building_name = this->building_name;
		string originator_name = "C" + this->building_name;
        string device_name = this->device_name;

        Default_PHYSICS_CNTs.emplace_back(device_name +"_Velocity");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Altimeter");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Temperature");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Trip");
        Default_PHYSICS_CNTs.emplace_back(device_name +"_Distance");

        Default_INSIDE_CNTs.emplace_back(device_name +"_Button_List");
        Default_INSIDE_CNTs.emplace_back(device_name +"_GoTo");

        socket.cnt_create(originator_name, 1, device_name);

	    for(const string& CNT_NAME : Default_CNTs)
        {
            s.cnt_create(originator_name, 2, device_name, CNT_NAME);
        }

        for(const string& CNT_NAME : Default_PHYSICS_CNTs)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[0], CNT_NAME);
        }
        for(const string& CNT_NAME : Default_INSIDE_CNTs)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[1], CNT_NAME);
        }
        for(const string& CNT_NAME : Default_OUTSIDE_CNTs)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[2], CNT_NAME);
        }

        string payload = "None";
        for(int i = parsed_struct.underground_floor ; i>=1 ; i--)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
            s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
        }
        for(int i = 1 ; i<=parsed_struct.ground_floor ; i++)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[2], std::to_string(i));
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

	    string building_name = this->building_name;
		string originator_name = "C" + this->building_name;
	    string device_name = this->device_name;

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

vector<vector<string>> socket_oneM2M::retrieve_oneM2M_cins(vector<int> floor_info)
{
	try
	{
        system_clock::time_point start = system_clock::now();

	    send_oneM2M s = this->socket;

        vector<vector<string>> ret; // ret  = {velocity, altimeter, button_inside, button_outside}
        vector<string> ret_vel, ret_alt, ret_bin, ret_bout; // EACH CON STRING

	    string building_name = this->building_name;
		string originator_name = "C" + this->building_name;
	    string device_name = this->device_name;

        web::http::http_response res;
        web::json::value response_json;
        std::wstring con;
        std::string con_string;

        if(this->Default_CNTs.empty())
        {
	        std::cerr << "ERROR OCCURED ON socket_oneM2M :  Default_CNTs is Empty" << std::endl;
            exit(0);
        }
        else
        {
            Default_PHYSICS_CNTs.emplace_back(device_name +"_Velocity");
	        Default_PHYSICS_CNTs.emplace_back(device_name +"_Altimeter");
	        Default_PHYSICS_CNTs.emplace_back(device_name +"_Temperature");
	        Default_PHYSICS_CNTs.emplace_back(device_name +"_Trip");
	        Default_PHYSICS_CNTs.emplace_back(device_name +"_Distance");

	        Default_INSIDE_CNTs.emplace_back(device_name +"_Button_List");
	        Default_INSIDE_CNTs.emplace_back(device_name +"_GoTo"); //IN_DEV

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

            //DISASSEMBLE CON
            response_json = res.extract_json().get();
			con = response_json[U("m2m:cin")][U("con")].as_string();

            con_string.assign(con.begin(), con.end());
            //std::cout << "BUTTON INSIDE CON IS : " << con_string << std::endl;

            std::istringstream iss(con_string);
            string value;
            while(iss >> value)
            {
	            ret_bin.push_back(value);
            }

            for(int i = floor_info[0] ; i>=1 ; i--)
	        {
                string floor = "B"+std::to_string(i);
				res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[2], floor);

	            //DISASSEMBLE CON
	            response_json = res.extract_json().get();
				con = response_json[U("m2m:cin")][U("con")].as_string();

                if(con != L"None")
                {
	                con_string.assign(con.begin(), con.end());
                    string result_string = floor + ":" + con_string;

                    //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : " << result_string << std::endl;
					ret_bout.push_back(result_string);
                }

                //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : ";
	            //std::wcout << con << std::endl;
	        }

	        for(int i = 1 ; i<=floor_info[1] ; i++)
	        {
                string floor = std::to_string(i);
                res = s.cin_retrieve_la(originator_name, 3, device_name, this->Default_CNTs[2], floor);

	            //DISASSEMBLE CON
	            response_json = res.extract_json().get();
				con = response_json[U("m2m:cin")][U("con")].as_string();

                if(con != L"None")
                {
	                con_string.assign(con.begin(), con.end());
                    string result_string = floor + ":" + con_string;

                    //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : " << result_string;
					ret_bout.push_back(result_string);
                }

                //std::cout << "BUTTON OUTSIDE FLOOR : "<< floor << " CON IS : ";
	            //std::wcout << con << std::endl;
	        }

            ret.push_back(ret_vel);
            ret.push_back(ret_alt);
            ret.push_back(ret_bin);
            ret.push_back(ret_bout);

	        return ret;
        }
	}
	catch (const std::exception& e)
	{
        std::cerr << "Exception Caught : " << e.what() << std::endl;
	}
}