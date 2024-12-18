#include "./handler.hpp"

namespace atlas {

    void update_event(http_session& sess, bool want_read, bool want_write) {

#ifdef __linux__

        struct epoll_event event;
        event.data.fd = sess.client_fd;
        event.events = EPOLLIN | EPOLLET;
        if (want_read) {
            event.events |= EPOLLIN;
        }
        if (want_write) {
            event.events |= EPOLLOUT;
        }

        if (epoll_ctl(sess.server->ev_fd, EPOLL_CTL_MOD, sess.client_fd, &event) == -1) {
            perror("epoll_ctl: client_socket");
            close(sess.client_fd);
            return;
        }

#elif defined(__APPLE__) || defined(__FreeBSD__)


        struct kevent change[2];
        int change_count = 0;

        if (want_read) {
            EV_SET(&change[change_count++], sess.client_fd, EVFILT_READ, EV_ADD, 0, 0, &sess);
        }
        else {
            EV_SET(&change[change_count++], sess.client_fd, EVFILT_READ, EV_DELETE, 0, 0, &sess);
        }

        if (want_write) {
            EV_SET(&change[change_count++], sess.client_fd, EVFILT_WRITE, EV_ADD, 0, 0, &sess);
        }
        else {
            EV_SET(&change[change_count++], sess.client_fd, EVFILT_WRITE, EV_DELETE, 0, 0, &sess);
        }

        kevent(sess.server->ev_fd, change, change_count, NULL, 0, &sess.server->ev_fd_timeout);

#endif

    }

    void try_accept(server* server) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int32_t client_socket = accept(server->sockfd, (struct sockaddr*)&client_address, &client_len);

        if (client_socket == -1) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("accept");
            }
            return;
        }

        bool accepted = false;

        for (uint32_t i = 0; i < server->max_concurrent_requests; i++) {
            http_session& sess = server->sessions[i];
            if (sess.session_status != HTTP_CONNECTION_CLOSED) // TODO: optimize traversal for large concurrency
                continue;

            sess.remote_addr = client_address;
            sess.client_fd = client_socket;
            sess.server = server;
            sess.upgraded_proto_data = nullptr;

            sess.operation.file_serve_task.active = false;
            sess.operation.file_serve_task.started_transfer = false;
            sess.operation.file_serve_task.file = nullptr;
            sess.operation.client_rx_buffer_limit = conf::DEFAULT_MAX_RX_BUFFER_SIZE;

            sess.req.session = &sess;
            sess.res.session = &sess;

            sess.session_status = HTTP_CONNECTION_ACCEPTED;
            reset_http_session_state(sess);

            sess.operation.index = i;

            sess.operation.last_io_t = time();
            set_non_blocking(client_socket);

            // register for read events
            update_event(sess, true, false);

            accepted = true;

            socket_tick_one(server, sess);

            break;
        }

        if (!accepted) {
            close(client_socket);
        }
    }

    void socket_tick_all(server* server, http_session* sessions) {
        server->curr_rtime = time();

        for (uint16_t i = 0; i < server->max_concurrent_requests; i++) {
            http_session& sess = sessions[i];

            socket_tick_one(server, sess);
        }
    }

    void socket_tick_one(server* server, http_session& sess) {
        // server->curr_rtime = time();

        std::unique_lock<std::mutex> lock(sess.buffers.mtx);


        if (sess.session_status == HTTP_CONNECTION_CLOSED)
            return;

        if ((int64_t)server->curr_rtime - (int64_t)sess.operation.last_io_t > conf::NO_IO_TIMEOUT_MILLIS) {
            std::cout << "Socket Timeout - Closing\n";
            close_connection(sess);
        }
        else {

            // Check states and whether you want more reads or writes
            if (sess.session_status == HTTP_CONNECTION_ACCEPTED || sess.session_status == HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY) {
                receive_tick(sess);

                int64_t remaining_to_send = sess.buffers.client_tx_buffer.length() - sess.operation.sent_bytes;
                bool has_data_to_send = remaining_to_send > 0;
                update_event(sess, true, has_data_to_send);
            }
            else if (sess.session_status == HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE || sess.session_status == HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER) {
                send_tick(sess);

                int64_t remaining_to_send = sess.buffers.client_tx_buffer.length() - sess.operation.sent_bytes;
                bool has_data_to_send = remaining_to_send > 0;
                update_event(sess, !has_data_to_send, true);
            }
            else if (sess.session_status == HTTP_CONNECTION_UPGRADED_TO_WEBSOCKET) {
                // std::cout << "Websocket tick\n";
                websocket_tick(sess);

                int64_t remaining_to_send = sess.buffers.client_tx_buffer.length() - sess.operation.sent_bytes;
                bool has_data_to_send = remaining_to_send > 0;
                update_event(sess, true, has_data_to_send);
            }
        }
    }



    void server_tick(server* server) {
#ifdef __linux__


        server->ev_fd = epoll_create1(0);
        if (server->ev_fd == -1) {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }

        struct epoll_event event;
        event.data.fd = server->sockfd;
        event.events = EPOLLIN;
        if (epoll_ctl(server->ev_fd, EPOLL_CTL_ADD, server->sockfd, &event) == -1) {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }

        struct epoll_event events[server->max_concurrent_requests];

        while (true) {
            int n = epoll_wait(server->ev_fd, events, server->max_concurrent_requests, -1);
            for (int i = 0; i < n; i++) {
                if (events[i].data.fd == server->sockfd) {
                    try_accept(server);
                }
                else {
                    socket_tick(server, server->sessions); // Handle socket I/O
                }
            }
        }

        close(server->ev_fd);


#elif defined(__APPLE__) || defined(__FreeBSD__)


        server->ev_fd = kqueue();
        if (server->ev_fd == -1) {
            perror("kqueue");
            exit(EXIT_FAILURE);
        }

        // std::cout << "kqueue created\n";

        struct kevent change;
        EV_SET(&change, server->sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);


        if (kevent(server->ev_fd, &change, 1, NULL, 0, NULL) == -1) {
            perror("kevent");
            exit(EXIT_FAILURE);
        }

        struct kevent events[server->max_concurrent_requests];


        while (true) {
            server->curr_rtime = time();

            int nev = kevent(server->ev_fd, NULL, 0, events, server->max_concurrent_requests, &server->ev_fd_timeout);
            if (nev == -1) {
                std::cout << "Error\n";
                perror("kevent wait");
                exit(EXIT_FAILURE);
            }
            else if (nev == 0) {
                socket_tick_all(server, server->sessions);

                continue;
            }

            for (int i = 0; i < nev; i++) {
                if (events[i].ident == server->sockfd)
                    try_accept(server);
                else
                    socket_tick_one(server, *(http_session*)events[i].udata);
            }
        }

        close(server->ev_fd);


#else

#error "Unsupported platform"

#endif


}


}