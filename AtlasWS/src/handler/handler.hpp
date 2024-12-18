#pragma once

#include "../type.hpp"
#include "../handler/connection.hpp"
#include "../handler/receive.hpp"
#include "../handler/send.hpp"
#include "../time/time.hpp"
#include "../conf/conf.hpp"
#include "../socket/socket.hpp"
#include "../websocket/websocket.hpp"

namespace atlas {

    void socket_tick_all(server* server, http_session* socket_list_loc);
    void socket_tick_one(server* server, http_session& sess);
    void server_tick(server* server);
    void update_event(http_session& sess, bool want_read, bool want_write);
    
}