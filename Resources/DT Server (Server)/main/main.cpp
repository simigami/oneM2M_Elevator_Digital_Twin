#include "DT_RealTime.h"
#include "DT_Simulation.h"
#include "DT_Plot.h"
#include "DT_Labeling.h"
#include "config.h"
#include <csignal>
#include <boost/asio.hpp>
#include "main.h"

#define WIN32_LEAN_AND_MEAN

void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received.\n";
	exit(signum);
}

int main()
{
	// check log dir exists and make one if not
	fs::path logDir(LOG_DIR_NAME);
	if (!fs::exists(logDir))
	{
		fs::create_directory(logDir);

		// make 4 directories for log files
		fs::create_directory(logDir / RTS_LOG_DIR_NAME);
		fs::create_directory(logDir / SIMULATION_LOG_DIR_NAME);
		fs::create_directory(logDir / STATECODE_LOG_DIR_NAME);
		fs::create_directory(logDir / PLOT_LOG_DIR_NAME);
	}

	try
	{
		signal(SIGINT, signalHandler);

		int input;
		std::cout << "Enter the number below for your specific task" << std::endl;
		std::cout << "1 : Run Digital Twin Server in RealTime" << std::endl;
		std::cout << "2 : Run Simulation Based on Log File" << std::endl;
		std::cout << "3 : Run Plotting Based on Log File" << std::endl;
		std::cout << "4 : Run Labeling Based on Csv File" << std::endl;
		std::cin >> input;

		while (input <= 0 && 5<= input)
		{
			std::cout << "Invalid Input" << std::endl;
			std::cout << "Enter the number below for your specific task" << std::endl;
			std::cout << "1 : Run Digital Twin Server in RealTime" << std::endl;
			std::cout << "2 : Run Simulation Based on Log File" << std::endl;
			std::cout << "3 : Run Plotting Based on Log File" << std::endl;
			std::cout << "4 : Run Labeling Based on Csv File" << std::endl;
			std::cin >> input;
		}

		if (input == 1)
		{
			std::remove(LOGFILEPATH);
			std::wofstream logFile(LOGFILEPATH, ios::app);
			logFile.close();

			// IN ORDER TO OPERATE RTS, IT NEED oneM2M CSE SERVER to be running
			// CHECK CSE ADDR AND PORT IN CONFIG.H
			boost::asio::io_context io_context;
			boost::asio::ip::tcp::socket socket(io_context);
			boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(CSE_ADDRS), CSE_LISTEN_PORT);

			bool flag;

			try {
				socket.connect(endpoint);
				flag =  true;
			}
			catch (boost::system::system_error& e) {
				std::wcerr << e.what() << std::endl;
				flag =  false; 
			}
			socket.close();

			if (!flag)
			{
				std::wcerr << "CSE SERVER IS NOT RUNNING PLEASE CHECK CSE SERVER FOR RTS MODE" << std::endl;
				return 0;
			}

			int visualizer = 0;
			std::cout << "CSE IS RUNNING, PRESS 1 FOR VISUALIZER, 0 FOR NO VISUALIZER : ";
			std::cin >> visualizer;

			while (visualizer != 0 && visualizer != 1)
			{
				std::cout << "Invalid Input" << std::endl;
				std::cout << "PRESS 1 FOR VISUALIZER, 0 FOR NO VISUALIZER : ";
				
				std::cin >> visualizer;
			}

			// if visualizer is enabled, check visualizer server is running
			if (visualizer == 1)
			{
				endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(UE5_SERVER_ADDR), UE5_SERVER_LISTEN_PORT);

				try 
				{
					socket.connect(endpoint);
					flag = true;
				}
				catch (boost::system::system_error& e) {
					std::wcerr << e.what() << std::endl;
					flag = false;
				}
				socket.close();

				if (!flag)
				{
					std::wcerr << "VISUALIZER SERVER IS NOT AUTOMATICALLY CHANGE TO non-visualization mode" << std::endl;
					visualizer = 0;
				}
			}

			dt_real_time dt_realtime_instance;
			dt_realtime_instance.VisualizeMod = visualizer;
			dt_realtime_instance.Run();
		}
		else if (input == 2)
		{
			dt_simulation dt_simulation_instance;
			dt_simulation_instance.run();
		}
		else if (input == 3)
		{
			dt_plot dt_plot_instance;
			dt_plot_instance.run();
		}
		else if (input == 4)
		{
			DTLabeling Instance;
			Instance.run();
		}

		main();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}