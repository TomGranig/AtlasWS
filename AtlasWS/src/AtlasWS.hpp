/*

    AtlasWS - A powerful web framework
    Copyright (C) Tom Granig. All rights reserved.

*/

#pragma once


#include "handler/handler.hpp"
#include "time/time.hpp"
#include "error/error-message.hpp"
#include "http-transfer/http-transfer.hpp"
#include "socket/socket.hpp"
#include "type.hpp"
#include "file/servefiles.hpp"
#include "websocket/websocket.hpp"

namespace atlas {

    int64_t get_file_size(char* path);

    void begin(server& server, uint16_t port);

};