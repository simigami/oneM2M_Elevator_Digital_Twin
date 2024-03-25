#pragma once
#include <boost/asio.hpp>
#include <cpprest/http_client.h>

#include "elevator.h"
#include "parse_json.h"
#include "socket_oneM2M.h"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

struct class_of_one_Building
{
	int outsideButtonMod = 0;

	vector<Elevator*> classOfAllElevators;
	map<wstring, wstring> subscriptionRI_to_RN;
	wstring ACP_NAME = L"";
};

class Building
{
public:
	Building(int buttonMod);

	class_of_one_Building* buildingElevatorInfo;
	vector<vector<int>>* currentButtonOutsideInfos;

	int getButtonMod();
	void getWhichElevatorToGetButtonOutside(vector<vector<int>> button_outside);

	vector<vector<int>> getCurrentElevatorsButtonOutsideLists();
	vector<vector<int>> getDiffBetweenCurrentAndEmbedded(vector<vector<int>> newList, vector<vector<int>> oldList);
};

class dt_real_time
{
public:
	dt_real_time();

	void Run();

	void startAsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void startAsyncAccept2(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void handleConnection(boost::asio::ip::tcp::socket& socket, int port);
	void Running_Embedded(const wstring& httpResponse);
	void Running_Embedded2(const wstring& httpResponse);
	void Running_Notification(const string& httpResponse);

	bool existsElevator(Building* one_building, const wstring& device_name);

	string get_RN_Path_From_SubscriptionRI(const string& substring);

	Building* get_building_vector(const wstring& ACOR_NAME);
	Elevator* getElevator(Building* class_of_one_building, const wstring& device_name);
	socket_oneM2M get_oneM2M_socket_based_on_AE_ID(vector<Elevator*> elevator_array, const wstring& AE_ID);

	//MAIN 함수에서 알고리즘에 전달할 notificationContent 구조체
    notificationContent* noti_content;

	vector<Building*> allBuildingInfo; //THIS WILL BE USED
	vector<thread> one_building_threads;
	vector<wstring> ACP_NAMES;

	Wparsed_struct parsed_struct;
	boost::asio::io_service io;

	std::wstring* httpRequest;

private:


};