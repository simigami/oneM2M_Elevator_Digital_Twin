#pragma once
#include <cpprest/http_client.h>
#include "config.h"
#include "parse_json.h"

using namespace web::http;

class send_oneM2M {
public:
    send_oneM2M();
    send_oneM2M(Wparsed_struct parsed_struct);

    void set_new_building_uri(wstring building_name);
    void set_new_elevator_uri(wstring building_name);

    void grp_create(const Wparsed_struct& data);

    void acp_create_one_ACP(vector<wstring>& ACP_NAMES, int num, ...);

	void acp_update(vector<wstring> ACP_NAMES, int num, ...);
    int acp_validate(wstring ACP_NAME, int num, ...);

    vector<wstring> acp_retrieve(int num, ...);

    void ae_create(wstring AE_NAME);

    bool ae_validate(const Wparsed_struct& data, int num, ...);

    //void cnt_create(const parse_json::parsed_struct& data, int num, ...);
    void cnt_create(int num, ...);

    bool cnt_validate(int num, ...);

	void sub_create(map<wstring, wstring>* mapper, wstring originator_string, int num, ...);

    void cin_create(wstring CIN_NAME, wstring payload, int num, ...);
    void cin_update(const Wparsed_struct& data, int num, ...);
    http_response cin_retrieve_la(wstring originator, int num, ...);

    vector<double> retrieve_oneM2M_Energy_CIN(Wparsed_struct parseStruct);

    web::json::array discovery_retrieve(wstring originator, int discovery_type_num, int level, int num, ...);
    void result_content_retrieve(int num, ...);

private:
    int numbering = 1;
    const std::string server_url;

    std::wstring uri_to_ip;
    std::wstring uri_to_cse;
    std::wstring uri_to_dt_notification;

    std::wstring building_originator;

    std::wstring uri_to_building;
    std::wstring uri_to_elevator;

    std::wstring uri_only_cse_and_building;

	std::wstring nu_URL;
	std::wstring ae_poa_URL;
};