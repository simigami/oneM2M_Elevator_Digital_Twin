#include "DT_RealTime.h"
#include "DT_Simulation.h"
#include <csignal>
#include <boost/asio.hpp>

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

		while (input != "1" && input != "2")
		{
			std::cout << "Invalid Input" << std::endl;
			std::cout << "PRESS 1 FOR DT Server RealTime, 2 FOR SIMULATION : ";
			std::cin >> input;
		}

		if (input == "1")
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
		else if (input == "2")
		{
			dt_simulation dt_simulation_instance;
			dt_simulation_instance.run();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}