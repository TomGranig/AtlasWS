#include "./receive.hpp"

namespace atlas {

    const char END_OF_HEADERS_SEQ[4] = {'\r', '\n', '\r', '\n'};


    void receive_tick(http_session& sess) {
        server* server = sess.server;

        int32_t n = read(sess.client_fd, server->rx_buffer, conf::SERVER_RX_BUFF_SIZE);
        if (n < 0)
            return;


        if (n == 0) { // orderly disconnect
            close_connection(sess);
            return;
        }

        sess.operation.last_io_t = server->curr_rtime;
        sess.operation.received_bytes += n;

        if (sess.operation.received_bytes > sess.operation.client_rx_buffer_limit + conf::SERVER_RX_BUFF_SIZE) {
            close_connection(sess);
        }

        uint32_t append_size = std::min(n, (int32_t)conf::SERVER_RX_BUFF_SIZE);

        if (append_size == 0)
            return;

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

            // look for the end of the headers sequence (\r\n\r\n)
            for (uint32_t i = 0; i < append_size; i++) {
                const char new_char = server->rx_buffer[i];

                if (new_char == END_OF_HEADERS_SEQ[sess.http_parse_state.end_of_headers_seq_ctr])
                    sess.http_parse_state.end_of_headers_seq_ctr++;
                else
                    sess.http_parse_state.end_of_headers_seq_ctr = 0;
                

                if (sess.http_parse_state.end_of_headers_seq_ctr == sizeof(END_OF_HEADERS_SEQ)) {
                    sess.req.body_transfer.body_start_index = i + 1;
                    handle_request(sess.req);

                    if (sess.req.body_transfer.has_body) {
                        const uint32_t new_body_content_size = append_size - i - 1;
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