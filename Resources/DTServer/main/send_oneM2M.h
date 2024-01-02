#pragma once
#include <string>
#include "parse_json.h"

class send_oneM2M {
public:
    send_oneM2M(parse_json::parsed_struct parsed_struct);

    void grp_create(const parse_json::parsed_struct& data);

    void acp_create(const parse_json::parsed_struct& data);

    void ae_create(const parse_json::parsed_struct& data);
    void ae_retrieve();

    void cnt_create(const parse_json::parsed_struct& data, int num, ...);
    void cnt_retrieve();

    void cin_create(string originator, string CIN_NAME, string payload, int num, ...);
    void cin_update(const parse_json::parsed_struct& data, int num, ...);

    bool cnt_validate(const string& CNT_NAME);
    bool acp_validate();

private:
    int numbering = 1;
    const std::string server_url;
    std::string URL_TO_CSE;
    std::string URL_TO_AE;
    std::string URL_TO_CNT;
};