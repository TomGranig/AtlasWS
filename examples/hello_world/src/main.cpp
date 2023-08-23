
#include "AtlasWS.hpp"

int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {
        // Write "Hello, world!" to the client
        atlas::write_head(res, atlas::HTTP_OK, {{"Content-Type", "text/html"}});
        atlas::write(res, "<h1>Hello, world!</h1>");
        atlas::end(res);
    };

    // Pause the main thread so the server doesn't close
    // You could do other things here as long as the main thread doesn't close
    pause();    
}