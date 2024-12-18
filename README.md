# AtlasWS - A simple and flexible HTTP and Websocket server library written in C++

## Features
- [x] State-machine based for low memory footprint and high performance using just one thread
- [x] Simple and flexible API
- [x] Support for HTTP/1.1
- [x] Built in basic query parameter, body, and cookie parsing
- [x] Websocket Server
- [x] Basic file serving from a directory
- [ ] JSON parsing



## Creating a Simple HTTP Server
```c++
#include "AtlasWS.hpp"

int main() {
    // Create a server object
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {
        // Write "Hello, world!" to the client
        atlas::write_head(res, atlas::HTTP_OK, {{"Content-Type", "text/html"}, {"Connection", "keep-alive"}});
        atlas::write(res, "<h1>Hello, world!</h1><p>You requested: " + req.path + "</p>");
        atlas::end(res);
    };

    // Pause the main thread so the server doesn't close
    // You could do other things here as long as the main thread doesn't close
    pause();    
}
```

## Creating a Simple Websocket Server
```c++
#include "AtlasWS.hpp"

int main() {
    // Create a server object
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {

        // can use whatever logic you want to determine whether to allow a WebSocket connection, i.e. checking "origin" header
         if (req.path == "/ws") { 
            
            // upgrade the http connection to a WebSocket connection
            if (!atlas::upgrade_to_websocket(req, res, [](atlas::websocket& ws) { 

                // set the callback for when a message is received from the client
                ws.onmessage = [](auto& ws, auto& message) {
                    std::cout << "Received message: " << message.data << "\n";

                    // send the message back to the client
                    atlas::send(ws, message);
                };

                // set the callback for when the connection is closed
                ws.onclose = [](auto& ws) {
                    std::cout << "WebSocket connection closed\n";
                };

            })) {
                
                // could not upgrade the request to a WebSocket connection
                atlas::error_page(res, 400, "Bad Request");
                return;
            }

            return;
        }

        // other handlers here...

        // send a 404 error page (no other request handlers matched)
        atlas::error_page(req, 404, "Not Found");

    };

    // Pause the main thread so the server doesn't close
    // You could do other things here as long as the main thread doesn't close
    pause();
}

## Building the Library and Examples
A makefile is provided for building the library. Run `make` in the AtlasWS directory to build the library. The library will be built in the `AtlasWS/build` directory.

Makefiles are also provided for building the examples. Run `make` in one of the example directories to build the example. To run the example, `cd` into the example's `build` directory and run `./main`.

There are currently 3 examples:
- `hello_world` - A simple "Hello, world!" example
- `http_chat_app` - A simple chat app that uses HTTP long polling
- `websocket_chat_app` - A simple chat app that uses Websockets
- `request_info` - An example that prints information about the request to the client

More examples will be added in the future.