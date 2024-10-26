#include "send_oneM2M.h"
#include "nlohmann/json.hpp"
#include <cstdarg>
#include <chrono>

using namespace web;
using namespace web::http::client;

send_oneM2M::send_oneM2M()
{

}

send_oneM2M::send_oneM2M(Wparsed_struct parsed_struct)
{
#ifdef HTTP
    uri_to_ip = PROTOCOL_NAME;
    uri_to_ip.append(CSE_ADDRW);
    uri_to_ip.append(L":");
    uri_to_ip.append(to_wstring(CSE_LISTEN_PORT));

    uri_to_cse = uri_to_ip;
    uri_to_cse.append(L"/");
    uri_to_cse.append(DEFAULT_CSE_NAME);

    uri_to_dt_notification = PROTOCOL_NAME;
    uri_to_dt_notification.append(DT_SERVER_ADDR);
    uri_to_dt_notification.append(L":");
    uri_to_dt_notification.append(to_wstring(oneM2M_NOTIFICATION_LISTEN_PORT));

    uri_only_cse_and_building = DEFAULT_CSE_NAME;
    uri_only_cse_and_building.append(L"/");
    uri_only_cse_and_building.append(parsed_struct.building_name);
#endif

    if(!parsed_struct.building_name.empty())
    {
	    uri_to_building = uri_to_cse;
        uri_to_building.append(L"/");
        uri_to_building.append(parsed_struct.building_name);

        if(!parsed_struct.device_name.empty())
		{
            uri_to_elevator = uri_to_building;
            uri_to_elevator.append(L"/");
            uri_to_elevator.append(parsed_struct.device_name);
		}
    }

    building_originator = L"C" + parsed_struct.building_name;
}

void send_oneM2M::set_new_building_uri(wstring building_name)
{
    uri_to_building = uri_to_cse;
	uri_to_building.append(L"/");
	uri_to_building.append(building_name);

    uri_only_cse_and_building = DEFAULT_CSE_NAME;
    uri_only_cse_and_building.append(L"/");
    uri_only_cse_and_building.append(building_name);

    building_originator = L"C" + building_name;
}

void send_oneM2M::set_new_elevator_uri(wstring building_name)
{
	uri_to_elevator = uri_to_building;
	uri_to_elevator.append(L"/");
	uri_to_elevator.append(building_name);
}

void send_oneM2M::acp_create(int num, ...)
{
    wstring ACP_URL = this->uri_to_cse;
    wstring rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
	        ACP_URL += L"/";
	        ACP_URL += ret;
        }
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::POST) ;

    //std::cout << "Default ACP Create Under " << ACP_URL << std::endl;

    json::value json_data;
    json_data[U("m2m:acp")][U("rn")] = json::value::string(utility::conversions::to_string_t(rn));

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));


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

void send_oneM2M::acp_create_one_ACP(vector<wstring>& ACP_NAMES, int num, ...)
{
    int index = 0;

    wstring ACP_URL = this->uri_to_cse;
    wstring rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const wstring);
        ACP_URL += L"/";
        ACP_URL += ret;
    }

	va_end(args);

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::POST) ;

    json::value json_data;

    json_data[U("m2m:acp")][U("rn")] = json::value::string(utility::conversions::to_string_t(rn));

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")] = json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")][0] =  json::value::string(utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acop")] = json::value::number(ACOP_ALL);

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")] = json::value::array();

    for(const wstring& ACOR_NAME : ACP_NAMES)
    {
	    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")][index] = json::value::string(utility::conversions::to_string_t(L"C"+ACOR_NAME));
        index++;
    }

    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acop")] = json::value::number(ACOP_ALL);

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

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

void send_oneM2M::acp_update(vector<wstring> ACP_NAMES, int num, ...)
{
    int index = 0;

    wstring ACP_URL = this->uri_to_cse;
    wstring rn = DEFAULT_ACP_NAME;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
	        ACP_URL += L"/";
	        ACP_URL += ret;
        }
		va_end(args);
    }
    ACP_URL += L"/";
    ACP_URL += DEFAULT_ACP_NAME;

    http_client client(utility::conversions::to_string_t(ACP_URL));
	http_request request(methods::PUT) ;

    //std::cout << "new ACP : " << "C" + data.building_name << " is Updating Update Under " << ACP_URL << std::endl;

    json::value json_data;

    json_data[U("m2m:acp")][U("pv")] = json::value::object();
	json_data[U("m2m:acp")][U("pv")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")] = json::value::array();
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acor")][0] =  json::value::string(utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    json_data[U("m2m:acp")][U("pv")][U("acr")][0][U("acop")] = json::value::number(ACOP_ALL);

    json_data[U("m2m:acp")][U("pvs")] = json::value::object();
	json_data[U("m2m:acp")][U("pvs")][U("acr")] =  json::value::array();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0] =  json::value::object();
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")] = json::value::array();

    for(const wstring& ACOR_NAME : ACP_NAMES)
    {
	    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acor")][index] = json::value::string(utility::conversions::to_string_t(L"C"+ACOR_NAME));
        index++;
    }
    json_data[U("m2m:acp")][U("pvs")][U("acr")][0][U("acop")] = json::value::number(ACOP_ALL);

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=1"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(rn));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    request.set_body(json_data);

    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }
}

