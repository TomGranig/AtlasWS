#include "./receive.hpp"

namespace atlas {

    int64_t max(int64_t a, int64_t b) { // TODO: put in sensible location
        return a > b ? a : b;
    }

    int64_t min(int64_t a, int64_t b) { // TODO: put in sensible location
        return a > b ? b : a;
    }

    void receive_tick(http_session& sess) {
        server* server = sess.server;

        int32_t n = read(sess.client_fd, server->rx_buffer, SERVER_RX_BUFF_SIZE);
        if (n < 0)
            return;


        if (n == 0) { // orderly disconnect
            // std::cout << "Received unexpected orderly TCP disconnect\n";
            close_connection(sess);
        }

        sess.operation.last_io_t = server->curr_rtime;
        sess.operation.received_bytes += n;

        if (sess.operation.received_bytes > sess.operation.client_rx_buffer_limit + SERVER_RX_BUFF_SIZE) {
            // std::cout << "Error: Max RX Buffer Size Exceeded\n";
            close_connection(sess);
        }

        uint32_t append_size = min(n, SERVER_RX_BUFF_SIZE);

        sess.buffers.client_rx_buffer += std::string_view(server->rx_buffer, append_size);

        if (sess.req.parsed_headers) {
            if (!sess.req.body_transfer.has_body)
                return;

            sess.req.body_transfer.received_content_length += append_size;

            if (!sess.req.body_transfer.has_specified_content_length)
                return;

            // have to deal with chunked encoding too in the future, where content length wont be specified in the headers

            if (sess.req.body_transfer.received_content_length >= sess.req.body_transfer.content_length)
                handle_request_payload(sess.req);

        }
        else if (sess.session_status == HTTP_CONNECTION_ACCEPTED) {
            for (uint32_t i = max(0, sess.buffers.client_rx_buffer.length() - n - 3); i < sess.buffers.client_rx_buffer.length(); i++) {

                if (sess.buffers.client_rx_buffer.substr(i, 4) == "\r\n\r\n") { // TODO: optimize \r\n pattern detection
                    sess.req.body_transfer.body_start_index = i + 4;
                    handle_request(sess.req);

                    if (sess.req.body_transfer.has_body) {
                        const uint32_t new_body_content_size = sess.buffers.client_rx_buffer.length() - i - 4;
                        sess.req.body_transfer.received_content_length += new_body_content_size;
                        if (new_body_content_size > 0) {
                        }

                        if (sess.req.body_transfer.received_content_length >= sess.req.body_transfer.content_length)
                            handle_request_payload(sess.req);
                    }

                    break;
                }

            }
        }
    }

}