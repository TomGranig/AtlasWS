#pragma once

#include <string>
#include <cstdint>



namespace base64 {

    std::string encode(const uint8_t* data, size_t length);

}