// return 0 = No ACP, return 1 = Has ACP, but No ACP NAME, 2 = Has ACP, and Has ACP NAME
int send_oneM2M::acp_validate(wstring ACP_NAME, int num, ...)
{
    wstring URL = uri_to_cse;

    va_list args;
    va_start(args, num);

	if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
            URL.append(L"/");
            URL.append(ret);

        }
		va_end(args);
    }

    URL.append(L"/");
    URL.append(DEFAULT_ACP_NAME);

    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

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
    if(response.status_code() == 404)
    {
        return DT_ACP_NOT_FOUND;
    }
    else if(response_body_string.find(U("m2m:acp")) != utility::string_t::npos)
    {
        if(response_body_string.find(utility::conversions::to_string_t(ACP_NAME)) != utility::string_t::npos)
        {
        	return ACP_FOUND;
        }
        else
        {
	        return THIS_DT_ACP_NOT_FOUND;
        }
    }
    else
    {
	    return false;
    }
#endif
}

vector<wstring> send_oneM2M::acp_retrieve(int num, ...)
{
    wstring URL = uri_to_cse;

    va_list args;
    va_start(args, num);

    if (num >= 1)
    {
        for (int i = 1; i <= num; i++)
        {
            auto ret = va_arg(args, const wstring);
            URL.append(L"/");
            URL.append(ret);

        }
        va_end(args);
    }

    URL.append(L"/");
    URL.append(DEFAULT_ACP_NAME);

    http_client client(utility::conversions::to_string_t(URL));
    http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"), U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    web::http::http_response response = client.request(request).get();
    utility::string_t response_body_string = response.extract_string().get();

    vector<wstring> ACP_NAMES;
    //GET ACP NAME FROM RESPONSE BODY
    try 
    {
        json::value json_data = json::value::parse(response_body_string);
        if (json_data.has_object_field(U("m2m:acp")))
        {
			json::value acp = json_data.at(U("m2m:acp"));
			json::value pvs_object = acp.at(U("pvs"));
			const auto acr_array = pvs_object.at(U("acr")).as_array();

            for (const auto& acr : acr_array) {
                auto acorArray = acr.at(U("acor")).as_array();

                for (const auto& acor : acorArray) {
                    auto name = acor.as_string();

                    // name to wstring
                    wstring acor_name = utility::conversions::to_utf16string(name);
                    ACP_NAMES.push_back(acor_name);
                }
            }
        }
	}
	catch (json::json_exception& e)
	{
		std::wcout << L"ACP NAME RETRIEVE ERROR : " << e.what() << std::endl;
    }
    return ACP_NAMES;
}

