#include "./http-range.hpp"

namespace atlas {

    // void serve_range(http_request* req, uint64_t content_length) {
    //     // set the headers and response code
    //     atlas::response(req, 200, { { "content-type", "text/plain" }, { "transfer-encoding", "chunked" }, { "connection", "keep-alive" } });

    //     atlas::write(req, "Hello World!");
    //     atlas::write(req, "\nYour request was:\n" + req->buffers.client_rx_buffer);

    //     atlas::end(req);
    // }

}