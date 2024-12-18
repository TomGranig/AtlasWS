#include "./error-message.hpp"

namespace atlas {

    void sv_error(const char* err) {
        perror(err);
        exit(1);
    }

    void error_page(http_request& req, uint16_t status_code, std::string error_description) {
        if (req.session->session_status != HTTP_CONNECTION_ACCEPTED)
            return;

        http_response& res = req.session->res;

        write_head(res, status_code, { { "content-type", "text/html" }, { "connection", "close" } });

        std::string status_str = get_status_str(status_code);

        write(res, "<html><head><title>" + status_str + R"(</title>
<style>
body {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 100vw;
    height: 100vh;
    justify-content: center;
    font-family: sans-serif;
    color: #727272;
    gap: 1.2em;
    padding: 4em;
    box-sizing: border-box;
    margin: 0;
}

h3 {
    font-family: monospace;
    background: #fc6c6c;
    padding: 10px 20px;
    border-radius: 10px;
    color: #fff;
}

h1 {
    margin: 0;
    color: #000;
}
</style>
</head><body>)"
        );
        write(res, "<h1>" + status_str + "</h1>");
        write(res, "<h3>" + error_description + "</h3>");
        write(res, "AtlasWS " + std::string(conf::WEB_VERSION) + " - A powerful HTTP and WebSocket Server Framwork");
        write(res, "</body></html>");

        

        end(res);
    }

    void error_page(http_response& res, uint16_t status_code, std::string error_description) {
        http_request& req = res.session->req;
        error_page(req, status_code, error_description);
    }


}