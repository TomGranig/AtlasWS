/*

    A very basic example of a chat app using the AtlasWS web server

    To run, ensure the current working directory is the "build" folder.
    Then, run the executable.

*/

#include "AtlasWS.hpp"
#include "main.hpp"

std::vector<chat_message> chat_log = {};
std::vector<atlas::http_response> waiting_clients = {};


void handle_send(atlas::http_request& req, atlas::http_response& res) {
    // get the message from the request body
    std::string message(req.body);
    std::string username = req.query_param["username"];

    // add the message to the chat log
    chat_log.push_back({
        username,
        message
    });

    // send the message to all waiting clients
    for (auto& client : waiting_clients) {

        // ensure the client is still waiting for a full response
        if (client.session->session_status != atlas::HTTP_CONNECTION_WAITING_FOR_FULL_RESPONSE) {
            continue;
        }

        // send the message to the client
        atlas::write(client, username + ": " + message + "\n");
        atlas::end(client);
    }

    // clear the waiting clients
    waiting_clients.clear();

    // send the response
    atlas::write_head(res, 200, { { "content-type", "text/plain" } });
    atlas::write(res, "Message sent successfully");
    atlas::end(res);
}

void handle_send_chat_log(atlas::http_request& req, atlas::http_response& res) {
    // send the chat log to the client
    atlas::write_head(res, 200, { { "content-type", "text/plain" } });
    for (auto& message : chat_log) {
        atlas::write(res, message.username + ": " + message.message + "\n");
    }
    atlas::end(res);
}


int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    chat_log.push_back({
        "Server",
        "Welcome to the AtlasWS HTTP Chat Example!"
    });

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {

        // check if the HTTP request is a POST request
        if (req.method == "POST") {
            if (req.path == "/send") {
                handle_send(req, res);
            }
        }
        else if (req.method == "GET") {
            if (req.path == "/get-messages") {
                // send the complete chat log to the client
                handle_send_chat_log(req, res);
            }
            else if (req.path == "/wait-for-messages") {
                // wait for a new message to be sent
                atlas::write_head(res, 200, { { "content-type", "text/plain" } });
                waiting_clients.push_back(res);
            }
            else if (atlas::serve_files(req, res, "../client/")) {
                // the requested file was served successfully
            }
            else {
                // send a 404 error page
                atlas::error_page(req, 404, "Not Found");
            }
        }
        };

    // Pause the main thread so the server doesn't close
    pause();
}
