#include "./socket.hpp"

namespace atlas {

    bool accept_new_socket(server* server, sockaddr_in* client_address, int32_t* client_socket) {
        uint32_t client_addr_len = sizeof(sockaddr_in);
        *client_socket = accept(server->sockfd, (struct sockaddr*)client_address, (socklen_t*)&client_addr_len);
        return *client_socket >= 0;
    }

    int32_t set_non_blocking(int32_t fd) {
        int32_t flags;
        if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
            flags = 0;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    int32_t create_listening_socket(uint16_t port) {
        int32_t fd = socket(AF_INET, SOCK_STREAM, 0);

        uint32_t option = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        signal(SIGPIPE, SIG_IGN);

        if (fd < 0)
            sv_error("Error opening socket");

        sockaddr_in serv_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {.s_addr = INADDR_ANY},
            .sin_zero = {}
        };

        if (bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            sv_error("ERROR on binding");

        listen(fd, 5);

        set_non_blocking(fd);

        return fd;
    }

}