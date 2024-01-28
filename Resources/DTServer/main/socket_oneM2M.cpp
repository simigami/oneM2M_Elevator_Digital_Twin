#include <cstdarg>
#include <unordered_set>
#include <cpprest/http_client.h>

#include "socket_oneM2M.h"

using namespace std;

socket_oneM2M::socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) : socket(parsed_struct)
{
	system_clock::time_point start = system_clock::now();

    int flag;

	ACOR_NAME = parsed_struct.building_name;
    building_name = parsed_struct.building_name;

    originator_name = "C" + parsed_struct.device_name;
    originator_name = "C" + parsed_struct.building_name;
    device_name = parsed_struct.device_name;

    try
    {
        this->socket_name = building_name;

        //cout << "CHECK This Building ACP..."  << endl;
        flag = socket.acp_validate(building_name, 0);

        if(flag == 0)
        {
	        //ACP NOT EXISTS
            //cout << "ACP RESOURCE NOT EXISTS..."  << endl;
	        socket.acp_create_one_ACP(parsed_struct, ACP_NAMES, 0);
            socket.ae_create(this->building_name);
        }
        else if(flag == 1)
        {
	        //ACP EXISTS BUT BUILDING NAME NOT EXISTS
            //cout << "ACOR : " << "C" + this->building_name << " NOT EXISTS..."  << endl;
            socket.acp_update(parsed_struct, ACP_NAMES, 0);
            socket.ae_create(this->building_name);
        }
        else
        {
            //cout << "ELEVATOR : " << this->device_name << " NOT EXISTS..."  << endl;
        }
        //ACP and BUILDING NAME EXISTS BUT ELEVATOR NOT EXISTS
	    socket.cnt_create(originator_name, 1, this->device_name);

        Default_CNTs.emplace_back("Elevator_physics");
        Default_CNTs.emplace_back("Elevator_button_inside");
        Default_CNTs.emplace_back("Elevator_button_outside");

        auto res = create_oneM2M_under_device_name(parsed_struct);

        //DISCOVERY TESTING
        //auto start = system_clock::now();
        //socket.discovery_retrieve(originator_name, 0);
        //chrono::duration<double> interval = system_clock::now() - start;;
		//cout << device_name << " DISCOVERY TIME : " << interval.count()<< " seconds..." << endl;
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
			bool flag = true;
            for(const vector<int> each_floor : parsed_struct.button_outside)
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
            	s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
            }
        }

        for(int i = 1 ; i<=parsed_struct.ground_floor ; i++)
        {
            s.cnt_create(originator_name, 3, device_name, Default_CNTs[2], std::to_string(i));
            bool flag = true;
            for(const vector<int> each_floor : parsed_struct.button_outside)
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
        		s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], std::to_string(i));
            }
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
        //std::cout << "Time Spend on DT_SERVER is : "  << delta.count() << " s"<< std::endl;

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

        payload = "None";
        for(int i = parsed_struct.underground_floor ; i>=1 ; i--)
        {
            bool flag = true;
            for(const vector<int> each_floor : parsed_struct.button_outside)
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
            	s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], "B"+std::to_string(i));
            }
        }

        for(int i = 1 ; i<=parsed_struct.ground_floor ; i++)
        {
            bool flag = true;
            for(const vector<int> each_floor : parsed_struct.button_outside)
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
				s.cin_create(originator_name, "status", payload, 3, device_name, Default_CNTs[2], std::to_string(i));
            }
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
	    //std::cout << "Time Spend on DT_SERVER is : "  << delta.count() << " s"<< std::endl;

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
            response_json = res.extract_json().get();
            //CHECK IF BUTTON INSIDE IS EMPTY
            if(response_json.has_field(U("m2m:cin")) && response_json[U("m2m:cin")].has_field(U("con")))
            {
	            //DISASSEMBLE CON
				con = response_json[U("m2m:cin")][U("con")].as_string();

	            con_string.assign(con.begin(), con.end());
	            //std::cout << "BUTTON INSIDE CON IS : " << con_string << std::endl;

	            std::istringstream iss(con_string);
	            string value;
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