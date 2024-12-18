#include "./websocket.hpp"

namespace atlas {

    websocket* get_new_websocket(server* server_instance) {
        for (uint16_t i = 0; i < server_instance->max_num_websockets; i++) {
            websocket* ws = server_instance->websockets + i;
            if (ws->state == WS_STATE_NOEXIST) {
                return ws;
            }
        }

        return NULL;
    }

    std::string encode_websocket_key(const std::string& key) {
        std::string concat = key;
        concat += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        constexpr uint32_t SHA1_DIGEST_SIZE = 20;

        uint8_t hash[SHA1_DIGEST_SIZE];
        sha1::digest(hash, (uint8_t*)concat.data(), concat.length());

        return base64::encode(hash, SHA1_DIGEST_SIZE);
    }

    void reset_frame_parse_state(websocket& ws) {
        ws.frame_parse_state = {
            .frame_read_offset = 0,

            // will be overwritten as the frame reading process progresses:
            .fin = false,
            .masked = false,
            .opcode = WS_OPCODE_CONTINUATION,
            .payload_length = 0,
            .payload_length_type = WS_PAYLOAD_LENGTH_INVALID,
            .reading_payload = false,
            .payload_bytes_read = 0,
        };
    }

    void reset_message_parse_state(websocket& ws) {
        ws.incoming_message.type = WS_MESSAGE_TEXT;
        ws.incoming_message.fragmented_transfer = false;
        ws.incoming_message.fragment_count = 0;

        ws.incoming_message.data.clear();
        ws.incoming_message.data.shrink_to_fit();
    }

    bool upgrade_to_websocket(http_request& req, http_response& res, std::function<void(websocket&)> onopen) {
        std::unique_lock<std::mutex> lock(req.session->buffers.mtx);

        if (onopen == nullptr)
            return false;

        if (req.headers["upgrade"] != "websocket" || req.headers["connection"] != "Upgrade") {
            return false;
        }

        const std::string& sec_websocket_key = req.headers["sec-websocket-key"];
        const std::string& sec_websocket_version = req.headers["sec-websocket-version"];

        if (sec_websocket_key.empty() || sec_websocket_version.empty()) {
            return false;
        }

        std::string sec_websocket_accept = encode_websocket_key(sec_websocket_key);

        internal_handler_write_head(res, HTTP_SWITCHING_PROTOCOLS, {
            {"upgrade", "websocket"},
            {"connection", "Upgrade"},
            {"sec-websocket-accept", sec_websocket_accept}
            });

        websocket& ws = *get_new_websocket(req.session->server_instance);

        ws.session = req.session;
        ws.state = WS_STATE_OPEN;
        ws.session->session_status = HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET;
        ws.session->operation.client_rx_buffer_limit = 0;
        ws.session->upgraded_proto_data = &ws;

        ws.onclose = nullptr;
        ws.onmessage = nullptr;

        ws.incoming_message.data = "";
        ws.incoming_message.type = WS_MESSAGE_TEXT; // will be overwritten

        reset_frame_parse_state(ws);
        reset_message_parse_state(ws);

        lock.unlock();

        onopen(ws);

        return true;
    }

    void websocket_close_cleanup(websocket& ws) {
        if (ws.onclose != nullptr) {
            ws.session->buffers.mtx.unlock(); // Unlock before calling the handler
            try {
                ws.onclose(ws);
            }
            catch (...) {
                ws.session->buffers.mtx.lock();
                throw;  // Re-lock before propagating exceptions
            }
            ws.session->buffers.mtx.lock();
        }

        reset_frame_parse_state(ws);
        reset_message_parse_state(ws);

        ws.state = WS_STATE_NOEXIST;
        ws.session->session_status = HTTP_CONNECTION_CLOSED;
        ws.session->upgraded_proto_data = nullptr;
    }

