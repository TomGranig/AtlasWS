
#include "AtlasWS.hpp"

int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {
        atlas::write_head(res, 200, { { "content-type", "text/plain" } });

        // write the request's method and path
        atlas::write(res, "Method: " + req.method + "\n");
        atlas::write(res, "Path: " + req.path + "\n");

        // write the request's headers
        atlas::write(res, "\nHeaders:\n");
        for (auto& header : req.headers) {
            atlas::write(res, header.first + ": " + header.second + "\n");
        }

        // write the request's URL parameters
        atlas::write(res, "\nURL Parameters:\n");
        for (auto& param : req.query_param) {
            atlas::write(res, param.first + ": " + param.second + "\n");
        }

        // write the request's body parameters
        atlas::write(res, "\nPOST Parameters:\n");
        for (auto& param : req.post_param) {
            atlas::write(res, param.first + ": " + param.second + "\n");
        }

        // write the request's cookies
        atlas::write(res, "\nCookies:\n");
        for (auto& cookie : req.cookies) {
            atlas::write(res, cookie.first + ": " + cookie.second + "\n");
        }


        atlas::end(res);


    };

    // Pause the main thread so the server doesn't close
    pause();    
}
