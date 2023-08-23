#include "./handler.hpp"

namespace atlas {

    void try_accept(server* server) {

        sockaddr_in client_address;
        int32_t client_socket;

        if (!accept_new_socket(server, &client_address, &client_socket))
            return;

        bool accepted = false;

        

        server->current_requests++;
        for (uint32_t i = 0; i < server->max_concurrent_requests; i++) {
            http_session& sess = server->sessions[i];
            if (sess.session_status != HTTP_CONNECTION_CLOSED) // TODO: optimize traversal for large concurrency
                continue;

            sess.remote_addr = client_address;
            sess.client_fd = client_socket;
            sess.server = server;

            sess.operation.file_serve_task.active = false;
            sess.operation.file_serve_task.started_transfer = false;
            sess.operation.file_serve_task.file = NULL;
            sess.operation.client_rx_buffer_limit = NWS_DEFAULT_MAX_RX_BUFFER_SIZE;

            sess.req.session = &sess;
            sess.res.session = &sess;

            sess.session_status = HTTP_CONNECTION_ACCEPTED;
            reset_http_session_state(sess);

            sess.operation.index = i;

            // std::cout << "New socket: ID: " << i << "\n" << std::endl;

            sess.operation.last_io_t = time();
            set_non_blocking(client_socket);

            accepted = true;
            break;
        }

        if (!accepted) {
            printf("Too many sockets\n");
            close(client_socket);
        }
    }

    void socket_tick(server* server, http_session* sessions) {
        server->curr_rtime = time();

        for (uint16_t i = 0; i < server->max_concurrent_requests; i++) {
            http_session& sess = sessions[i];
            if (sess.session_status == HTTP_CONNECTION_CLOSED)
                continue;

            if (server->curr_rtime - sess.operation.last_io_t > NWS_NO_IO_TIMEOUT_MILLIS) { // TODO: WEBSOCKET ISSUE MAY BE CAUSED BY THIS
                // the connection timed out
                // std::cout << "Error: conn timeout: id=" << sess.operation.index << " path=`" << sess.req.path << "`\n";
                close_connection(sess);
                continue;
            }
            if (sess.session_status == HTTP_CONNECTION_ACCEPTED || sess.session_status == HTTP_CONNECTION_WAITING_FOR_FULL_REQUEST_BODY) {
                receive_tick(sess);
            } else if (sess.session_status == HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE || sess.session_status == HTTP_CONNECTION_SENDING_REMAINING_TX_BUFFER) {
                // if (r->file_serve_task.active && r->request_status == HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE)
                //     serve_file_range_tick(r);

                // std::cout << "FILERANGE Not implemented\n";

                send_tick(sess);
            }
        }
    }

    void server_tick(server* server) {
        for (;;) { // TODO: change to something like while (server->is_running)
            try_accept(server);
            socket_tick(server, server->sessions);

            struct timespec delay = { .tv_sec = 0, .tv_nsec = 10000 };
            nanosleep(&delay, NULL);
        }
    }

}