#include "util.hpp"

namespace base64 {

    std::string encode(const uint8_t* data, size_t length) {
        static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string result;
        result.reserve((length + 2) / 3 * 4);

        for (size_t i = 0; i < length; i += 3) {
            uint32_t octet_a = i < length ? data[i] : 0;
            uint32_t octet_b = i + 1 < length ? data[i + 1] : 0;
            uint32_t octet_c = i + 2 < length ? data[i + 2] : 0;

            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

            result.push_back(base64_chars[(triple >> 3 * 6) & 0x3F]);
            result.push_back(base64_chars[(triple >> 2 * 6) & 0x3F]);
            result.push_back(i + 1 < length ? base64_chars[(triple >> 1 * 6) & 0x3F] : '=');
            result.push_back(i + 2 < length ? base64_chars[(triple >> 0 * 6) & 0x3F] : '=');
        }

        return result;
    }

}