void send_oneM2M::ae_create(wstring AE_NAME)
{
    wstring api = L"NBuilding";

    http_client client(utility::conversions::to_string_t(uri_to_cse));
	http_request request(methods::POST) ;

    json::value json_data;
    json_data[U("m2m:ae")][U("rn")] = json::value::string(utility::conversions::to_string_t(AE_NAME));
    json_data[U("m2m:ae")][U("api")] = json::value::string(utility::conversions::to_string_t(api));
    json_data[U("m2m:ae")][U("rr")] = json::value::boolean(true);
    json_data[U("m2m:ae")][U("srv")] = json::value::array();
    json_data[U("m2m:ae")][U("srv")][0] =json::value::string(U("2a"));
    json_data[U("m2m:ae")][U("srv")][1] = json::value::string(U("3"));
    json_data[U("m2m:ae")][U("srv")][2] = json::value::string(U("4"));
    json_data[U("m2m:ae")][U("poa")] = json::value::array();
	json_data[U("m2m:ae")][U("poa")][0] =json::value::string(utility::conversions::to_string_t(uri_to_dt_notification));

    // Create an HTTP request
    request.headers().set_content_type(U("application/vnd.onem2m-res+json; ty=2"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(AE_NAME));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    request.set_body(json_data);

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(400 <= response.status_code()  && response.status_code() < 500)
    {
	    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
        exit(0);
    }
}

bool send_oneM2M::ae_validate(const Wparsed_struct& data, int num, ...)
{
    wstring originator_name = L"C" + data.building_name;
    wstring AE_ID = data.building_name;
    wstring URL = this->uri_to_cse;

    va_list args;
    va_start(args, num);

    for(int i=1; i<=num; i++)
    {
	    auto ret = va_arg(args, const wstring);
	    URL += L"/";
	    URL += ret;
    }
	va_end(args);

    URL += L'/';
    URL += AE_ID;

    //std::cout << "AE Name : " << AE_ID << " vaildate on URL : " << URL << std::endl;
    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("Accept"), utility::conversions::to_string_t("application/json"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(DEFAULT_ORIGINATOR_NAME));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

	auto response = client.request(request).get();
	utility::string_t response_body_string = response.extract_string().get();

    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;

    if(response_body_string.find(U("m2m:cb")) != utility::string_t::npos)
    {
	    return BUILDING_NOT_FOUND;
    }
    else if(response_body_string.find(U("m2m:ae")) != utility::string_t::npos)
    {
        return BUILDING_FOUND;
    }
    else
    {
	    return BUILDING_NOT_FOUND;
    }
}

void send_oneM2M::cnt_create(int num, ...)
{
    va_list args;
    va_start(args, num);

    wstring CNT_URL = this->uri_to_building;
    wstring CNT_NAME = L"";

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<num; i++)
        {
	        auto ret = va_arg(args, const wstring);
            CNT_URL.append(L"/");
            CNT_URL.append(ret);
        }
        CNT_NAME = va_arg(args, const wstring);
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(CNT_URL));
	http_request request(methods::POST) ;

    json::value json_data1;
    json_data1[U("m2m:cnt")][U("rn")] = json::value::string(utility::conversions::to_string_t(CNT_NAME));
    json_data1[U("m2m:cnt")][U("mni")] = json::value::number(MNI);
    json_data1[U("m2m:cnt")][U("mbs")] = json::value::number(MBS);

    json_data1[U("m2m:cnt")][U("lbl")] = json::value::array();
    json_data1[U("m2m:cnt")][U("lbl")][0] =json::value::string(U("key1"));
    json_data1[U("m2m:cnt")][U("lbl")][1] = json::value::string(U("key2"));

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=3"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(building_originator));
	request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));


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

bool send_oneM2M::cnt_validate(int num, ...)
{
	va_list args;
    va_start(args, num);

    wstring CNT_URL = this->uri_to_building;

    if(num>=1)
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
	        CNT_URL += L"/";
	        CNT_URL += ret;
        }
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(CNT_URL));
	http_request request(methods::GET);

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();
    utility::string_t response_body_string = response.extract_string().get();

    if(response.status_code() == status_codes::OK && response_body_string.find(U("m2m:cnt")) != utility::string_t::npos)
    {
        return true;
    }
    else
    {
        return false;
	}
}

void send_oneM2M::sub_create(map<wstring, wstring>* mapper, wstring originator_string, int num, ...)
{
    try
    {
		va_list args;
	    va_start(args, num);

	    wstring CNT_URL = this->uri_to_building;
	    wstring originator = originator_string;
	    wstring CNT_NAME = L"";

	    if(num<1)
	    {
	        exit(0);
	    }
	    else
	    {
	        for(int i=1; i<num; i++)
	        {
		        auto ret = va_arg(args, const wstring);
		        CNT_URL += L"/";
		        CNT_URL += ret;
	        }
	        CNT_NAME = va_arg(args, const wstring);
			va_end(args);
		    
			http_client client(utility::conversions::to_string_t(CNT_URL));
			http_request request(methods::POST) ;

		    json::value json_data;
		    json_data[U("m2m:sub")][U("rn")] = json::value::string(utility::conversions::to_string_t(CNT_NAME));

		#ifdef oneM2M_tinyIoT
		#endif

		    json_data[U("m2m:sub")][U("enc")] = json::value::object();
			json_data[U("m2m:sub")][U("enc")][U("net")] =  json::value::array();
			json_data[U("m2m:sub")][U("enc")][U("net")][0] =  json::value::number(3);
			json_data[U("m2m:sub")][U("enc")][U("net")][1] =  json::value::number(4);

		    json_data[U("m2m:sub")][U("nu")] = json::value::array();
		    json_data[U("m2m:sub")][U("nu")][0] = json::value::string(utility::conversions::to_string_t(uri_only_cse_and_building));

		    // Create an HTTP request
		    request.headers().set_content_type(U("application/json; ty=23"));
		    request.headers().add(U("User-Agent"),U("cpprestsdk"));
		    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
			request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));
			request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(building_originator));

		    request.set_body(json_data);

		    // Send the HTTP request and wait for the response
		    auto response = client.request(request).get();

		    // Print the HTTP response
		    //std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;
		    if(400 <= response.status_code()  && response.status_code() < 500)
		    {
			    std::wcout << L"HTTP Response 400 Range ERROR, RESPONSE IS : " << response.to_string() << std::endl;
		        exit(0);
		    }
