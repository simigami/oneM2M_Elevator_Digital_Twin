#include <cstdarg>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include "nlohmann/json.hpp"

#include "send_oneM2M.h"
#include "main.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

extern int cin_numbering;

send_oneM2M::send_oneM2M()
{
	string temp = host_protocol;
    temp += host_ip;
	temp += ":";
	temp += host_port;
    temp += "/";
    temp += "tinyIoT";
    URL_TO_CSE = temp;
}

send_oneM2M::send_oneM2M(parse_json::parsed_struct parsed_struct)
{
    string buliding_name = parsed_struct.building_name;
    string device_name = parsed_struct.device_name;

	string temp = host_protocol;
    temp += host_ip;
	temp += ":";
	temp += host_port;
    temp += "/";
    temp += "tinyIoT";
    URL_TO_CSE = temp;

    if(!device_name.empty())
    {
	    URL_TO_AE = temp + "/" + buliding_name;
    }
    else
    {
	    std::cout << "device name null error\n"  << std::endl;
        exit(0);
    }
}

void send_oneM2M::acp_create(int num, ...)
{
    string ACP_URL = this->URL_TO_CSE;
    string rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        ACP_URL += "/";
	        ACP_URL += ret;
        }
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::POST) ;

    std::cout << "Default ACP Create Under " << ACP_URL << std::endl;

    json::value json_data;
    json_data[U("m2m:acp")][U("rn")] = json::value::string(utility::conversions::to_string_t(rn));

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));


    request.set_body(json_data);

    auto response = client.request(request).get();

    // Print the HTTP response
    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }
}

void send_oneM2M::acp_create_one_ACP(const parse_json::parsed_struct& data, vector<string> ACP_NAMES, int num, ...)
{
    int index = 0;

    string ACP_URL = this->URL_TO_CSE;
    string rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const string);
        ACP_URL += "/";
        ACP_URL += ret;
    }

	va_end(args);

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::POST) ;

    std::cout << "new ACP : " << "C" + data.building_name << " is CREATING Under " << ACP_URL << std::endl;

    json::value json_data;

    json_data[U("m2m:acp")][U("rn")] = json::value::string(utility::conversions::to_string_t(rn));

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")] = json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")][0] =  json::value::string(utility::conversions::to_string_t(DEFAULT_ORIGINATOR));
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acop")] = json::value::number(acop_all);

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")] = json::value::array();
    for(const string& ACOR_NAME : ACP_NAMES)
    {
	    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")][index] = json::value::string(utility::conversions::to_string_t("C"+ACOR_NAME));
        index++;
    }
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acop")] = json::value::number(acop_all);

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    request.set_body(json_data);

    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }
    else
    {
		utility::string_t response_body_string = response.extract_string().get();
	    //std::wcout << L"ACP HTTP Response:\n" << response_body_string << std::endl;
    }
}

void send_oneM2M::acp_update(const parse_json::parsed_struct& data, vector<string> ACP_NAMES, int num, ...)
{
    int index = 0;

    string ACP_URL = this->URL_TO_CSE;
    string rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        ACP_URL += "/";
	        ACP_URL += ret;
        }
		va_end(args);
    }
    ACP_URL += "/";
    ACP_URL += DEFAULT_ACP_NAME;

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::PUT) ;

    std::cout << "new ACP : " << "C" + data.building_name << " is Updating Update Under " << ACP_URL << std::endl;

    json::value json_data;

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")] = json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")][0] =  json::value::string(utility::conversions::to_string_t(DEFAULT_ORIGINATOR));
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acop")] = json::value::number(acop_all);

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")] = json::value::array();
    for(const string& ACOR_NAME : ACP_NAMES)
    {
	    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")][index] = json::value::string(utility::conversions::to_string_t("C"+ACOR_NAME));
        index++;
    }
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acop")] = json::value::number(acop_all);

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    request.set_body(json_data);

    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }
}

// return 0 = No ACP, return 1 = Has ACP, but No ACP NAME, 2 = Has ACP, and Has ACP NAME
int send_oneM2M::acp_validate(string ACP_NAME, int num, ...)
{
    string originator_name = "CAdmin";
    string URL = "";

    URL += host_protocol;
    URL += host_ip;
    URL += ":";
    URL += host_port;
    URL += "/";
    URL += DEFAULT_CSE_NAME;
    URL += "/";
    URL += DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
            URL += "/";
            URL += ret;
        }
		va_end(args);
        std::cout << "ACP Name : " << ACP_NAME << " validation on URL : " << URL << std::endl;
    }
    else
    {
        std::cout << "ACP Name : " << ACP_NAME << " validation on URL : " << URL << std::endl;
    }

    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator_name));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

	auto response = client.request(request).get();
    utility::string_t response_body_string = response.extract_string().get();

    //std::wcout << L"ACP HTTP Response:\n" << response_body_string << std::endl;

