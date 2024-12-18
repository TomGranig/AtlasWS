#pragma once

#include "../type.hpp"
#include "../http-transfer/http-transfer.hpp"

namespace atlas {

    void serve_range(http_request* req, uint64_t content_length);

}