/* static_request_handler.cc
Request handler to serve static asset responses.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#include <fstream>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include "request.h"
#include "reply.h"
#include "static_request_handler.h"

namespace http {
namespace server {

static_request_handler* static_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    static_request_handler* srh = new static_request_handler();
    srh -> client_location_path_ = location_path;
    srh -> server_root_path_ = config.static_locations_.at(location_path);
    return srh;
}

std::unordered_map<std::string, std::string> mappings(
{
    { "gif", "image/gif" },
    { "htm", "text/html" },
    { "html", "text/html" },
    { "jpg", "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "png", "image/png" },
    { "zip", "application/zip" },
    { "txt", "text/plain" },
    { "pdf", "application/pdf" }
});

// Return a Not found reply if there are no echo or
// static uris that can be handled
void static_request_handler::default_handle_bad_request(reply& rep) {
    rep.status = reply::not_found;
    rep.content = http::server::stock_replies::not_found;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/html";
}

// Get mapping of mime type
std::string static_request_handler::get_mime_type(std::string file_name) {
    size_t last_dot_index = file_name.find_last_of(".");
    if (last_dot_index != std::string::npos) {
        std::string file_extension = file_name.substr(last_dot_index + 1);
        if (mappings.find(file_extension) != mappings.end()) {
            return mappings[file_extension];
        }
    }
    return "text/plain";
}

void static_request_handler::handle_request(request& req, reply& rep,  const char *data) {
    // Find the root directory and target file from the client's request uri
    std::string uri = req.uri;
    size_t space_index = 0;
    while (true) {
        space_index = uri.find("%20", space_index);
        if (space_index == std::string::npos) {
            break;
        }
        uri.replace(space_index, 3, " ");
    }

    // The path that is unique to the client that we want to map to the server side path
    std::string client_uri_path = uri;

    // The rest of the uri path that is the same on the server side as it is on the client side (subdirectories)
    std::string sub_uri_path = uri;

    size_t slash_pos = 0;
    while (true) {
        if (client_uri_path == client_location_path_) {
            break;
        }
        slash_pos = client_uri_path.find_last_of("/");
        if (slash_pos == std::string::npos) {
            break;
        }
        client_uri_path = client_uri_path.substr(0, slash_pos);
        sub_uri_path = uri.substr(slash_pos);
    }

    BOOST_LOG_TRIVIAL(info) << "Currently serving static requests on path: " << uri;

    //--------------------------------------------------------------------------
    // Fill out the reply to be sent to the client.
    std::string file_name = server_root_path_ + sub_uri_path;
    std::ifstream send_file;
    send_file.open(file_name.c_str());
    if (!send_file.good()) {
        BOOST_LOG_TRIVIAL(error) << "Could not open file at path: " << file_name;
        default_handle_bad_request(rep);
    } else {
        std::vector<char> send_data;
        char c = '\0';
        while (send_file.get(c)) {
            send_data.push_back(c);
        }
        std::string send_data_string(send_data.begin(), send_data.end());
        rep.status = reply::ok;
        rep.content = send_data_string;
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = get_mime_type(uri);
    }
}

}  // namespace server
}  // namespace http
