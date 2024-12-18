#pragma once

#include "../type.hpp"
#include "../conf/conf.hpp"
#include "../error/error-message.hpp"

namespace atlas {

    http_parse_status parse_http_headers(http_request& req);
    void parse_req_url_params(http_request& req);
    void parse_req_post_params(http_request& req);
    void parse_cookie_header(http_request& req);
    std::string_view get_header(std::unordered_map<std::string, std::string>& headers, const std::string& header_name);

}