    void process_incoming_message(websocket& ws) {
        switch (ws.incoming_message.type) {
        case WS_MESSAGE_TEXT:
        case WS_MESSAGE_BINARY: {
            if (ws.onmessage != nullptr) {
                ws.session->buffers.mtx.unlock(); // Unlock before calling the handler
                try {
                    ws.onmessage(ws, ws.incoming_message);
                }
                catch (...) {
                    ws.session->buffers.mtx.lock();
                    throw;  // Re-lock before propagating exceptions
                }
                ws.session->buffers.mtx.lock();
            }
            break;
        }

        case WS_MESSAGE_CLOSE: {
            close_connection(*ws.session);
            break;
        }

        case WS_MESSAGE_PING: {
            websocket_message pong_message = {
                .type = WS_MESSAGE_PING,
                .data = ws.incoming_message.data
            };

            std::cout << "Received ping, sending pong\n";

            send(ws, pong_message);
            break;
        }

        case WS_MESSAGE_PONG: {
            // do nothing

            std::cout << "Received pong\n";

            break;
        }

        default:
            break;
        }

    }

    void parse_frame_segment(websocket& ws, uint8_t* buffer, uint32_t buffer_size) {
        for (size_t i = 0; i < buffer_size; i++) {
            switch (ws.frame_parse_state.frame_read_offset) {
            case 0: {
                ws.frame_parse_state.fin = (buffer[i] & WS_BYTE0_MASK_FIN) != 0;
                ws.frame_parse_state.opcode = (websocket_frame_opcode)(buffer[i] & WS_BYTE0_MASK_OPCODE);
                ws.frame_parse_state.frame_read_offset++;

                if (ws.incoming_message.fragment_count == 0) {
                    ws.incoming_message.fragmented_transfer = ws.frame_parse_state.fin == false;
                    ws.incoming_message.type = (websocket_message_type)ws.frame_parse_state.opcode;
                }
                else if (ws.incoming_message.fragmented_transfer && ws.frame_parse_state.opcode != WS_OPCODE_CONTINUATION) {
                    std::cout << "ERROR: Expected a continuation frame, got: " << (int64_t)ws.frame_parse_state.opcode << "\n";
                    close_connection(*ws.session);
                    return;
                }

                ws.incoming_message.fragment_count++;

                continue;
            }
            case 1: {
                ws.frame_parse_state.masked = (buffer[i] & WS_BYTE1_MASK_MASKED) != 0;
                const uint8_t payload_length_det = buffer[i] & WS_BYTE1_MASK_PAYLOAD_LENGTH_DET;

                switch (payload_length_det) {
                case WS_PAYLOAD_LENGTH_DET_16BIT:
                    ws.frame_parse_state.payload_length_type = WS_PAYLOAD_LENGTH_16BIT;
                    ws.frame_parse_state.payload_length = 0;
                    break;
                case WS_PAYLOAD_LENGTH_DET_64BIT:
                    ws.frame_parse_state.payload_length_type = WS_PAYLOAD_LENGTH_64BIT;
                    ws.frame_parse_state.payload_length = 0;
                    break;
                default:
                    ws.frame_parse_state.payload_length_type = WS_PAYLOAD_LENGTH_7BIT;
                    ws.frame_parse_state.payload_length = payload_length_det;
                    break;
                }

                ws.frame_parse_state.frame_read_offset++;
                continue;
            }
            }

            if (ws.frame_parse_state.reading_payload) {

                if (ws.frame_parse_state.masked) {
                    buffer[i] ^= ws.frame_parse_state.masking_key[ws.frame_parse_state.payload_bytes_read % 4];
                }

                ws.incoming_message.data += buffer[i];
                ws.frame_parse_state.payload_bytes_read++;

                if (ws.frame_parse_state.payload_bytes_read >= ws.frame_parse_state.payload_length) {
                    if (ws.frame_parse_state.fin) {
                        process_incoming_message(ws);
                        reset_message_parse_state(ws);
                    }

                    reset_frame_parse_state(ws);

                    continue;
                }

                ws.frame_parse_state.frame_read_offset++;
                continue;
            }

            if (ws.frame_parse_state.payload_length_type == WS_PAYLOAD_LENGTH_7BIT) {
                if (ws.frame_parse_state.masked && ws.frame_parse_state.frame_read_offset < 2 + 4) {
                    ws.frame_parse_state.masking_key[ws.frame_parse_state.frame_read_offset - 2] = buffer[i];
                    ws.frame_parse_state.frame_read_offset++;
                    continue;
                }

                ws.frame_parse_state.reading_payload = true;
                ws.incoming_message.data.reserve(ws.frame_parse_state.payload_length); // reserve the space for the incoming message to avoid many reallocations
                // ws.frame_parse_state.frame_read_offset++; // no need to increment here as we incremented after each reading of the masking key
                i--; // reprocess the current byte
                continue;
            }

            if (ws.frame_parse_state.payload_length_type == WS_PAYLOAD_LENGTH_16BIT) {


                if (ws.frame_parse_state.frame_read_offset < 2 + 2) {
                    ws.frame_parse_state.payload_length |= buffer[i] << (8 * (2 + 1 - ws.frame_parse_state.frame_read_offset));
                    ws.frame_parse_state.frame_read_offset++;
                    continue;
                }

                if (ws.frame_parse_state.masked && ws.frame_parse_state.frame_read_offset < 2 + 2 + 4) {
                    ws.frame_parse_state.masking_key[ws.frame_parse_state.frame_read_offset - (2 + 2)] = buffer[i];
                    ws.frame_parse_state.frame_read_offset++;
                    continue;
                }

                ws.frame_parse_state.reading_payload = true;
                ws.incoming_message.data.reserve(ws.frame_parse_state.payload_length); // reserve the space for the incoming message to avoid many reallocations

                // ws.frame_parse_state.frame_read_offset++; // no need to increment here as we incremented after each reading of the masking key
                i--; // reprocess the current byte
                continue;
            }

            if (ws.frame_parse_state.payload_length_type == WS_PAYLOAD_LENGTH_64BIT) {
                if (ws.frame_parse_state.frame_read_offset < 2 + 8) {
                    ws.frame_parse_state.payload_length |= buffer[i] << (8 * (2 + 7 - ws.frame_parse_state.frame_read_offset));
                    ws.frame_parse_state.frame_read_offset++;
                    continue;
                }

                if (ws.frame_parse_state.masked && ws.frame_parse_state.frame_read_offset < 2 + 8 + 4) {
                    ws.frame_parse_state.masking_key[ws.frame_parse_state.frame_read_offset - (2 + 8)] = buffer[i];
                    ws.frame_parse_state.frame_read_offset++;
                    continue;
                }

                if (ws.frame_parse_state.payload_length > conf::MAX_WEBSOCKET_PAYLOAD_SIZE) {
                    std::cout << "ERROR: Payload too large\n";
                    close_connection(*ws.session);
                    return;
                }

                ws.frame_parse_state.reading_payload = true;
                ws.incoming_message.data.reserve(ws.frame_parse_state.payload_length); // reserve the space for the incoming message to avoid many reallocations

                // ws.frame_parse_state.frame_read_offset++; // no need to increment here as we incremented after each reading of the masking key
                i--; // reprocess the current byte
                continue;
            }

            std::cout << "ERROR: Invalid frame parse state\n";
            close_connection(*ws.session);


        }
    }