#ifndef oneM2M_tinyIoT
    if(response.status_code() == 404)
    {
	    return 0;
    }
    else if(response.status_code() == 200)
    {
        if(response_body_string.find(utility::conversions::to_string_t(ACP_NAME)) != utility::string_t::npos)
        {
	        return 2;
        }
        else
        {
	     	return 1;
        }
    }
#endif

#ifdef oneM2M_tinyIoT
    if(response_body_string.find(U("m2m:cb")) != utility::string_t::npos)
    {
	    return 0;
    }
    else if(response_body_string.find(U("m2m:acp")) != utility::string_t::npos)
    {
        if(response_body_string.find(utility::conversions::to_string_t(ACP_NAME)) != utility::string_t::npos)
        {
        	return 2;
        }
        else
        {
	        return 1;
        }
    }
    else
    {
	    return false;
    }
#endif
}

void send_oneM2M::ae_create(string AE_NAME)
{
    string CSE_URL = this->URL_TO_CSE;

	string building_name = AE_NAME;

    string originator = "C" + building_name;
    string api = "NBuilding";
    string ri = building_name;

    http_client client(utility::conversions::to_string_t(CSE_URL));
	http_request request(methods::POST) ;

    std::cout << "AE : " <<  building_name << " Create Under " << CSE_URL << std::endl;

    json::value json_data;
    json_data[U("m2m:ae")][U("rn")] = json::value::string(utility::conversions::to_string_t(ri));
    json_data[U("m2m:ae")][U("api")] = json::value::string(utility::conversions::to_string_t(api));
    json_data[U("m2m:ae")][U("rr")] = json::value::boolean(false);
    json_data[U("m2m:ae")][U("srv")] = json::value::array();
    json_data[U("m2m:ae")][U("srv")][0] =json::value::string(U("2a"));
    json_data[U("m2m:ae")][U("srv")][1] = json::value::string(U("3"));
    json_data[U("m2m:ae")][U("srv")][2] = json::value::string(U("4"));

    // Create an HTTP request
    request.headers().set_content_type(U("application/vnd.onem2m-res+json; ty=2"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(ri));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    request.set_body(json_data);

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }

    // Print the HTTP response
    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

    this->URL_TO_AE = this->URL_TO_CSE + "/" + ri;
}

void send_oneM2M::ae_retrieve()
{

}

bool send_oneM2M::ae_validate(const parse_json::parsed_struct& data, int num, ...)
{
    string originator_name = "C" + data.building_name;
    string AE_ID = data.building_name;

    string URL = this->URL_TO_CSE;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        URL += "/";
	        URL += ret;
        }
		va_end(args);

        URL += "/'";
        URL += AE_ID;
    }

    std::cout << "AE Name : " << AE_ID << " vaildate on URL : " << URL << std::endl;
    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator_name));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

	auto response = client.request(request).get();
	utility::string_t response_body_string = response.extract_string().get();

    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

    if(response_body_string.find(U("m2m:cb")) != utility::string_t::npos)
    {
	    return false;
    }
    else if(response_body_string.find(U("m2m:ae")) != utility::string_t::npos)
    {
        return true;
    }
    else
    {
	    return false;
    }
}

void send_oneM2M::cnt_create(string originator_string, int num, ...)
{
    va_list args;
    va_start(args, num);

    string CNT_URL = this->URL_TO_AE;
    string originator = originator_string;
    string CNT_NAME = "";

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<num; i++)
        {
	        auto ret = va_arg(args, const string);
	        CNT_URL += "/";
	        CNT_URL += ret;
        }
        CNT_NAME = va_arg(args, const string);
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(CNT_URL));
	http_request request(methods::POST) ;

    //std::cout << "CNT : " << CNT_NAME << " Create Under " << CNT_URL << std::endl;

    json::value json_data1;
    json_data1[U("m2m:cnt")][U("rn")] = json::value::string(utility::conversions::to_string_t(CNT_NAME));

#ifdef oneM2M_tinyIoT
	json_data1[U("m2m:cnt")][U("ri")] = json::value::string(utility::conversions::to_string_t(CNT_NAME));
#endif

    json_data1[U("m2m:cnt")][U("lbl")] = json::value::array();
    json_data1[U("m2m:cnt")][U("lbl")][0] =json::value::string(U("key1"));
    json_data1[U("m2m:cnt")][U("lbl")][1] = json::value::string(U("key2"));

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=3"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(CNT_NAME));
	request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));


    request.set_body(json_data1);

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    // Print the HTTP response
    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;
    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }

}

void send_oneM2M::cnt_retrieve()
{

}

