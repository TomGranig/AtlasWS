#pragma once

#include "../type.hpp"
#include "../handler/connection.hpp"
#include "../handler/receive.hpp"
#include "../handler/send.hpp"
#include "../time/time.hpp"
#include "../conf/conf.hpp"
#include "../socket/socket.hpp"

namespace atlas {

    void socket_tick(server* server, http_request* socket_list_loc);
    void server_tick(server* server);
    
}