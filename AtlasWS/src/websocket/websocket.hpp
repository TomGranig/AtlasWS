#pragma once


#include "../type.hpp"
#include "../handler/connection.hpp"
#include "../handler/receive.hpp"
#include "../handler/send.hpp"
#include "../time/time.hpp"
#include "../conf/conf.hpp"
#include "../socket/socket.hpp"
#include "../util/util.hpp" 
#include <algorithm>

#include "sha1.hpp"

namespace atlas {
    bool upgrade_to_websocket(http_request& req, http_response& res, std::function<void(websocket&)> onopen);
    void send(websocket& ws, const websocket_message& message);

    void websocket_tick(http_session& sess);
    void websocket_close_cleanup(websocket& ws);
}