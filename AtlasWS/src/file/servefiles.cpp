#include "./servefiles.hpp"

// TODO: implement partial content / range requests support

namespace atlas {


    // returns a path that is securely under the base path
    std::filesystem::path safe_join(const std::filesystem::path& basepath, const std::filesystem::path& relpath) {
        auto full_base = basepath.is_absolute() ? basepath : std::filesystem::current_path() / basepath;

        // weakly canonicalize the base path
        auto norm_base = std::filesystem::weakly_canonical(full_base);

        //dpnt allow absolute paths
        if (relpath.empty() || relpath.is_absolute()) {
            return {};
        }

        auto combined_path = norm_base / relpath;

        // canonicalize the combined path to resolve any ".." or "." components
        auto norm_combined = std::filesystem::weakly_canonical(combined_path);

        // check if the combined normalizd path starts with the normalized base path
        if (norm_combined.native().rfind(norm_base.native(), 0) == 0) {
            return norm_combined;
        }

        return {}; // access denied
    }

    // A function that returns the content type based on the file extension
    std::string get_content_type(const std::string& req_path) {
        // A map of common file extensions and their content types
        static const std::unordered_map<std::string, std::string> mime_types = {
            {".html", "text/html"}, {".css", "text/css"},  {".js", "application/javascript"}, {".json", "application/json"}, {".xml", "application/xml"}, {".txt", "text/plain"}, {".css", "text/css"}, {".pdf", "application/pdf"}, {".jpg", "image/jpeg"},
            {".png", "image/png"},  {".gif", "image/gif"}, {".svg", "image/svg+xml"},
            // Add more extensions and content types as needed
        };

        // Find the last dot in the request path
        size_t dot_pos = req_path.find_last_of('.');
        // If there is no dot, return a default content type
        if (dot_pos == std::string::npos) {
            return "application/octet-stream";
        }
        // Extract the file extension from the request path
        std::string ext = req_path.substr(dot_pos);
        // Find the extension in the map
        auto it = mime_types.find(ext);
        // If the extension is not in the map, return a default content type
        if (it == mime_types.end()) {
            return "application/octet-stream";
        }
        // Return the content type from the map
        return it->second;
    }

    bool serve_files(atlas::http_request& req, atlas::http_response& res, std::filesystem::path base) {

        std::string request_path = req.path == "/" ? "index.html" : req.path.substr(1);

        std::filesystem::path filepath = safe_join(base, request_path);
        if (filepath.empty()) {
            // Invalid request, return error or something
            std::cout << "serve_files: access denied to requested path: " << req.path << "\n from base path: " << base << " and requested path: " << req.path << "\n";
            return false;
        }


        // If the requested path is a directory, append "index.html" to it
        if (std::filesystem::is_directory(filepath)) {
            filepath /= "index.html";
        }

        std::fstream file = std::fstream(filepath, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "serve_files: " << strerror(errno) << "\n";  // print the error message
            return false;
        }

        std::string content_type = get_content_type(filepath.string());

        std::ostringstream os;
        os << file.rdbuf();


        atlas::write_head(res, 200, { {"content-type", content_type}, {"connection", "keep-alive"} });
        // TODO: implement a write() function that takes a file stream etc. and writes the file 
        // in chunks so that we don't have to load the whole file into memory and we can
        // then serve large files with range requests
        atlas::write(res, os.str());
        atlas::end(res);

        return true;
    }

}