/*

    AtlasWS - A powerful web framework
    Copyright (C) Tom Granig 2023. All rights reserved.

*/


#pragma once

#include "../type.hpp"
#include "../websocket/websocket.hpp"

namespace atlas {

    void reset_http_session_state(http_session& sess);
    void reset_connection(http_session& sess);
    void close_connection(http_session& sess);
    

}