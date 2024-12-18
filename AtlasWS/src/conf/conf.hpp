/*

    AtlasWS - A powerful web framework
    Copyright (C) Tom Granig 2023. All rights reserved.

*/

#pragma once

namespace atlas::conf {
    constexpr uint32_t DEFAULT_MAX_RX_BUFFER_SIZE = 65536;
    constexpr uint32_t DEFAULT_MAX_CONCURRENT_REQUESTS = 10000;
    constexpr uint32_t NO_IO_TIMEOUT_MILLIS = 15000;
    constexpr uint32_t FILE_SERVE_TASK_MAX_TX_BUFFER_SIZE = 50000;
    constexpr uint32_t DEFAULT_INITIAL_SESS_RX_BUFFER_SIZE = 4096;
    constexpr uint32_t DEFAULT_INITIAL_SESS_TX_BUFFER_SIZE = 4096;
    constexpr uint32_t SERVER_RX_BUFF_SIZE = 262144;

    constexpr uint32_t METHOD_NAME_SIZE_LIMIT = 50;
    constexpr uint32_t URL_SIZE_LIMIT = 8192;
    constexpr uint32_t HTTP_VER_NAME_SIZE_LIMIT = 10;
    constexpr uint32_t HEADER_NAME_SIZE_LIMIT = 256;
    constexpr uint32_t HEADER_VALUE_SIZE_LIMIT = 4096;
    constexpr uint32_t CONFIG_NUM_REQ_COOKIES = 50;


    constexpr uint32_t MAX_NUM_WEBSOCKETS = 5000;
    constexpr uint32_t MAX_WEBSOCKET_PAYLOAD_SIZE = 100000000;
    constexpr uint32_t MAX_WEBSOCKET_PAYLOAD_BUFFER_RESEERVE_SIZE = 131072;

    constexpr const char* PROTO_IDENTIFIER = "HTTP/1.1";
    constexpr const char* WEB_VERSION = "v0.2.0";
}


/*

    Platform checks

*/

static_assert(sizeof(char) == 1, "This server requires a platform with 1 byte char width");

