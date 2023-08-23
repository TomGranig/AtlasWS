#pragma once

#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include "../http-transfer/http-transfer.hpp"

namespace atlas {

    bool serve_files(atlas::http_request& req, atlas::http_response& res, std::filesystem::path base);

}