#ifdef oneM2M_ACME
            else if (response.status_code() == status_codes::Created)
            {
				json::value response_json = response.extract_json().get();
				std::wstring ri = response_json[U("m2m:sub")][U("ri")].as_string();

                mapper->emplace(ri, CNT_URL);
			}
#endif
	    }
    }
    catch (const std::exception& e)
    {
	    std::cerr << "Exception caught on main.cpp: " << e.what() << std::endl;
        exit(0);
    }
}

void send_oneM2M::cin_create(wstring CIN_NAME, wstring payload, int num, ...)
{
    va_list args;
    va_start(args, num);

    wstring CIN_URL = this->uri_to_building;

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
	        CIN_URL += L"/";
	        CIN_URL += ret;
        }
		va_end(args);
    }

    http_client client(utility::conversions::to_string_t(CIN_URL));
	http_request request(methods::POST) ;

    json::value json_data;

    json_data[U("m2m:cin")][U("con")] =json::value::string(utility::conversions::to_string_t(payload));

    // Create an HTTP request
    request.headers().set_content_type(U("application/json; ty=4"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));
	request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(building_originator));

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
    }
}

void send_oneM2M::cin_update(const Wparsed_struct& data, int num, ...)
{

}

http_response send_oneM2M::cin_retrieve_la(wstring originator, int num, ...)
{
    va_list args;
    va_start(args, num);

    wstring CIN_URL = this->uri_to_building;

    if(num<1)
    {
        exit(0);
    }
    else
    {
        for(int i=1; i<=num; i++)
        {
	        auto ret = va_arg(args, const wstring);
	        CIN_URL += L"/";
	        CIN_URL += ret;
        }
		va_end(args);
    }
    CIN_URL += L"/";
    CIN_URL += L"la";

    http_client client(utility::conversions::to_string_t(CIN_URL));
	http_request request(methods::GET) ;

    //std::cout << "CIN RETRIEVE Under : " << CIN_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
#ifndef oneM2M_tinyIoT
    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(DEFAULT_RI));
#endif
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(response.status_code() == status_codes::Created)
    {
	    json::value response_json = response.extract_json().get();
        std::wstring ri = response_json[U("m2m:cin")][U("ri")].as_string();
    }

    return response;
}

web::json::array send_oneM2M::discovery_retrieve(wstring originator, int discovery_type_num, int level, int num, ...)
{
    //FILTER USAGE MUST SET TO 1 FOR USE DISCOVERY
    int fu = 1;

    //RETURN ITS RESOURCE THAT HAS TYPE NUMBER
    int ty = discovery_type_num;

     //RETURN PARENT RESOURCE THAT HAS CHILD TYPE NUMBER
    //IF IN NESTED CNT, IT RETURNS ALL OF PARENT CNT, AE, and CSEs
    int chty = 3;

    //RETURN CHILD RESOURCE THAT HAS PARENT TYPE NUMBER
    //if AE - CNT1 - CNT2 - CIN1, and pty = 3(CNT) => IT RETURNS CNT2, and CIN1
    int pty = 3;

    //MAXIMUM NUMBER of RETURNED RESOURCES
    int lim = 5;

    //RETURNS RESOURCE THAT CREATED BEFORE THIS TIMESTAMP
    wstring crb = L"20240124T012530";
    
    //RETURNS RESOURCE THAT CREATED AFTER THIS TIMESTAMP
    wstring cra = L"20240124T012630";

    int drt = 2;

    va_list args;
    va_start(args, num);

    wstring URL = this->uri_to_cse;

    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const wstring);
        URL += L"/";
        URL += ret;
    }
	va_end(args);
    
    URL +=
        L"?fu=" + to_wstring(fu) +
        L"&ty=" + to_wstring(ty) +
        L"&lvl=" + to_wstring(level);

