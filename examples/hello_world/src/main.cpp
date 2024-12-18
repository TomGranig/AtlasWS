
#include "AtlasWS.hpp"



int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);


    // called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {
        // write the response head (status code, headers)
        atlas::write_head(res, 200, { { "content-type", "text/html" } });

        // write the response body
        atlas::write(res, "<h1>Hello, World!</h1>");

        // end the response
        atlas::end(res);
    };


    // Pause the main thread so the server doesn't close
    // You could do other things here as long as the main thread doesn't close
    pause();
}

