/*

    AtlasWS - A powerful web framework
    Copyright (C) Tom Granig 2023. All rights reserved.

*/


#include "./connection.hpp"

namespace atlas {

    void close_connection(http_session& sess) {
        if (sess.session_status == HTTP_CONNECTION_CLOSED)
            return;

        close(sess.client_fd);

        if (sess.operation.file_serve_task.active) {
            sess.operation.file_serve_task.active = false;
            fclose(sess.operation.file_serve_task.file);
        }

        if (sess.session_status == HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET && sess.upgraded_proto_data != nullptr) {
            websocket& ws = *(websocket*)sess.upgraded_proto_data;
            websocket_close_cleanup(ws);
        }

        sess.session_status = HTTP_CONNECTION_CLOSED;
        sess.server->current_requests--;

    }

    void reset_http_session_state(http_session& sess) {
        if (sess.session_status == HTTP_CONNECTION_CLOSED)
            return;

        sess.session_status = HTTP_CONNECTION_ACCEPTED;

        sess.operation.received_bytes = 0;
        sess.operation.sent_bytes = 0;
        sess.operation.send_size = 0;

        sess.operation.has_known_body_size = false;
        sess.operation.body_expected_size = 0;

        sess.http_parse_state.end_of_headers_seq_ctr = 0;

        // sess.req.body.clear();

        sess.req.path.clear();
        sess.req.path.shrink_to_fit();
        sess.req.url.clear();
        sess.req.url.shrink_to_fit();
        sess.req.method.clear();
        sess.req.method.shrink_to_fit();

        sess.req.query_param.clear();
        sess.req.post_param.clear();
        sess.req.cookies.clear();
        sess.req.headers.clear();
        sess.res.headers.clear();

        sess.req.body_transfer.content_length = 0;
        sess.req.body_transfer.received_content_length = 0;
        sess.req.body_transfer.has_body = false;
        sess.req.body_transfer.has_specified_content_length = false;
        sess.req.body_transfer.method = NWS_TRANSFER_NORMAL;
        sess.req.parsed_headers = false;
        sess.req.require_immediate_response = true;

        sess.operation.connection_lifetime = NWS_CONNECTION_CLOSE;
        sess.operation.transfer_method = NWS_TRANSFER_NORMAL;

        sess.operation._DEBUG_TOTAL_BYTES_SENT = 0;

        sess.buffers.client_tx_buffer.clear();
        // sess.buffers.client_tx_buffer.shrink_to_fit();
        sess.buffers.client_rx_buffer.clear();
        // sess.buffers.client_rx_buffer.shrink_to_fit();

        sess.buffers.client_rx_buffer.reserve(conf::DEFAULT_INITIAL_SESS_RX_BUFFER_SIZE);
        sess.buffers.client_tx_buffer.reserve(conf::DEFAULT_INITIAL_SESS_TX_BUFFER_SIZE);


        sess.operation.file_serve_task.active = false;

    }

    void reset_connection(http_session& sess) {
        switch (sess.operation.connection_lifetime) {
        case NWS_CONNECTION_CLOSE:
            close_connection(sess);
            return;
        case NWS_CONNECTION_KEEPALIVE:
            reset_http_session_state(sess);

            if (sess.operation.file_serve_task.active) {
                sess.operation.file_serve_task.active = false;
                fclose(sess.operation.file_serve_task.file);
            }
            return;
        }
    }

}