bool send_oneM2M::cnt_validate(const parse_json::parsed_struct& data, int num, ...)
{
    string originator_name = "C" + data.building_name;
    string CNT_ID = data.device_name;

    string URL = this->URL_TO_CSE;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        URL += "/";
	        URL += ret;
        }
		va_end(args);
    }
    URL += "/";
	URL += CNT_ID;

    std::cout << "CNT Name : " << CNT_ID << " vaildate on URL : " << URL << std::endl;
    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator_name));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

	auto response = client.request(request).get();
	utility::string_t response_body_string = response.extract_string().get();

    //std::wcout << L"CNT HTTP Response:\n" << response.to_string() << std::endl;

    if(response_body_string.find(U("m2m:cb")) != utility::string_t::npos)
    {
	    return false;
    }
    else if(response_body_string.find(U("m2m:cnt")) != utility::string_t::npos)
    {
        return true;
    }
    else
    {
	    return false;
    }
    return false;
}

void send_oneM2M::cin_create(string originator, string CIN_NAME, string payload, int num, ...)
{
    va_list args;
    va_start(args, num);

    string CIN_URL = this->URL_TO_AE;

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        CIN_URL += "/";
	        CIN_URL += ret;
        }
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(CIN_URL));
	http_request request(methods::POST) ;

    //std::cout <<  "CIN : " << CIN_NAME << " Create Under " << CIN_URL << std::endl;

    json::value json_data;

	//json_data[U("m2m:cin")][U("rn")] = json::value::string(utility::conversions::to_string_t(CIN_NAME));
	//json_data[U("m2m:cin")][U("cnf")] = json::value::string(utility::conversions::to_string_t("application/json"));
    json_data[U("m2m:cin")][U("con")] =json::value::string(utility::conversions::to_string_t(payload));

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=4"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(CIN_NAME));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    request.set_body(json_data);

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }

    else if(response.status_code() == status_codes::Created)
    {
	    json::value response_json = response.extract_json().get();
        std::wstring ri = response_json[U("m2m:cin")][U("ri")].as_string();

        //std::wcout  << L"RI is : " << ri << std::endl;
    }
    // Print the HTTP response
    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;
}

void send_oneM2M::cin_update(const parse_json::parsed_struct& data, int num, ...)
{

}

http_response send_oneM2M::cin_retrieve_la(string originator, int num, ...)
{
    va_list args;
    va_start(args, num);

    string CIN_URL = this->URL_TO_AE;

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const string);
	        CIN_URL += "/";
	        CIN_URL += ret;
        }
		va_end(args);
    }
    CIN_URL += "/";
    CIN_URL += "la";

    http_client client(utility::conversions::to_string_t(CIN_URL));
	http_request request(methods::GET) ;

    //std::cout << "CIN RETRIEVE Under : " << CIN_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
#ifndef oneM2M_tinyIoT
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
#endif
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(response.status_code() == status_codes::Created)
    {
	    json::value response_json = response.extract_json().get();
        std::wstring ri = response_json[U("m2m:cin")][U("ri")].as_string();

        std::wcout  << L"RI is : " << ri << std::endl;
    }
    // Print the HTTP response
    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

    return response;
}

void send_oneM2M::discovery_retrieve(string originator, int num, ...)
{
    //FILTER USAGE MUST SET TO 1 FOR USE DISCOVERY
    int fu = 1;

    //RETURN ITS RESOURCE THAT HAS TYPE NUMBER
    int ty = 4;

     //RETURN PARENT RESOURCE THAT HAS CHILD TYPE NUMBER
    //IF IN NESTED CNT, IT RETURNS ALL OF PARENT CNT, AE, and CSEs
    int chty = 3;

    //RETURN CHILD RESOURCE THAT HAS PARENT TYPE NUMBER
    //if AE - CNT1 - CNT2 - CIN1, and pty = 3(CNT) => IT RETURNS CNT2, and CIN1
    int pty = 3;

    //MAXIMUM NUMBER of RETURNED RESOURCES
    int lim = 5;

    //RETURNS RESOURCE THAT CREATED BEFORE THIS TIMESTAMP
    string crb = "20240124T012530";
    
    //RETURNS RESOURCE THAT CREATED AFTER THIS TIMESTAMP
    string cra = "20240124T012630";

    int drt = 2;

    va_list args;
    va_start(args, num);

    string DIS_URL = this->URL_TO_AE;

	auto start = system_clock::now();
    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const string);
        DIS_URL += "/";
        DIS_URL += ret;
    }
	va_end(args);
    
    DIS_URL +=
        "?fu=" + std::to_string(fu) +
        "&ty=" + std::to_string(ty);
        //"&cra=" + cra +
        //"&crb=" + crb;
        //"&lim=" + std::to_string(lim);

