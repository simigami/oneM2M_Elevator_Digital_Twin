#pragma once
#include <boost/asio.hpp>
#include "send_UnrealEngine.h"

class socket_UnrealEngine
{
public:
	socket_UnrealEngine();
	void send_data_to_UE5(std::string request_content);

private:
	boost::asio::io_context context;
	boost::asio::ip::tcp::socket UE5AcceptSocket;
	bool isAcceptSocket = false;
};
