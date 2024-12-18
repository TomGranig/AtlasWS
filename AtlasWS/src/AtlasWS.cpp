/*

    AtlasWS - A powerful web framework
    Copyright (C) Tom Granig 2023. All rights reserved.

*/


#include "AtlasWS.hpp"

namespace atlas {

    int64_t get_file_size(char* path) {
        FILE* file = fopen(path, "r+");

        if (file == NULL)
            return -1;

        fseek(file, 0, SEEK_END);

        int64_t size = ftell(file);
        fclose(file);

        return size;
    }

    std::string to_hex_str(uint16_t number) {
        std::stringstream stream;
        stream << std::hex << number;
        return stream.str();
    }

    void begin(server& server, uint16_t port) {
        server.max_in_buffer_size = conf::DEFAULT_MAX_RX_BUFFER_SIZE;
        server.max_concurrent_requests = conf::DEFAULT_MAX_CONCURRENT_REQUESTS;
        server.max_num_websockets = conf::MAX_NUM_WEBSOCKETS;
        server.current_requests = 0;

        server.port = port;
        server.onrequest = [](auto& req, auto& res){
            write_head(res, HTTP_NOT_IMPLEMENTED, {{"content-type", "text/html"}});
            write(res, "<h1>Not implemented</h1>");
            end(res);
        };

        server.sockfd = create_listening_socket(server.port);
        server.rx_buffer = (char*)malloc(conf::SERVER_RX_BUFF_SIZE);

        server.sessions = new http_session[server.max_concurrent_requests];
        for (uint16_t i = 0; i < server.max_concurrent_requests; i++) {
            http_session* sess = server.sessions + i;
            sess->session_status = HTTP_CONNECTION_CLOSED;
        }

        server.websockets = new websocket[server.max_num_websockets];
        for (uint16_t i = 0; i < server.max_num_websockets; i++) {
            websocket* ws = server.websockets + i;
            ws->state = WS_STATE_NOEXIST;
        }

        server.ev_fd_timeout.tv_sec = 1;
        server.ev_fd_timeout.tv_nsec = 0;


        // pthread_t http_server_tick_thread;
        // pthread_create(&http_server_tick_thread, NULL, (void* (*)(void*))server_tick, &server);

        server.server_handler_thread = std::thread(server_tick, &server);
        


    }

}
