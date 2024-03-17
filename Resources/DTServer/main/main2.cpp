#include "main2.h"
#include "DT_RealTime.h"
#include "DT_Simulation.h"

int main()
{
	try
	{
		std::string input;
		std::cout << "PRESS 1 FOR DT Server RealTime, 2 FOR SIMULATION : ";
		std::cin >> input;

		if (input == "1")
		{
			dt_real_time dt_realtime_instance;
			dt_realtime_instance.Run();
		}
		else if (input == "2")
		{
			dt_simulation dt_simulation_instance;
			dt_simulation_instance.run();
		}
		else
		{
			std::cout << "Invalid Input" << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}