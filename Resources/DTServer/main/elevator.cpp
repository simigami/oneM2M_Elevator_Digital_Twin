#include "elevator.h"

Elevator::Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) :
sock(parsed_struct, ACP_NAMES),
p(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    })
{
	this->isRunning = true;
	this->RETRIEVE_interval_second = 1;

	this->Elevator_opcode = 0;
	this->go_To_Floor = 0;
	this->building_name = parsed_struct.building_name;
	this->device_name = parsed_struct.device_name;

	this->floor_info.push_back(parsed_struct.underground_floor);
	this->floor_info.push_back(parsed_struct.ground_floor);

	cout << "CREATED ELEVATOR CLASS BUILDING NAME : " << building_name << " DEVICE : " << device_name << endl;
  }

Elevator::~Elevator()
{
	stop_thread();
}

void Elevator::start_thread()
{
	thread temp(&Elevator::run, this);
	temp.detach();
}

void Elevator::stop_thread()
{
}

void Elevator::run()
{
	while(true)
	{
		this->RETRIEVE_from_oneM2M();
		std::this_thread::sleep_for(std::chrono::seconds(this->RETRIEVE_interval_second));
	}
}

void Elevator::RETRIEVE_from_oneM2M()
{
	//RETRIEVE from oneM2M Resource Tree Based on its building_name and device_name
	vector<vector<long double>> temp2;

	vector<vector<string>> ret;
	vector<string> ret_bin;

    system_clock::time_point start = system_clock::now();

	socket_oneM2M s = this->sock;
	physics p = this->p;
	simulation sim = p.s;
	vector<int> main_trip_list = sim.main_trip_list;
	vector<vector<int>> outside_button_list;

	//ret = s.retrieve_oneM2M_cins(e.floor_info);
	ret = s.retrieve_oneM2M_cins(this->floor_info);

	//SAVE LATEST RETRIEVE INFO if info vector is EMPTY
	if(this->latest_RETRIEVE_info.empty())
	{
		cout << this->building_name << " -> " << this->device_name  << " : latest RETRIEVE info is EMPTY. SAVING..." << endl;
		this->latest_RETRIEVE_info = ret;

		//IF BUTTON OUTSIDE DATA EXISTS
		// * BUTTON OUTSIDE DATA(REQUEST FROM LOBBY) IS PRIOR TO INSIDE DATA : ALG 1.
		if(!this->latest_RETRIEVE_info[3].empty())
		{
			//VERY FIRST REQUEST FROM DATA -> DEFINE ELEVATOR DIRECTION
			for(const string& elem : this->latest_RETRIEVE_info[3])
			{
				stringstream ss(elem);
				string floor, direction;
				int floor_int, direction_int;

				getline(ss, floor, ':');
				getline(ss >> ws, direction);

				floor_int = sim.string_floor_to_int(floor);
				direction_int = direction._Equal("Up") ? 1 : 0;

				cout << "FLOOR : " << floor << " DIRECTION : " << direction;
				outside_button_list.push_back({floor_int, direction_int});
			}
			//SET INITIAL DIRECTION
			if(p.current_direction == NULL)
			{
				p.set_initial_elevator_direction(outside_button_list);
			}
			sim.update_main_trip_list_via_outside_data(outside_button_list, p.current_direction);
		}
		//IF BUTTON INSIDE DATA EXISTS
		if(!this->latest_RETRIEVE_info[2].empty())
		{
			if(p.current_direction == NULL)
			{
				p.set_initial_elevator_direction(this->latest_RETRIEVE_info[2]);
			}
			sim.update_main_trip_list_via_inside_data(this->latest_RETRIEVE_info[2], p.current_direction);
		}

		cout << "MAIN TRIP LIST : ";
		for(const auto& elem : sim.main_trip_list)
		{
			cout << elem << " ";
		}
		cout << endl;
	}

	//COMPARE LATEST RETRIEVE INFO if info vector has value
	else
	{
		//CHECK the difference
		s.check_cin_difference_between_previous_RETRIEVE(this->latest_RETRIEVE_info, ret);

		this->latest_RETRIEVE_info = ret;
	}
	std::chrono::duration<double> delta = system_clock::now() - start;
}

void Elevator::dev_print()
{
	cout << "ELEVATOR : " << this << " DEV PRINT" << endl;
	cout << "SOCK : " << this->sock.socket_name << endl;
	cout << "PHYSICS : " << this->p.current_velocity << endl;
	cout << "BUILDING NAME : " << this->building_name << endl;
	cout << "DEVICE NAME : " << this->device_name << endl;
}

/*
 * 			string first_goTo_floor = this->latest_RETRIEVE_info[2].front();
			int goTo_floor_index = this->p.s.string_floor_to_int(first_goTo_floor);

			//DRAW V_T TABLE FOR FIRST GOTO FLOOR
			this->current_goTo_Floor_vector_info = this->p.draw_vt_on_sigle_floor(goTo_floor_index);
 */