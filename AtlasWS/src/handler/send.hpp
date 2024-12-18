#pragma once

#include "../type.hpp"
#include "../handler/connection.hpp"

namespace atlas {

    void buffer_str(http_response& res, std::string_view string);
    void send_tick(http_session& sess);

}