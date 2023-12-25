#include <iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define _WIN32_WINNT_WIN10 0x0A00

using namespace boost::asio;
using ip::tcp;
using std::string;

void print_elapsed_time()
{
	int elapsed = 0;
    while(true)
    {
	    std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;
        std::cout << "Elapsed Time " << elapsed << "seconds \n";
    }
}

int main()
{
    try
    {
    	boost::asio::io_service io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 10050));
        std::thread timerThread(print_elapsed_time);

        while(true)
        {
	        tcp::socket socket(io);
            acceptor.accept(socket);

            boost::asio::streambuf buf;
            boost::asio::read_until(socket, buf, "\n");

            std::istream input_stream(&buf);
            string json_data;
            std::getline(input_stream, json_data);

            std::cout << "Received JSON data" << json_data << std::endl;

        }

    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}