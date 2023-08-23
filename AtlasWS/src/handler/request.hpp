#pragma once

#include "../type.hpp"
#include "../http-parse/http-parse.hpp"

namespace atlas {

    void handle_request(http_request& req);
    void handle_request_payload(http_request& req);
}