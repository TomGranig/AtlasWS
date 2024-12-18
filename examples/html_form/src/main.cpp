
#include "AtlasWS.hpp"

std::vector<std::pair<std::string, std::string>> responses = {};

void handle_main_page(atlas::http_request& req, atlas::http_response& res) {

    // Check if the request is a POST request
    if (req.method == "POST") {
        if (!req.post_param["name"].empty() && !req.post_param["message"].empty()) {
            responses.push_back({ req.post_param["name"], req.post_param["message"] });

            // redirect the client to the responses page
            atlas::write_head(res, atlas::HTTP_FOUND, { { "location", "/responses" } });
            atlas::write(res, "Successfully recorded response");
        }
        else {
            atlas::write(res, "Invalid response");
        }

        atlas::end(res);
        return;
    }
    // Write the response's body
    atlas::write(res, R"(<!DOCTYPE html>
        <html>
            <head>
                <title>Form Example</title>
            </head>
            <body>
                <h1>Form Example</h1>
                <p>Fill out the form below and submit it to see the responses recorded at <a href="/responses">/responses</a></p>
                <form action="/" method="POST">
                    <input type="text" name="name" placeholder="Name">
                    <textarea style="width: 100%;" name="message" placeholder="Message"></textarea>
                    <input type="submit" value="Submit">
                </form>
            </body>
        </html>
    )");

    atlas::end(res);
}

void handle_responses_page(atlas::http_request& req, atlas::http_response& res) {
    // Write the response's body
    atlas::write(res, R"(<!DOCTYPE html>
        <html>
            <head>
                <title>Responses</title>
            </head>
            <body>
                <h1>Responses</h1>
                <p>Below are the responses recorded by the form at <a href="/">/</a></p>
                <ul>
    )");

    // Write the responses
    for (auto& response : responses) {
        atlas::write(res, "<li style=\"white-space: pre-wrap;\"><div style=\"font-weight: bold;\">" + response.first + ":</div>" + response.second + "</li>");
    }

    atlas::write(res, R"(</ul>
            </body>
        </html>
    )");

    atlas::end(res);
}

int main() {
    // Create a server
    atlas::server server;

    // Start the server and listen on port 8080
    atlas::begin(server, 8080);

    // Set the server's onrequest callback
    // This callback is called when an HTTP request is received
    server.onrequest = [&](auto& req, auto& res) {
        if (req.path == "/") {
            handle_main_page(req, res);
        }
        else if (req.path == "/responses") {
            handle_responses_page(req, res);
        }
        else {
            atlas::error_page(res, 404, "Not found");
        }
    };

    // Pause the main thread so the server doesn't close
    pause();
}
