#include <iostream>
#include <boost/asio.hpp>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
using boost::asio::ip::tcp;
using namespace web;
using namespace web::json;

void startListening(const std::string& address, unsigned int port) {
    try {
        boost::asio::io_service io_service;

        // Create an endpoint
        tcp::endpoint endpoint(boost::asio::ip::make_address(address), port);

        // Create an acceptor to listen for new connections
        tcp::acceptor acceptor(io_service, endpoint);

        // Start accepting connections
        std::cout << "Listening on " << address << ":" << port << std::endl;
        while (true) {
            tcp::socket socket(io_service);
            acceptor.accept(socket);

            // Read request
            boost::asio::streambuf request;
            boost::asio::read_until(socket, request, "\"}}");

            // Print request
            std::istream request_stream(&request);
            std::string header;
            while (std::getline(request_stream, header) && header != "\r") {
                std::cout << header << std::endl;
            }
            std::cout << std::endl;

            // Read and print body
            if (request.size() > 0) {
                std::string body(boost::asio::buffers_begin(request.data()), boost::asio::buffers_end(request.data()));
                std::cout << "Body:" << std::endl;
                std::cout << body << std::endl;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

/*
 *
 *vector<string> parseAndModifyJson(const web::http::http_response response) {

    // Modify the value of "sur" to "/"
	wstring sur;
    string sur_string;
    value response_json = response.extract_json().get();

    sur = response_json[U("m2m:sgn")][U("nev")][U("sur")].as_string();
    sur_string.assign(sur.begin(), sur.end());

    // Extract values except the last one into a vector<string>
    vector<string> sur_values;
    string delimiter = "/";
    size_t pos = 0;

    while ((pos = sur_string.find(delimiter)) != string::npos) 
    {
        sur_values.push_back(sur_string.substr(0, pos));
        sur.erase(0, pos + delimiter.length());
    }

    string last_index_value = sur_values[sur_values.size()-1];
    string second_last_index_value = sur_values[sur_values.size()-2];

    if(second_last_index_value == "Elevator_physics")
    {
	    if(last_index_value == "Velocity")
	    {
		    //THIS NOTIFICATION IS ABOUT VELOCITY CHANGE
            double con = response_json[U("m2m:sgn")][U("nev")][U("rep")][U("con")].as_double();
	    }
    	else if(last_index_value == "Altimeter")
        {
		    //THIS NOTIFICATION IS ABOUT ALTIMETER CHANGE
            double con = response_json[U("m2m:sgn")][U("nev")][U("rep")][U("con")].as_double();
	    }
    }

    else if(second_last_index_value == "Elevator_button_inside")
    {
	    if(last_index_value == "Button_List")
	    {
		    //THIS NOTIFICATION IS ABOUT BUTTON INSIDE LIST CHANGE
            json::array con = response_json[U("m2m:sgn")][U("nev")][U("rep")][U("con")].as_array();
            vector<int> button_inside_array;

            for(const auto& v : con)
            {
	            button_inside_array.push_back(v.as_integer());
            }
	    }
        else if(last_index_value == "goTo")
        {
	        int con = response_json[U("m2m:sgn")][U("nev")][U("rep")][U("con")].as_integer();
        }
    }

    else if(second_last_index_value == "Elevator_button_outside")
    {
	    wstring con = response_json[U("m2m:sgn")][U("nev")][U("rep")][U("con")].as_string();
        int called_floor;
        bool direction;

        if(con == L"Up")
        {
	        direction = true;
        }
        else
        {
	        direction = false;
        }

        if(!last_index_value.empty() && last_index_value[0] == 'B')
        {
	        called_floor = (last_index_value[last_index_value.size()-1] - '0') * -1;
        }
        else
        {
	        called_floor = stoi(last_index_value);
        }
    }

    return sur_values;
}
 */
void temp(boost::asio::ip::tcp::acceptor acceptor, boost::asio::ip::tcp::socket socket)
{
	cout << "AAAA";
}

vector<vector<double>> getElevatorTrajectory(double current_altimeter, double current_velocity, double dest_altimeter, double acceleration, double maximum_velocity, bool direction)
{
	// 0.1초 단위 시간 간격
	const double dt = 0.1;

	// 엘리베이터가 정지 상태인지 확인
	bool isStationary = (fabs(current_velocity) < 1e-6);

    // 현재 속도에서 최대 속도까지 가속할 때 가는 거리
    double distance_acceleration = ((maximum_velocity*maximum_velocity)-(current_velocity*current_velocity))/(2*acceleration);

    // maximum_velocity에서 0m/s까지 감속할 때 가는 거리
    double distance_deceleration = 0.5 * maximum_velocity * maximum_velocity / acceleration;

	// 목표 고도까지의 거리 계산
	double distance = fabs(dest_altimeter - current_altimeter);

	// 최대 속도 도달 여부 판단
	bool reachesMaxVelocity = distance > (distance_acceleration + distance_deceleration);

	vector<vector<double>> trajectory;

    int multiplyFactor = 0;

    if(direction)
    {
	    multiplyFactor = 1;
    }
    else
    {
	    multiplyFactor = -1;
    }

	if (isStationary) {

        //엘리베이터가 정지한 경우
	    if (reachesMaxVelocity) {
	    	// 최대 속도 도달 시 3단계 운동
            double distance_max_velocity = distance - distance_acceleration - distance_deceleration;
			double timeToAccVelocity = (maximum_velocity - current_velocity) / acceleration;
			double timeAtMaxVelocity = distance_max_velocity / maximum_velocity;
			double timeToDccVelocity = maximum_velocity / acceleration;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

            // 1. 최대 속도까지 가속
	    	for (double t = 0; t < timeToAccVelocity; t)
            {
                t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));
                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
	    	}

            // 2. 최대 속도 유지
			for (double t = 0; t < timeAtMaxVelocity - dt; t)
            {
				t += dt;
				temp_altimeter += multiplyFactor * maximum_velocity * dt;
                trajectory.push_back({t + timeToAccVelocity, maximum_velocity, temp_altimeter});

            }

            // 3. 최대 속도부터 0으로 감속
			double timeToDecelerate = maximum_velocity / acceleration;
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, maximum_velocity - t * acceleration, temp_altimeter});
                }
            }
        }

        else
        {
	        // 1. 목표 속력까지 가속
			double timeToAccelerate = floor(sqrt(distance/acceleration) * 1000) / 1000.0f;
			double timeToDecelerate = timeToAccelerate;
            double reachableVelocity = acceleration * timeToAccelerate;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

			for (double t = 0; t <= timeToAccelerate; t) 
            {
				t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));

                if(timeToAccelerate < t && t < timeToAccelerate + dt)
                {
                    temp_altimeter += multiplyFactor * 2 * ((0.5 * acceleration * timeToAccelerate * timeToAccelerate) - previous_temp);
                    temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;
	                trajectory.push_back({t, reachableVelocity, temp_altimeter});
                }
                else
                {
	                temp_altimeter += multiplyFactor * (0.5 * acceleration * t * t) - previous_temp;
	                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
                }
            }

			// 2. 목표 속도부터 0으로 감속
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccelerate, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccelerate, reachableVelocity - t * acceleration, temp_altimeter});
                }
            }
        }
	}
    else
    {
	    if (reachesMaxVelocity) {
	    	// 최대 속도 도달 시 3단계 운동
            double distance_max_velocity = distance - distance_acceleration - distance_deceleration;
			double timeToAccVelocity = (maximum_velocity - current_velocity) / acceleration <= 0 ?  0 : (maximum_velocity - current_velocity) / acceleration;
			double timeAtMaxVelocity = distance_max_velocity == dt ? 0 : distance_max_velocity / maximum_velocity;
			double timeToDccVelocity = maximum_velocity / acceleration;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, current_velocity, current_altimeter});

            // 1. 최대 속도까지 가속
	    	for (double t = 0; t < timeToAccVelocity; t)
            {
                t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));
                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                trajectory.push_back({t, current_velocity + t * acceleration <= maximum_velocity ? current_velocity + t * acceleration : maximum_velocity, temp_altimeter});
	    	}

            // 2. 최대 속도 유지
			for (double t = 0; t < timeAtMaxVelocity - dt; t)
            {
				t += dt;
				temp_altimeter += multiplyFactor * maximum_velocity * dt;
                trajectory.push_back({t + timeToAccVelocity, maximum_velocity, temp_altimeter});
            }

            // 3. 최대 속도부터 0으로 감속
			double timeToDecelerate = maximum_velocity / acceleration;
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDccVelocity-t) * (timeToDccVelocity-t)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 1000) / 1000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccVelocity + timeAtMaxVelocity, maximum_velocity - t * acceleration, temp_altimeter});
                }
            }
        }

        else
        {
            double DistancezeroToCurrentVelocity = 0.5 * (current_velocity/acceleration) * (current_velocity/acceleration) * acceleration;
        	// 1. 목표 속력까지 가속
			double timeToAccelerate = floor(sqrt((distance+DistancezeroToCurrentVelocity)/acceleration) * 1000) / 1000.0f;
			double timeToDecelerate = timeToAccelerate;

            timeToAccelerate -= (current_velocity/acceleration);
            timeToAccelerate = floor(timeToAccelerate * 1000) / 1000.0f;

            double reachableVelocity = acceleration * timeToAccelerate;

            double temp_altimeter = current_altimeter;
            double previous_temp;

            trajectory.push_back({0.0, 0.0, current_altimeter});

			for (double t = 0; t <= timeToAccelerate; t) 
            {
				t += dt;
                previous_temp = (0.5 * acceleration * (t-dt) * (t-dt));

                if(timeToAccelerate < t && t < timeToAccelerate + dt)
                {
                    temp_altimeter += multiplyFactor * (2 * ((0.5 * acceleration * timeToAccelerate * timeToAccelerate) - previous_temp));
                    temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;
	                trajectory.push_back({t, reachableVelocity, temp_altimeter});
                }
                else
                {
	                temp_altimeter += multiplyFactor * ((0.5 * acceleration * t * t) - previous_temp);
	                trajectory.push_back({t, current_velocity + t * acceleration, temp_altimeter});
                }
            }

			// 2. 목표 속도부터 0으로 감속
			for (double t = 0; t <= timeToDecelerate - dt; t) 
            {
                previous_temp = (0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt));
				t += dt;
                temp_altimeter += multiplyFactor * fabs((0.5 * acceleration * (timeToDecelerate-t-dt) * (timeToDecelerate-t-dt)) - previous_temp);
                temp_altimeter = floor(temp_altimeter * 10000) / 10000.0f;

                if(timeToDecelerate - dt < t)
                {
	                trajectory.push_back({t + timeToAccelerate, 0.0, dest_altimeter});
                }
                else
                {
	                trajectory.push_back({t + timeToAccelerate, reachableVelocity - t * acceleration, temp_altimeter});
                }
            }
        }
    }
	return trajectory;
}

int main()
{
    vector<vector<double>> t = getElevatorTrajectory(-52, 0.125, -55, 1.25, 2.5, false);

    for(auto elem : t)
    {
	    cout << " T " << elem[0] << " V " << elem[1] << " A " << elem[2] << endl;
    }
}