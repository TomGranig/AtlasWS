#include "./chunked.hpp"

namespace atlas {
    void write_chunked_transfer(http_response& res, std::string_view string) {
        http_session& sess = *res.session;

        // append the string to the chunk buffer
        sess.buffers.client_tx_chunk_buffer += string;
        sess.session_status = HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE;

        if (sess.buffers.client_tx_chunk_buffer.length() > 4096) {
            // create an output string stream
            std::ostringstream oss;
            // write the chunk size in hexadecimal
            oss << std::hex << sess.buffers.client_tx_chunk_buffer.length() << "\r\n";
            // write the chunk data
            oss << sess.buffers.client_tx_chunk_buffer << "\r\n";
            // write the stream content to the buffer
            buffer_str(res, oss.str());

            sess.buffers.client_tx_chunk_buffer.clear();
        }
    }

    void end_chunked_transfer(http_response& res) {
        http_session& sess = *res.session;

        // if there is a remaining chunk, buffer it
        if (sess.buffers.client_tx_chunk_buffer.length() > 0) {

            // create an output string stream
            std::ostringstream oss;
            // write the chunk size in hexadecimal
            oss << std::hex << sess.buffers.client_tx_chunk_buffer.length() << "\r\n";
            // write the chunk data
            oss << sess.buffers.client_tx_chunk_buffer << "\r\n";

            // write the stream content to the buffer
            buffer_str(res, oss.str());
            sess.buffers.client_tx_chunk_buffer.clear();
        }

        // transfer the terminating chunk
        buffer_str(res, "0\r\n\r\n");
    }
}