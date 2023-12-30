#pragma once
#include <string>
#include "parse_json.h"

#define acop_all 63

class send_oneM2M {
public:
    send_oneM2M();
    send_oneM2M(parse_json::parsed_struct parsed_struct);

    void grp_create(const parse_json::parsed_struct& data);

    void acp_create(int num, ...);
    void acp_update(const parse_json::parsed_struct& data, vector<string> ACP_NAMES, int num, ...);
    bool acp_validate(int num, ...);

    void ae_create(const parse_json::parsed_struct& data);
    void ae_retrieve();
    bool ae_validate(const parse_json::parsed_struct& data, int num, ...);

    void cnt_create(const parse_json::parsed_struct& data, int num, ...);
    void cnt_retrieve();
    bool cnt_validate(const parse_json::parsed_struct& data, int num, ...);

    void cin_create(string originator, string CIN_NAME, string payload, int num, ...);
    void cin_update(const parse_json::parsed_struct& data, int num, ...);

    bool cnt_validate(const string& CNT_NAME);


private:
    int numbering = 1;
    const std::string server_url;
    std::string URL_TO_CSE;
    std::string URL_TO_AE;
    std::string URL_TO_CNT;
};