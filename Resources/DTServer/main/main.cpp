#include "DT_RealTime.h"
#include "DT_Simulation.h"
#include <csignal>

#define WIN32_LEAN_AND_MEAN

void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received.\n";
	exit(signum);
}

int main()
{
	try
	{
		signal(SIGINT, signalHandler);

		std::string input;
		std::cout << "PRESS 1 FOR DT Server RealTime, 2 FOR SIMULATION : ";
		std::cin >> input;

		if (input == "1")
		{
			std::remove(LOGFILEPATH);
			std::wofstream logFile(LOGFILEPATH, ios::app);
			logFile.close();

			int visualizer = 0;
			std::cout << "PRESS 1 FOR VISUALIZER, 0 FOR NO VISUALIZER : ";
			std::cin >> visualizer;

			while (visualizer != 0 && visualizer != 1)
			{
				std::cout << "Invalid Input" << std::endl;
				std::cout << "PRESS 1 FOR VISUALIZER, 0 FOR NO VISUALIZER : ";
				
				std::cin >> visualizer;
			}

			dt_real_time dt_realtime_instance;
			dt_realtime_instance.VisualizeMod = visualizer;
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