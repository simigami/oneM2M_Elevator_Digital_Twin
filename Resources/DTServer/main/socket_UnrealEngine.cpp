#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/bind/bind.hpp>
#include "socket_UnrealEngine.h"
#include "send_UnrealEngine.h"

#include <iostream>

void handle_write(const boost::system::error_code& error, std::size_t bytes_transferred) {
	if (!error) 
	{
        std::cout << "Data sent successfully\n";
    }
	else 
	{
        std::cerr << "Error: " << error.message() << std::endl;
    }
}

socket_UnrealEngine::socket_UnrealEngine(string building_name, string device_name, int underground_floor, int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity)
{
	this->sock.UE_info.building_name = building_name;
	this->sock.UE_info.device_name = device_name;

	this->sock.UE_info.underground_floor = underground_floor;
	this->sock.UE_info.ground_floor = ground_floor;

	this->sock.UE_info.each_floor_altimeter = each_floor_altimeter;

	this->sock.UE_info.acceleration = acceleration;
	this->sock.UE_info.max_velocity = max_velocity;

	this->sock.UE_info.each_floor_altimeter = set_sock_altimeter_offsets();

	this->sock.UE_info.tta = 0;
	this->sock.UE_info.ttm = 0;
	this->sock.UE_info.ttd = 0;
}

vector<double> socket_UnrealEngine::set_sock_altimeter_offsets()
{
	double lowest_altimeter = abs(this->sock.UE_info.each_floor_altimeter[0]);

	if(this->sock.UE_info.each_floor_altimeter[0] >0)
	{
		for(auto& elem : this->sock.UE_info.each_floor_altimeter)
		{
			elem -= lowest_altimeter;
		}
	}
	else
	{
		for(auto& elem : this->sock.UE_info.each_floor_altimeter)
		{
			elem += lowest_altimeter;
		}
	}

	return this->sock.UE_info.each_floor_altimeter;
}

void socket_UnrealEngine::send_data_to_UE5(const UE_Info& info)
{
#ifdef UE5_IP
	try {
        // Server address and port
        const std::string SERVER_ADDRESS = "127.0.0.1";
        const std::string SERVER_PORT = "10052";

        // Initialize Boost.Asio
        boost::asio::io_context io_context;

        // Create a resolver to resolve the server address
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(SERVER_ADDRESS, SERVER_PORT);

        // Create a socket and connect to the server
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send the string to the server
		std::string json = this->sock.struct_to_json(info);

		boost::asio::write(socket, boost::asio::buffer(json + "\n")); // "\n"을 추가하여 메시지 종료를 명시
        //boost::asio::async_write(socket, boost::asio::buffer(json), handle_write);

		// RESPONSE 잘 받지 못하는 경우에 생김
		boost::asio::streambuf response;
        boost::asio::read_until(socket, response, '\n');

        // Extract response content
        std::istream response_stream(&response);
        std::string response_content;
        std::getline(response_stream, response_content);

        // Check response content and exit function if successful
        if (response_content == "Data Received") 
		{
            // Data received successfully, exit function
			cout << "DATA SEND AND RECEIVED COMPLETE FROM UE5" << endl;
            return;
        }
		else 
		{
            // Handle response error (e.g., print error message)
            std::cerr << "Error: Invalid response from UE5 server." << endl;
        }

		//io_context.run();
    }
	catch (std::exception& e) 
	{
		std::cerr << endl << "FROM SERVER TO UE5 -> ERROR : " << endl << e.what() << endl;
    }
#endif
}

void socket_UnrealEngine::set_goTo_Floor(int floor, double tta, double ttm, double ttd)
{
	this->sock.UE_info.goToFloor = floor;

	this->sock.UE_info.tta = tta;
	this->sock.UE_info.ttm = ttm;
	this->sock.UE_info.ttd = ttd;
}

void socket_UnrealEngine::accept_con()
{
}

UE_Info socket_UnrealEngine::wrap_for_UE_socket(string building_name, string device_name, int underground_floor,
	int ground_floor, vector<double> each_floor_altimeter, double acceleration, double max_velocity)
{
	UE_Info temp;
	temp.building_name = building_name;
	temp.device_name = device_name;
	temp.underground_floor = underground_floor;
	temp.ground_floor = ground_floor;

	if(each_floor_altimeter[0] < 0)
	{
		for(auto elem : each_floor_altimeter)
		{
			temp.each_floor_altimeter.push_back(elem+abs(each_floor_altimeter[0]));
		}
	}
	else if(each_floor_altimeter[0] > 0)
	{
		for(auto elem : each_floor_altimeter)
		{
			temp.each_floor_altimeter.push_back(elem-abs(each_floor_altimeter[0]));
		}
	}
	temp.acceleration = acceleration;
	temp.max_velocity = max_velocity;

	return temp;
}
