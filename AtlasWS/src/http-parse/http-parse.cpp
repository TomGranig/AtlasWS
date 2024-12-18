#include "./http-parse.hpp"

namespace atlas {

    char ___lower(char c) {
        return (c <= 'Z' && c >= 'A') ? c - ('Z' - 'z') : c;
    }


    std::string make_lowercase(std::string in) {
        for (size_t i = 0; i < in.length(); i++) {
            in[i] = (in[i] <= 'Z' && in[i] >= 'A') ? in[i] - ('Z' - 'z') : in[i];
        }
        return in;
    }

    std::string_view get_header(std::unordered_map<std::string, std::string>& headers, const std::string& header_name) {
        auto it = headers.find(make_lowercase(header_name));
        if (it != headers.end())
            return it->second;

        return std::string_view();
    }

    bool str_scan_until(std::string_view buffer, uint64_t& index, std::string term, uint64_t search_char_limit, std::string& parsed) {
        uint64_t initial_index = index;
        uint64_t term_index = 0;

        for (uint64_t i = 0; i < search_char_limit && index < buffer.length() && term_index < term.length(); i++) {
            if (buffer[index] == term[term_index]) {
                term_index++;

                if (term_index == term.length()) {
                    parsed = buffer.substr(initial_index, i - term_index + 1);
                    index++;
                    return true;
                }
            }
            else {
                term_index = 0;
            }

            index++;
        }

        parsed = buffer.substr(initial_index);
        return false;
    }

    bool str_scan_no_whitespace_until(std::string_view buffer, uint64_t& index, std::string term, uint64_t search_char_limit, std::string& parsed) {
        uint64_t initial_index = index;
        uint64_t term_index = 0;

        for (uint64_t i = 0; i < search_char_limit && index < buffer.length() && term_index < term.length(); i++) {
            if (buffer[index] == '\n' || buffer[index] == '\r' || buffer[index] == ' ' || buffer[index] == '\t')
                return false;

            if (buffer[index] == term[term_index]) {
                term_index++;

                if (term_index == term.length()) {
                    parsed = buffer.substr(initial_index, i - term_index + 1);
                    index++;
                    return true;
                }
            }
            else {
                term_index = 0;
            }

            index++;
        }

        parsed = buffer.substr(initial_index);
        return false;
    }

    void str_skip_spaces(std::string buffer, uint64_t& index) {
        while (buffer[index] == ' ' && index < buffer.length()) {
            index++;
        }
    }

    http_parse_status parse_http_headers(http_request& req) {
        uint64_t index = 0;

        if (!str_scan_until(req.session->buffers.client_rx_buffer, index, " ", conf::METHOD_NAME_SIZE_LIMIT, req.method))
            return NWS_PARSE_FAIL_REQ_METHOD;

        if (!str_scan_until(req.session->buffers.client_rx_buffer, index, " ", conf::URL_SIZE_LIMIT, req.url))
            return NWS_PARSE_FAIL_REQ_URL;

        if (!str_scan_until(req.session->buffers.client_rx_buffer, index, "\r\n", conf::HTTP_VER_NAME_SIZE_LIMIT, req.version))
            return NWS_PARSE_FAIL_REQ_HTTP_VER;

        if (req.version != conf::PROTO_IDENTIFIER) {
            error_page(req, 505, "Request version is " + req.version + " but server requires " + conf::PROTO_IDENTIFIER);
            return NWS_PARSE_FAIL_REQ_HTTP_VER;
        }

        std::string header_name;
        std::string header_value;

        int count = 0;

        while (count++ < 1000) { // TODO: make this a for loop
            if (!str_scan_no_whitespace_until(req.session->buffers.client_rx_buffer, index, ":", conf::HEADER_NAME_SIZE_LIMIT, header_name))
                break;

            str_skip_spaces(req.session->buffers.client_rx_buffer, index);

            if (!str_scan_until(req.session->buffers.client_rx_buffer, index, "\r\n", conf::HEADER_VALUE_SIZE_LIMIT, header_value))
                return NWS_PARSE_FAIL_REQ_HEADER;

            req.headers[make_lowercase(header_name)] = header_value;
        }

        return NWS_PARSE_OK;
    }

    std::string decode_uri_component(std::string_view param_value) {
        uint64_t index = 0;
        std::string out = "";

        // make this a ranged lut in the future or > <= mapped to val
        static std::unordered_map<char, char> hexmap = {
            { '0', 0 },
            { '1', 1 },
            { '2', 2 },
            { '3', 3 },
            { '4', 4 },
            { '5', 5 },
            { '6', 6 },
            { '7', 7 },
            { '8', 8 },
            { '9', 9 },
            { 'a', 10 },
            { 'b', 11 },
            { 'c', 12 },
            { 'd', 13 },
            { 'e', 14 },
            { 'f', 15 },
        };

        for (size_t i = 0; i < 1000; i++) {
            std::string piece;
            bool did = str_scan_until(param_value, index, "%", conf::HEADER_VALUE_SIZE_LIMIT, piece);

            for (size_t p = 0; p < piece.length(); p++) {
                if (piece[p] == '+')
                    piece[p] = ' ';
            }

            out += piece;

            if (index + 2 > param_value.length()) {
                return out;
            }

            char nibb1 = ___lower(param_value[index++]);
            char nibb2 = ___lower(param_value[index++]);

            char val = ((int)hexmap[nibb1] * 16) | hexmap[nibb2];

            out += val;

            if (!did) {
                return out;
            }
        }

        return out;
    }

    void parse_uri_encoded_param_str(std::string_view encoded_param_str, std::unordered_map<std::string, std::string>& decoded_output) {
        uint64_t index = 0;

        for (size_t i = 0; i < 1000; i++) { // TODO: make this `1000` a configurable value
            std::string param_name;
            std::string param_value;

            if (!str_scan_until(encoded_param_str, index, "=", conf::HEADER_NAME_SIZE_LIMIT, param_name))
                break;

            // & not required for last parameter
            str_scan_until(encoded_param_str, index, "&", conf::HEADER_VALUE_SIZE_LIMIT, param_value);
            decoded_output[param_name.substr(0, 100)] = decode_uri_component(param_value);
        }
    }

    void parse_req_url_params(http_request& req) {
        uint64_t index = 0;

        if (!str_scan_until(req.url, index, "?", conf::METHOD_NAME_SIZE_LIMIT, req.path))
            return; // no ? in the url, so no need to parse query parameters

        parse_uri_encoded_param_str(std::string_view(req.url).substr(index), req.query_param);
    }

    void parse_req_post_params(http_request& req) {
        parse_uri_encoded_param_str(req.body, req.post_param);
    }

    void parse_cookie_header(http_request& req) { // TODO: optimize by reducing copying
        std::string_view cookie_header = get_header(req.headers, "cookie");
        if (cookie_header.empty())
            return;

        uint64_t index = 0;

        for (size_t i = 0; i < conf::CONFIG_NUM_REQ_COOKIES; i++) {
            std::string cookie_name;
            std::string cookie_value;

            if (!str_scan_until(cookie_header, index, "=", conf::HEADER_NAME_SIZE_LIMIT, cookie_name))
                break;

            // & not required for last parameter
            str_scan_until(cookie_header, index, "; ", conf::HEADER_VALUE_SIZE_LIMIT, cookie_value);

            req.cookies[cookie_name.substr(0, 100)] = decode_uri_component(cookie_value);
        }
    }

}