#pragma once
#include <boost/asio.hpp>
#include <cpprest/http_client.h>

#include "elevator.h"
#include "parse_json.h"
#include "socket_oneM2M.h"

#define host_protocol "http://"
#define host_ip "192.168.0.178"
#define dt_server_ip "192.168.0.134"
#define host_port "10051"
#define notification_port "10053"

#define DEFAULT_ACP_NAME "DT_SERVER"
#define DEFAULT_ORIGINATOR "CAdmin"
#define DEFAULT_CSE_NAME "tinyIoT"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

struct class_of_one_Building
{
	vector<Elevator*> classOfAllElevators;
	string ACP_NAME;
};

class DTServer
{
public:
	DTServer();

	void Run();

	void startAsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void startAsyncAccept2(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void handleConnection(boost::asio::ip::tcp::socket& socket, int port);
	void Running_Embedded(const std::string& httpResponse);
	void Running_Notification(const std::string& httpResponse);

	bool existsElevator(class_of_one_Building one_building, string device_name);

	class_of_one_Building* get_building_vector(string ACOR_NAME);
	Elevator* getElevator(class_of_one_Building* class_of_one_building, string device_name);
	socket_oneM2M get_oneM2M_socket_based_on_AE_ID(vector<Elevator*> elevator_array, const string AE_ID);

	//MAIN 함수에서 알고리즘에 전달할 notificationContent 구조체
    notificationContent* tempContentMoveToAlgorithm;

	vector<class_of_one_Building*> class_of_all_Buildings; //THIS WILL BE USED
	vector<thread> one_building_threads;
	vector<string> ACP_NAMES;

	parse_json::parsed_struct parsed_struct;
	send_oneM2M ACP_Validation_Socket;
	boost::asio::io_service io;

	std::string* httpRequest;

private:


};