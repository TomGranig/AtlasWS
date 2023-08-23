#include "./servefiles.hpp"

// TODO: implement partial content / range requests support

namespace atlas {


    // Returns the canonical form of basepath/relpath if the canonical form
    // is under basepath, otherwise returns an empty path

    std::filesystem::path safe_join(const std::filesystem::path& basepath, const std::filesystem::path& relpath) {
        // Get the current working directory
        auto cwd = std::filesystem::current_path();
        // Prepend it to the base path if it is not absolute
        auto full_base = basepath.is_absolute() ? basepath : cwd / basepath;

        // Check if the relative path is empty or root
        if (relpath.empty() || relpath.is_absolute()) {
            return full_base;
        }
        // Check if the base path is empty or root
        if (full_base.empty() || full_base == "/") {
            return relpath;
        }
        // Normalize the paths and remove any "." or ".." components
        auto norm_base = std::filesystem::weakly_canonical(full_base);
        auto norm_rel = std::filesystem::weakly_canonical(relpath);
        // Check if the relative path is a subpath of the base path
        if (std::filesystem::relative(norm_rel, norm_base).empty()) {
            return norm_base / norm_rel;
        }

        return {};
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
        auto filepath = safe_join(base, req.path.substr(1));
        if (filepath.empty()) {
            // Invalid request, return error or something
            std::cout << "serve_files: access denied to requested path: " << req.path << "\n";
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


        atlas::write_head(res, 200, { {"content-type", content_type} });
        // TODO: implement a write() function that takes a file stream etc. and writes the file 
        // in chunks so that we don't have to load the whole file into memory and we can
        // then serve large files with range requests
        atlas::write(res, os.str());
        atlas::end(res);

        return true;
    }

}