#ifndef oneM2M_tinyIoT 
        DIS_URL += "&drt=" + std::to_string(drt);
#endif

    http_client client(utility::conversions::to_string_t(DIS_URL));
	http_request request(methods::GET) ;

    std::cout << "DISCOVERY Under : " << DIS_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
#ifndef oneM2M_tinyIoT
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
#endif
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    std::chrono::duration<double> interval = system_clock::now() - start;;
	cout << URL_TO_AE << " DISCOVERY TIME : " << interval.count()<< " seconds..." << endl;

    if(response.status_code() == status_codes::OK)
    {
        json::value response_json = response.extract_json().get();

    	json::array arr = response_json[U("m2m:uril")].as_array();

        std::wcout << L"DISCOVERED RESOURCE :" <<  std::endl;
        for(const json::value& elem : arr)
        {
	        std::wcout << elem.as_string() << endl;
        }  
    }
    // Print the HTTP response
    else
    {
	     std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;   
    }
}

void send_oneM2M::result_content_retrieve(string originator, int num, ...)
{
    //RETURN ITS RESOURCE THAT HAS TYPE NUMBER
    int ty = 4;

     //RETURN PARENT RESOURCE THAT HAS CHILD TYPE NUMBER
    //IF IN NESTED CNT, IT RETURNS ALL OF PARENT CNT, AE, and CSEs
    int chty = 3;

    //RETURN CHILD RESOURCE THAT HAS PARENT TYPE NUMBER
    //if AE - CNT1 - CNT2 - CIN1, and pty = 3(CNT) => IT RETURNS CNT2, and CIN1
    int pty = 3;

    int rcn = 8;

    va_list args;
    va_start(args, num);

    auto start = system_clock::now();
    string RCN_URL = this->URL_TO_AE;

    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const string);
        RCN_URL += "/";
        RCN_URL += ret;
    }
	va_end(args);
    
    RCN_URL +=
        "?rcn=" + std::to_string(rcn) +
        "&ty=" + std::to_string(ty);

    http_client client(utility::conversions::to_string_t(RCN_URL));
	http_request request(methods::GET) ;

    std::cout << "RCN RETRIEVE Under : " << RCN_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
#ifndef oneM2M_tinyIoT
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
#endif
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    std::chrono::duration<double> interval = system_clock::now() - start;;
	cout << URL_TO_AE << " RCN TIME : " << interval.count()<< " seconds..." << endl;

    if(response.status_code() == status_codes::OK)
    {
        json::value response_json = response.extract_json().get();
        std::wstring json_str = response_json.serialize();

		// Parse the string using jsoncpp
		nlohmann::json j = nlohmann::json::parse(json_str);

    	std::cout << "HTTP Response:\n" << j.dump(4) << std::endl;   
    }
    else
    {
	     std::wcerr << L"HTTP ERROR :\n" << response.status_code() << std::endl;   
    }
}

void send_oneM2M::grp_create(const parse_json::parsed_struct& data)
{
    if(this->URL_TO_AE.empty())
    {
        std::cout << "NO AE Addr Before GRP Creation" << endl;
	    exit(0);
    }
    else
    {

		string CSE_URL = this->URL_TO_CSE;

		string building_name = data.building_name;
	    string device_name = data.device_name;

	    string originator = "C" + device_name;
	    string api = "NElevator";
	    string ri = device_name;

	    http_client client(utility::conversions::to_string_t(CSE_URL));
		http_request request(methods::POST) ;

        std::cout << "URL is" << CSE_URL << "\n"  << std::endl;
		std::cout << "Creating AE\n"  << std::endl;

	    json::value json_data;
	    json_data[U("m2m:ae")][U("rn")] = json::value::string(utility::conversions::to_string_t(ri));
	    json_data[U("m2m:ae")][U("api")] = json::value::string(utility::conversions::to_string_t(api));
	    json_data[U("m2m:ae")][U("rr")] = json::value::boolean(false);
	    json_data[U("m2m:ae")][U("srv")] = json::value::array();
	    json_data[U("m2m:ae")][U("srv")][0] =json::value::string(U("2a"));
	    json_data[U("m2m:ae")][U("srv")][1] = json::value::string(U("3"));
	    json_data[U("m2m:ae")][U("srv")][2] = json::value::string(U("4"));

	    // Create an HTTP request
	    request.headers().set_content_type(U("application/vnd.onem2m-res+json; ty=2"));
	    request.headers().add(U("User-Agent"),U("cpprestsdk"));
	    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
	    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(ri));
        request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(RVI));

	    request.set_body(json_data);

	    // Send the HTTP request and wait for the response
	    auto response = client.request(request).get();

	    // Print the HTTP response
	    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

	    this->URL_TO_AE = this->URL_TO_CSE + "/" + ri;
    }
}