#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "../http-transfer/http-transfer.hpp"

namespace atlas {

    void sv_error(const char* err);
    void error_page(http_request& req, uint16_t status_code, std::string error_description);
    void error_page(http_response& res, uint16_t status_code, std::string error_description);

}