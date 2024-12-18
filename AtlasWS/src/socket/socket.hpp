#pragma once

#include "../type.hpp"
#include "../error/error-message.hpp"

namespace atlas {

    bool accept_new_socket(server* server_instance, sockaddr_in* client_address, int32_t* client_socket);
    int32_t set_non_blocking(int32_t fd);
    int32_t create_listening_socket(uint16_t port);

}