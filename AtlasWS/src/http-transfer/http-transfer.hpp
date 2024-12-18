#pragma once

#include "../type.hpp"
#include "./chunked.hpp"
#include "../handler/handler.hpp"
#include "../status-code/status-code.hpp"
#include "../http-parse/http-parse.hpp"
#include "../error/error-message.hpp"

namespace atlas {
    
    void end(http_response& res);

    void write(http_response& res, std::string_view string);
    void write(http_response& res, std::vector<uint8_t> vec);
    void write(http_response& res, uint8_t* buffer, size_t buffer_size);

    void write_file_contents_sync(http_response& res, std::string file_path);

    void write_head(http_response& res, uint16_t status, const std::unordered_map<std::string, std::string>& headers);

    void end_normal_transfer(http_response& res);

    void write_normal_transfer(http_response& res, std::string_view string);

    void internal_handler_write_head(http_response& res, uint16_t status, const std::unordered_map<std::string, std::string>& headers);

    void internal_handler_end(http_response& res);

    void internal_handler_write(http_response& res, std::string_view string);

}

