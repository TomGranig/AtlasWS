
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <thread>
#include <functional>
#include <cassert>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

namespace atlas {

    typedef struct http_request http_request;
    typedef struct http_response http_response;
    typedef struct http_session http_session;
    typedef struct server server;

    using request_handler = std::function<void(http_request&, http_response&)>;

    enum http_connection_status {
        HTTP_CONNECTION_CLOSED,
        HTTP_CONNECTION_ACCEPTED,
        HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY,
        HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE,
        HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER
    };

    enum transfer_method {
        NWS_TRANSFER_CHUNKED,
        NWS_TRANSFER_NORMAL,
    };

    enum connection_lifetime {
        NWS_CONNECTION_KEEPALIVE,
        NWS_CONNECTION_CLOSE
    };

    enum http_parse_status {
        NWS_PARSE_OK,
        NWS_PARSE_FAIL_REQ_METHOD,
        NWS_PARSE_FAIL_REQ_URL,
        NWS_PARSE_FAIL_REQ_HTTP_VER,
        NWS_PARSE_FAIL_REQ_HEADER,
    };

    struct file_range_service_task {
        bool active;
        bool started_transfer;
        uint64_t serve_offset;
        uint64_t serve_end_offset;
        FILE* file;
    };

    struct server {
        uint32_t min_tx_buffer_size;
        uint32_t min_rx_buffer_size;
        uint32_t max_in_buffer_size;

        http_session* sessions;
        uint32_t max_concurrent_requests;
        uint32_t current_requests;
        uint16_t port;

        char* rx_buffer;
        request_handler onrequest;
        int32_t sockfd, n;

        uint64_t curr_rtime;
    };

    struct http_request {
        http_session* session;
        std::unordered_map<std::string, std::string> headers;

        std::string_view body;
        std::string path;
        std::string url;
        std::unordered_map<std::string, std::string> query_param;
        std::unordered_map<std::string, std::string> post_param;
        std::unordered_map<std::string, std::string> cookies;
        std::string method;
        std::string version;

        bool parsed_headers;
        bool require_immediate_response;

        struct {
            bool has_body;
            bool has_specified_content_length;
            uint32_t content_length;
            uint32_t received_content_length;
            transfer_method method;
            uint32_t body_start_index;
        } body_transfer;

    };

    struct http_response {
        http_session* session;
        std::unordered_map<std::string, std::string> headers;
    };

    struct http_session {
        server* server;
        sockaddr_in remote_addr;
        int32_t client_fd;
        http_connection_status session_status;

        http_request req;
        http_response res;

        struct {
            std::string client_rx_buffer;
            std::string client_tx_buffer;
            std::string client_tx_chunk_buffer;
        } buffers;

        struct {
            transfer_method transfer_method;
            connection_lifetime connection_lifetime;

            bool has_known_body_size;
            uint64_t body_expected_size;

            uint32_t _DEBUG_TOTAL_BYTES_SENT;
            file_range_service_task file_serve_task;
            uint64_t last_io_t;
            uint32_t index;

            uint64_t received_bytes;
            uint64_t sent_bytes;
            uint64_t send_size;

            uint64_t client_rx_buffer_limit;
        } operation;
    };

}