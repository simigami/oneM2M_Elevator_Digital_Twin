#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/bind/bind.hpp>
#include <boost/serialization/serialization.hpp>
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
	/*
	 *
	 *	//MAKE ASIO SERVICE
	boost::asio::io_service io_service;

	//MAKE ASIO SOCKET
	boost::asio::ip::tcp::socket sock(io_service);

	//MAKE ENDPOINT and ERROR HANDLER FOR ASIO SOCKET
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(UE5_IP), UE5_PORT);
	boost::system::error_code ec;

	//SERIALIZE UE_Info Structure to oarchive
	std::string json = this->sock.struct_to_json(info);

	//CONNECT Socket and Send Data
	sock.connect(ep, ec);
	boost::asio::async_write(sock, boost::asio::buffer(json), &on_send_completed);

	io_service.run();
	 */

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

        boost::asio::async_write(socket, boost::asio::buffer(json), handle_write);

		io_context.run();
    }
	catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

#endif
}

void socket_UnrealEngine::set_goTo_Floor(int floor)
{
	this->sock.UE_info.goToFloor = floor;
}

void socket_UnrealEngine::accept_con()
{
}