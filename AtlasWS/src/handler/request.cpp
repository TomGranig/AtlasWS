#include "./request.hpp"

namespace atlas {


    void begin_response(http_request& req) {
        // Manually manage locking for the response handler
        req.session->buffers.mtx.unlock(); // Unlock before calling the handler
        try {
            req.session->server_instance->onrequest(req, req.session->res);
        }
        catch (...) {
            req.session->buffers.mtx.lock();
            throw;  // Re-lock before propagating exceptions
        }
        req.session->buffers.mtx.lock();

        // Verify if a response was initiated; if not, show an error page
        if (req.require_immediate_response && req.session->session_status != HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE) {
            error_page(req, HTTP_INTERNAL_SERVER_ERROR, "Not handled");
        }
    }

    void handle_request(http_request& req) {
        http_parse_status status = parse_http_headers(req);
        req.parsed_headers = true;

        switch (status) {
        case NWS_PARSE_FAIL_REQ_METHOD:
            // std::cout << "Error: HTTP parse req method fail\n";
            error_page(req, HTTP_BAD_REQUEST, "Could not parse request method");
            return;
        case NWS_PARSE_FAIL_REQ_URL:
            // std::cout << "Error: HTTP parse req URL fail\n";
            error_page(req, HTTP_URI_TOO_LONG, "Could not parse request URL");
            return;
        case NWS_PARSE_FAIL_REQ_HTTP_VER:
            // std::cout << "Error: HTTP parse req version fail\n";
            error_page(req, HTTP_BAD_REQUEST, "Could not parse request HTTP version");
            return;
        case NWS_PARSE_FAIL_REQ_HEADER:
            // std::cout << "Error: HTTP parse header fail! STARTBUFFER[n]:\n";
            error_page(req, HTTP_BAD_REQUEST, "Could not parse request headers");
            return;

        case NWS_PARSE_OK:
            // std::cout << "Note: HTTP parse OK\n";
            break;
        }

        parse_req_url_params(req);
        parse_cookie_header(req);

        if (req.method == "POST") {
            req.session->session_status = HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY;
            req.body_transfer.has_body = true;
            req.body_transfer.received_content_length = 0;

            std::string& content_length_header = req.headers["content-length"];
            if (!content_length_header.empty()) {
                req.body_transfer.has_specified_content_length = true;
                req.body_transfer.content_length = std::stoi(content_length_header, nullptr);
            }

            if (!req.headers["transfer-encoding"].empty()) {
                error_page(req, HTTP_NOT_IMPLEMENTED, "Transfer-Encoding of request bodies are not yet implemented");
                return;
            }

            req.body_transfer.method = NWS_TRANSFER_NORMAL;


        }
        else {
            req.body_transfer.has_body = false;
        }

        if (req.session->session_status != HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY) { // not waiting for the request body
            // std::cout << "begin response without body\n";
            begin_response(req);
        }
    }

    void handle_request_payload(http_request& req) {
        const uint32_t body_len = req.body_transfer.received_content_length;
        req.body = static_cast<std::string_view>(req.session->buffers.client_rx_buffer).substr(req.body_transfer.body_start_index, body_len);

        std::string_view content_type = get_header(req.headers, "content-type");

        if (content_type == "application/x-www-form-urlencoded") {
            parse_req_post_params(req);
        }

        begin_response(req);
    }

}