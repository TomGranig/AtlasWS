#include "./http-transfer.hpp"

namespace atlas {


    void set_header_if_unset(std::unordered_map<std::string, std::string>& headers, std::string header_name, std::string header_value) {
        if (headers[header_name].empty())
            headers[header_name] = header_value;
    }

    void end_normal_transfer(http_response& res) {
        // Do nothing 
    }

    void write_normal_transfer(http_response& res, std::string_view string) {
        res.session->session_status = HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE; // TODO: is this even neccessary?
        buffer_str(res, string);
    }

    void internal_handler_end(http_response& res) {
        if (res.session->session_status == HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER)
            return;

        if (res.session->session_status == HTTP_CONNECTION_ACCEPTED) {
            write_head(res, 200, {}); // No content was sent
        }

        res.session->session_status = HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER;

        switch (res.session->operation.transfer_method) {
        case NWS_TRANSFER_CHUNKED: // e.g. Content-Length not set by programmer in respond(status, headers)
            end_chunked_transfer(res);
            break;

        case NWS_TRANSFER_NORMAL: // e.g. Content-Length is set by programmer in respond(status, headers)
            end_normal_transfer(res);
            break;
        default:
            assert(false);
        }
    }


    void internal_handler_write_head(http_response& res, uint16_t status, const std::unordered_map<std::string, std::string>& headers) {

        if (res.session->session_status != HTTP_CONNECTION_ACCEPTED && res.session->session_status != HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY) {
            // std::cout << "Error: duplicate write_head() or write_head() after write()\n";
            std::cout << "Cant write head: session_status = " << res.session->session_status << "\n";
            return;
        }

        res.session->operation.send_size = 0;
        res.headers = headers;


        std::stringstream oss;
        oss << conf::PROTO_IDENTIFIER << " " << get_status_str(status) << "\r\n";

        buffer_str(res, oss.str());

        res.headers["server"] = "AtlasWS/0.1";
        set_header_if_unset(res.headers, "connection", "close");
        set_header_if_unset(res.headers, "content-type", "text/html");

        if (get_header(res.headers, "connection") == "keep-alive") {
            res.session->operation.connection_lifetime = NWS_CONNECTION_KEEPALIVE;

            if (get_header(res.headers, "content-length").empty()) // when there is no content length header, chunked encoding must be used
                set_header_if_unset(res.headers, "transfer-encoding", "chunked");
        }
        else {
            res.session->operation.connection_lifetime = NWS_CONNECTION_CLOSE;
        }

        if (get_header(res.headers, "transfer-encoding") == "chunked") {
            res.session->operation.transfer_method = NWS_TRANSFER_CHUNKED;
        }
        else {
            res.session->operation.transfer_method = NWS_TRANSFER_NORMAL;
        }

        for (const auto& header : res.headers) {
            oss.str("");
            oss << header.first << ": " << header.second << "\r\n";
            buffer_str(res, oss.str());
        }


        buffer_str(res, "\r\n");

        res.session->session_status = HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE;
    }

    void internal_handler_write(http_response& res, std::string_view string) {
        if (res.session->session_status != HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE && res.session->session_status != HTTP_CONNECTION_CLOSED
            && res.session->session_status != HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER) {
            internal_handler_write_head(res, 200, {});
        }

        // ensure we dont write to a closed/closing connection
        if (res.session->session_status == HTTP_CONNECTION_CLOSED || res.session->session_status == HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER) {
            std::cout << "Cant write to closed/closing connection: session_status = " << res.session->session_status << "\n";
            return;
        }

        switch (res.session->operation.transfer_method) {
        case NWS_TRANSFER_CHUNKED: {
            write_chunked_transfer(res, string);
            break;
        }
        case NWS_TRANSFER_NORMAL: {
            write_normal_transfer(res, string);
            break;
        }
        default:
            assert(false);
        }
    }







    void end(http_response& res) {
        std::unique_lock<std::mutex> lock(res.session->buffers.mtx);
        internal_handler_end(res);
    }

    void write(http_response& res, std::string_view string) {
        std::unique_lock<std::mutex> lock(res.session->buffers.mtx);
        internal_handler_write(res, string);
    }

    void write(http_response& res, std::vector<uint8_t> vec) {
        std::string_view sv(reinterpret_cast<char*>(vec.data()), vec.size());
        write(res, sv);
    }

    void write(http_response& res, uint8_t* buffer, size_t buffer_size) {
        std::string_view sv(reinterpret_cast<char*>(buffer), buffer_size);
        write(res, sv);
    }

    void write_file_contents_sync(http_response& res, std::string file_path) {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            error_page(res.session->req, HTTP_NOT_FOUND, "File not found");
            return;
        }

        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        uint8_t* buffer = new uint8_t[file_size];
        file.read(reinterpret_cast<char*>(buffer), file_size);
        file.close();

        write(res, buffer, file_size);

        delete[] buffer;
    }


    void write_head(http_response& res, uint16_t status, const std::unordered_map<std::string, std::string>& headers) {
        std::unique_lock<std::mutex> lock(res.session->buffers.mtx);
        internal_handler_write_head(res, status, headers);
    }



}
