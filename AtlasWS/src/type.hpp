
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
#include <unordered_map>
#include <mutex>
#include <vector>
#include <filesystem>



#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif


namespace atlas {

    typedef struct http_request http_request;
    typedef struct http_response http_response;
    typedef struct http_session http_session;
    typedef struct server server;
    typedef struct websocket websocket;

    using request_handler = std::function<void(http_request&, http_response&)>;

    enum http_connection_status {
        HTTP_CONNECTION_CLOSED,
        HTTP_CONNECTION_ACCEPTED,
        HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY,
        HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE,
        HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER,
        HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET
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
        // uint32_t min_tx_buffer_size;
        // uint32_t min_rx_buffer_size;
        uint32_t max_in_buffer_size;

        // TODO: must use a free list & linked list for addition/removal of sessions and websockets rather than a linear table

        http_session* sessions;
        uint32_t max_concurrent_requests;
        uint32_t current_requests;
        uint16_t port;

        websocket* websockets;
        uint32_t max_num_websockets;
        uint32_t current_num_websockets;

        char* rx_buffer;
        request_handler onrequest;
        int32_t sockfd, n;

        uint64_t curr_rtime;

        int32_t ev_fd;
        struct timespec ev_fd_timeout;

        std::thread server_handler_thread;
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

        void *upgraded_proto_data;

        struct {
            std::string client_rx_buffer;
            std::string client_tx_buffer;
            std::string client_tx_chunk_buffer;
            std::mutex mtx;
        } buffers;

        struct {
            uint32_t end_of_headers_seq_ctr;
            
        } http_parse_state;

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


    enum websocket_state {
        WS_STATE_NOEXIST,
        WS_STATE_OPEN,
        WS_STATE_CLOSING,
        WS_STATE_CLOSED,
    };

    enum websocket_frame_opcode: uint8_t {
        WS_OPCODE_CONTINUATION = 0x0,
        WS_OPCODE_TEXT = 0x1,
        WS_OPCODE_BINARY = 0x2,
        WS_OPCODE_CLOSE = 0x8,
        WS_OPCODE_PING = 0x9,
        WS_OPCODE_PONG = 0xA
    };

    enum websocket_message_type: uint8_t {
        WS_MESSAGE_CONTINUATION = WS_OPCODE_CONTINUATION,
        WS_MESSAGE_TEXT = WS_OPCODE_TEXT,
        WS_MESSAGE_BINARY = WS_OPCODE_BINARY,
        WS_MESSAGE_CLOSE = WS_OPCODE_CLOSE,
        WS_MESSAGE_PING = WS_OPCODE_PING,
        WS_MESSAGE_PONG = WS_OPCODE_PONG
    };

    struct websocket_message {
        websocket_message_type type;
        std::string data;
        bool fragmented_transfer;
        uint64_t fragment_count;
    };

    enum websocket_payload_length_type {
        WS_PAYLOAD_LENGTH_7BIT,
        WS_PAYLOAD_LENGTH_16BIT,
        WS_PAYLOAD_LENGTH_64BIT,
        WS_PAYLOAD_LENGTH_INVALID
    };

    constexpr uint8_t WS_PAYLOAD_LENGTH_DET_16BIT = 126;
    constexpr uint8_t WS_PAYLOAD_LENGTH_DET_64BIT = 127;

    constexpr uint8_t WS_BYTE0_MASK_FIN = 0b10000000;
    constexpr uint8_t WS_BYTE0_MASK_OPCODE = 0b00001111;
    constexpr uint8_t WS_BYTE1_MASK_MASKED = 0b10000000;
    constexpr uint8_t WS_BYTE1_MASK_PAYLOAD_LENGTH_DET = 0b01111111;

    // Decoding Payload Length
    // To read the payload data, you must know when to stop reading. That's why the payload length is important to know. Unfortunately, this is somewhat complicated. To read it, follow these steps:

    // 1. Read bits 9-15 (inclusive) and interpret that as an unsigned integer. If it's 125 or less, then that's the length; you're done. If it's 126, go to step 2. If it's 127, go to step 3.
    // 2. Read the next 16 bits and interpret those as an unsigned integer. You're done.
    // 3. Read the next 64 bits and interpret those as an unsigned integer. (The most significant bit must be 0.) You're done.

    struct websocket_frame_parse_state {
        uint64_t frame_read_offset;
        bool fin;
        bool masked;
        websocket_frame_opcode opcode;
        uint64_t payload_length;
        websocket_payload_length_type payload_length_type;
        uint8_t masking_key[4];
        bool reading_payload;
        uint64_t payload_bytes_read;
    };

    struct websocket {
        http_session* session;
        websocket_state state;

        std::function<void(websocket&, websocket_message&)> onmessage;
        std::function<void(websocket&)> onclose;

        websocket_message incoming_message;

        websocket_frame_parse_state frame_parse_state;

    };

}