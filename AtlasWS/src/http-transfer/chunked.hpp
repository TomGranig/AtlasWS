#pragma once

#include "../type.hpp"
#include "../handler/send.hpp"

namespace atlas {
    
    void write_chunked_transfer(http_response& res, std::string_view string);
    void end_chunked_transfer(http_response& res);

}