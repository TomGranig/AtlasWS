#pragma once

#include <string>
#include <sstream>
#include <cstdint>
#include <cstring>


namespace sha1 {

    struct context {
        uint32_t state[5];
        uint32_t count[2];
        unsigned char buffer[64];
    };

    void digest(uint8_t* hash_out, const uint8_t* plaintext, uint32_t len);

}