#ifndef oneM2M_tinyIoT 
    URL += L"&drt=" + to_wstring(drt);
#endif

    http_client client(utility::conversions::to_string_t(URL));
	http_request request(methods::GET) ;

    //std::cout << "DISCOVERY Under : " << DIS_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(originator));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

	//std::cout << URL_TO_AE << " DISCOVERY TIME : " << interval.count()<< " seconds..." << std::endl;

    if(response.status_code() == status_codes::OK)
    {
        json::value response_json = response.extract_json().get();

    	json::array arr = response_json[U("m2m:uril")].as_array();

        //std::wcout << L"DISCOVERED RESOURCE :" <<  std::endl;
        return arr;
    }
    // Print the HTTP response
    else
    {
	     std::wcout << L"HTTP Response:\n" << response.to_string() << std::endl;   
    }
}

void send_oneM2M::result_content_retrieve(int num, ...)
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

    wstring RCN_URL = this->uri_to_building;

    for(int i=1; i<=num; i++)
    {
        auto ret = va_arg(args, const wstring);
        RCN_URL += L"/";
        RCN_URL += ret;
    }
	va_end(args);
    
    RCN_URL += L"?rcn=" + to_wstring(rcn) + L"&ty=" + to_wstring(ty);

    http_client client(utility::conversions::to_string_t(RCN_URL));
	http_request request(methods::GET) ;

    //std::cout << "RCN RETRIEVE Under : " << RCN_URL << std::endl;

    // Create an HTTP request
    request.headers().set_content_type(U("application/json"));
    request.headers().add(U("User-Agent"),U("cpprestsdk"));
    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
    request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

    // Send the HTTP request and wait for the response
    auto response = client.request(request).get();

    if(response.status_code() == status_codes::OK)
    {
        json::value response_json = response.extract_json().get();
        std::wstring json_str = response_json.serialize();

		// Parse the string using jsoncpp
		nlohmann::json j = nlohmann::json::parse(json_str);

    	//std::cout << "HTTP Response:\n" << j.dump(4) << std::endl;   
    }
    else
    {
	     //std::wcerr << L"HTTP ERROR :\n" << response.status_code() << std::endl;   
    }
}

void send_oneM2M::grp_create(const Wparsed_struct& data)
{
    if(this->uri_to_building.empty())
    {
        std::cout << "NO AE Addr Before GRP Creation" << std::endl;
	    exit(0);
    }
    else
    {

		wstring CSE_URL = this->uri_to_cse;

		wstring building_name = data.building_name;
	    wstring device_name = data.device_name;

	    wstring originator = L"C" + device_name;
	    wstring api = L"NElevator";
	    wstring ri = device_name;

	    http_client client(utility::conversions::to_string_t(CSE_URL));
		http_request request(methods::POST) ;

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
	    request.headers().add(U("X-M2M-Origin"), utility::conversions::to_string_t(building_originator));
	    request.headers().add(U("X-M2M-RI"), utility::conversions::to_string_t(ri));
        request.headers().add(U("X-M2M-RVI"), utility::conversions::to_string_t(DEFAULT_RVI));

	    request.set_body(json_data);

	    // Send the HTTP request and wait for the response
	    auto response = client.request(request).get();

	    // Print the HTTP response
	    this->uri_to_building = this->uri_to_cse + L"/" + ri;
    }
}

vector<double> send_oneM2M::retrieve_oneM2M_Energy_CIN(Wparsed_struct parseStruct)
{
    wstring building_name = parseStruct.building_name;
    wstring originator_name = L"C" + building_name;
    wstring device_name = parseStruct.device_name;

    web::http::http_response res;
    web::json::value response_json;
    std::wstring con;

    vector<double> energy_info;
    try
    {
        //CIN RETRIEVE
        res = cin_retrieve_la(originator_name, 2, device_name, device_name + L"_Energy");

        response_json = res.extract_json().get();
        con = response_json[U("m2m:cin")][U("con")].as_string();

        // con info is [idle] [standby] [iso_reference_cycle_energy]
        // move this string to vector<double> energy_info
        wstringstream iss(con);
        wstring value;
        while (iss >> value)
        {
            energy_info.push_back(stod(value));
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception Caught : " << e.what() << std::endl;
    }

    return energy_info;
}