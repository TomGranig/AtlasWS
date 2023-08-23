# AtlasWS - A simple and flexible web server library written in C++

## Features
- [x] State-machine based for low memory footprint and high performance
- [x] Simple and flexible API
- [x] Support for HTTP/1.1 (HTTP/2.0 planned)
- [x] Built in basic query parameter, body, and cookie parsing

## Example
```c++
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
```

## Building
A makefile is provided for building the library. Run `make` in the AtlasWS directory to build the library. The library will be built in the `AtlasWS/build` directory.
Makefiles are also provided for building the examples. Run `make` in one of the example directories to build the example. To run the example, cd into the example's `build`` directory and run `./main`.

There are currently 3 examples:
- `hello_world` - A simple "Hello, world!" example
- `http_chat_app` - A simple chat app that uses HTTP long polling
- `request_info` - An example that prints information about the request to the client

More examples will be added in the future.