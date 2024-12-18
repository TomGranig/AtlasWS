#include "./send.hpp"

namespace atlas {

    
    void buffer_str(http_response& res, std::string_view string) {
        res.session->buffers.client_tx_buffer += string;
    } 

    void send_tick(http_session& sess) {

        if (sess.session_status == HTTP_CONNECTION_CLOSED)
            return;

        constexpr size_t MAX_SINGLE_SEND_SIZE = 10000000;

        server* server_instance = sess.server_instance;
        uint32_t send_size = sess.buffers.client_tx_buffer.length() - sess.operation.sent_bytes;
        if (send_size > MAX_SINGLE_SEND_SIZE)
            send_size = MAX_SINGLE_SEND_SIZE;

        int32_t bytes_sent = ::write(sess.client_fd, sess.buffers.client_tx_buffer.data() + sess.operation.sent_bytes, send_size);
        if (bytes_sent < 0 && errno != EWOULDBLOCK) {
            // std::cout << "ERROR! CLOSED on id=" << sess.operation.index << " errno=" << strerror(errno) << ", status=" << sess.session_status << "\n";
            close_connection(sess);
            return;
        }

        if (bytes_sent <= 0)
            return;

        sess.operation.last_io_t = server_instance->curr_rtime;
        sess.operation.sent_bytes += bytes_sent;
        sess.operation._DEBUG_TOTAL_BYTES_SENT += bytes_sent;

        if ((sess.operation.sent_bytes >= sess.buffers.client_tx_buffer.length()) && sess.session_status == HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER) {
            reset_connection(sess);
            return;
        }
    }

}