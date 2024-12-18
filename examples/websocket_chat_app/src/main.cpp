/*

    A very basic example of a chat app using AtlasWS's websocket server functionality

    To run, ensure the current working directory is the "build" folder.
    Then, run the executable.

*/

#include "AtlasWS.hpp"
#include "main.hpp"

std::vector<chat_message> chat_log = {};
std::vector<atlas::websocket*> websockets = {};


void send_chat_log(atlas::websocket& ws) {
    for (auto& message : chat_log) {
        atlas::send(ws, { atlas::WS_MESSAGE_TEXT, message.username + ": " + message.message });
    }
}

void relay_message(atlas::websocket& ws, atlas::websocket_message& message) {
    uint32_t colon_pos = message.data.find(':');
    std::string message_username = message.data.substr(0, colon_pos);
    std::string message_text = message.data.substr(colon_pos + 1);

    chat_log.push_back({
        message_username,
        message_text
    });

    for (auto& ws_ptr : websockets) {
        if (ws_ptr == &ws) {
            continue;
        }

        atlas::send(*ws_ptr, { atlas::WS_MESSAGE_TEXT, message_username + ": " + message_text });
    }
}


int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    chat_log.push_back({
        .username = "Server",
        .message = "Welcome to the AtlasWS WebSocket Chat Example"
    });

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {

         if (req.path == "/ws") { // can use whatever logic you want to determine whether to allow a WebSocket connection, i.e. checking "origin" header
            
            if (!atlas::upgrade_to_websocket(req, res, [](atlas::websocket& ws) { // upgrade the http connection to a WebSocket connection

                // add the websocket to the list of websockets
                websockets.push_back(&ws);

                send_chat_log(ws);

                ws.onmessage = [](auto& ws, auto& message) {
                    if (message.data == "keepalive") {
                        return;
                    }

                    relay_message(ws, message);
                };

                ws.onclose = [](auto& ws) {
                    std::cout << "WebSocket connection closed\n";

                    // remove the websocket from the list of websockets
                    websockets.erase(std::remove(websockets.begin(), websockets.end(), &ws), websockets.end());
                };

            })) {
                
                // could not upgrade the request to a WebSocket connection
                atlas::error_page(res, 400, "Bad Request");
                return;
            }

            return;
        }

        if (atlas::serve_files(req, res, "../client/")) {
            // the requested file was served successfully
            return;
        }


        // send a 404 error page (no other request handlers matched)
        atlas::error_page(req, 404, "Not Found");

    };

    // Pause the main thread so the server doesn't close
    pause();
}
