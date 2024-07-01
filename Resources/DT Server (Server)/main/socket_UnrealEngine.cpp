#include <boost/asio.hpp>
#include <iostream>

#include "config.h"
#include "socket_UnrealEngine.h"

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

socket_UnrealEngine::socket_UnrealEngine() : context(), UE5AcceptSocket(context)
{
}

void socket_UnrealEngine::send_data_to_UE5(std::string request_content)
{
#ifdef UE5_SERVER_ADDR
	try 
	{
		boost::asio::ip::tcp::resolver resolver(context);

		// Create a resolver to resolve the server address
		std::string server_ip = UE5_SERVER_ADDR;
		std::string server_port = std::to_string(UE5_SERVER_LISTEN_PORT);
		boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(boost::asio::ip::tcp::v4(), server_ip, server_port);

		// Create a socket and connect to the server
		boost::asio::connect(UE5AcceptSocket, endpoints);

		boost::asio::write(UE5AcceptSocket, boost::asio::buffer(request_content + "\n")); // "\n"을 추가하여 메시지 종료를 명시
        //boost::asio::async_write(socket, boost::asio::buffer(json), handle_write);

		// RESPONSE 잘 받지 못하는 경우에 생김
		boost::asio::streambuf buffer;
        boost::asio::read_until(UE5AcceptSocket, buffer, '\n');

        // Extract response content
        std::istream response_stream(&buffer);
        std::string response_content;
        std::getline(response_stream, response_content);

        // Check response content and exit function if successful
        if (response_content == "BuildingDataNULL") 
		{
        	std::cout << "FROM UE5 TO SERVER -> Error : Building Data NULL" << std::endl;
            return;
        }
		if (response_content == "Received") 
		{
        	std::cout << "FROM UE5 TO SERVER -> DATA SEND AND RECEIVED COMPLETE" << std::endl;
            return;
        }

    }
	catch (const boost::system::system_error& e) 
	{
		std::cerr <<  "FROM SERVER TO UE5 -> ERROR : " <<  e.what() << std::endl;
    }
#endif
}