    void websocket_send_tick(http_session& sess) {
        if (sess.session_status != HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET)
            return;

        constexpr size_t MAX_SINGLE_SEND_SIZE = 10000000;

        server* server_instance = sess.server_instance;
        uint32_t send_size = sess.buffers.client_tx_buffer.length() - sess.operation.sent_bytes;
        if (send_size > MAX_SINGLE_SEND_SIZE)
            send_size = MAX_SINGLE_SEND_SIZE;

        int32_t bytes_sent = ::write(sess.client_fd, sess.buffers.client_tx_buffer.data() + sess.operation.sent_bytes, send_size);
        if (bytes_sent < 0 && errno != EWOULDBLOCK) {
            // std::cout << "ERROR! CLOSED on id=" << sess.operation.index << " errno=" << strerror(errno) << ", status=" << sess.session_status << "\n";
            close_connection(sess); // logic for disposing of websocket resources?
            return;
        }

        if (bytes_sent <= 0)
            return;

        sess.operation.last_io_t = server_instance->curr_rtime;
        sess.operation.sent_bytes += bytes_sent;
        sess.operation._DEBUG_TOTAL_BYTES_SENT += bytes_sent;

        if (sess.operation.sent_bytes >= sess.buffers.client_tx_buffer.length()) {
            // clear the transmit buffer
            sess.buffers.client_tx_buffer.clear();
            sess.buffers.client_tx_buffer.shrink_to_fit();
            sess.operation.sent_bytes = 0;
        }
    }

