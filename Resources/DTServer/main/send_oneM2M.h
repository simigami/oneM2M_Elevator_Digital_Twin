#pragma once
#include <string>
#include <cpprest/http_client.h>
#include "parse_json.h"

#define acop_all 63

#define oneM2M_tinyIoT

#ifdef oneM2M_tinyIoT
#define DEFAULT_RI "DT_RI"
#define RVI "2a"
#endif

#ifndef oneM2M_tinyIoT
#define oneM2M_ACME
#define DEFAULT_RI "DT_RI"
#define RVI "4"
#endif

using namespace web::http;

class send_oneM2M {
public:
    send_oneM2M();
    send_oneM2M(parse_json::parsed_struct parsed_struct);

    void grp_create(const parse_json::parsed_struct& data);

    void acp_create(int num, ...);
    void acp_create_one_ACP(const parse_json::parsed_struct& data, vector<string> ACP_NAMES, int num, ...);
	void acp_update(const parse_json::parsed_struct& data, vector<string> ACP_NAMES, int num, ...);
    int acp_validate(string ACP_NAME, int num, ...);

    void ae_create(string AE_NAME);
    void ae_retrieve();
    bool ae_validate(const parse_json::parsed_struct& data, int num, ...);

    //void cnt_create(const parse_json::parsed_struct& data, int num, ...);
    void cnt_create(string originator_string, int num, ...);
    void cnt_retrieve();
    bool cnt_validate(const parse_json::parsed_struct& data, int num, ...);

    void cin_create(string originator, string CIN_NAME, string payload, int num, ...);
    void cin_update(const parse_json::parsed_struct& data, int num, ...);
    http_response cin_retrieve_la(string originator, int num, ...);

    bool cnt_validate(const string& CNT_NAME);

    void discovery_retrieve(string originator, int num, ...);
    void result_content_retrieve(string originator, int num, ...);

private:
    int numbering = 1;
    const std::string server_url;
    std::string URL_TO_CSE;
    std::string URL_TO_AE;
    std::string URL_TO_CNT;
};