    void websocket_receive_tick(http_session& sess) {
        if (sess.session_status != HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET)
            return;

        server* server_instance = sess.server_instance;

        int32_t n = read(sess.client_fd, server_instance->rx_buffer, conf::SERVER_RX_BUFF_SIZE);
        if (n < 0)
            return;

        if (n == 0) { // orderly TCP disconnect
            close_connection(sess); // logic for disposing of websocket resources?
        }

        sess.operation.last_io_t = server_instance->curr_rtime;
        sess.operation.received_bytes = 0;

        parse_frame_segment(*(websocket*)sess.upgraded_proto_data, (uint8_t*)server_instance->rx_buffer, n);

    }

    void websocket_tick(http_session& sess) {
        websocket_receive_tick(sess);
        websocket_send_tick(sess);
    }

    void internal_handler_ws_send(websocket& ws, const websocket_message& message) {
        websocket_payload_length_type payload_length_type;
        uint64_t payload_length = message.data.length();
        uint8_t payload_length_det = 0;
        uint64_t header_length = 0;

        if (payload_length <= 125) {
            payload_length_type = WS_PAYLOAD_LENGTH_7BIT;
            payload_length_det = payload_length;
        }
        else if (payload_length <= 65535) {
            payload_length_type = WS_PAYLOAD_LENGTH_16BIT;
            payload_length_det = WS_PAYLOAD_LENGTH_DET_16BIT;
        }
        else {
            payload_length_type = WS_PAYLOAD_LENGTH_64BIT;
            payload_length_det = WS_PAYLOAD_LENGTH_DET_64BIT;
        }

        const bool masked = false; // we don't mask outgoing messages
        const bool fin = true; // we don't need to send fragmented messages

        uint8_t header_buffer[16]; // we won't normally use all of this space

        header_buffer[0] = (fin ? WS_BYTE0_MASK_FIN : 0) | ((uint8_t)message.type & WS_BYTE0_MASK_OPCODE);
        header_buffer[1] = (masked ? WS_BYTE1_MASK_MASKED : 0) | payload_length_det;
        header_length += 2;

        if (payload_length_type == WS_PAYLOAD_LENGTH_16BIT) {
            header_buffer[2] = (payload_length >> 8);
            header_buffer[3] = payload_length;
            header_length += 2;
        }
        else if (payload_length_type == WS_PAYLOAD_LENGTH_64BIT) {
            header_buffer[2] = (payload_length >> 56);
            header_buffer[3] = (payload_length >> 48);
            header_buffer[4] = (payload_length >> 40);
            header_buffer[5] = (payload_length >> 32);
            header_buffer[6] = (payload_length >> 24);
            header_buffer[7] = (payload_length >> 16);
            header_buffer[8] = (payload_length >> 8);
            header_buffer[9] = payload_length;
            header_length += 8;
        }

        if (masked) {
            // we don't mask outgoing messages
            perror("ERROR: Attempted to send a masked websocket message which is not supported\n");
        }

        std::string_view header_str((char*)header_buffer, header_length);

        http_session& sess = *ws.session;

        sess.buffers.client_tx_buffer += header_str;
        sess.buffers.client_tx_buffer += message.data;

        update_event(sess, true, true);
    }


    void send(websocket& ws, const websocket_message& message) {
        std::unique_lock<std::mutex> lock(ws.session->buffers.mtx);
        return internal_handler_ws_send(ws, message);
    }

}