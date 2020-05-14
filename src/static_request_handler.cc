/* static_request_handler.cc
Description:
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

#include "static_request_handler.h"
#include "response_helper_library.h"

/* static_request_handler* Init(const std::string& location_path, const NginxConfig& config)
Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a static_request_handler object.
Description: 
    - Uses values provided in the function's parameter to initialize a static_request_handler object */
static_request_handler* static_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    static_request_handler* srh = new static_request_handler();
    srh -> client_location_path_ = location_path;
    srh -> server_root_path_ = config.static_locations_.at(location_path);
    return srh;
}

/* Mapping from file extension to mime type */
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

/*  void static_request_handler::default_bad_request(Response& response)
Parameter(s):
    - response: Response object (see response.h)
Returns:
    - N/A
Description: 
    - Returns a not found response, called when a file is unable to be opened
    at file path by static handler. */
void static_request_handler::default_bad_request(Response& response) {
    response.code_ = Response::not_found;
    response.body_ = stock_responses::not_found;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";
}

/*  std::string static_request_handler::get_mime_type(std::string file_name)
Parameter(s):
    - file_name: name of file requested to be returned.
Returns:
    - Mime type that goes with the file name.
Description: 
    - Looks at the file extension of file_name, returns string value
    to be used for Content-Type */
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

/*  Response static_request_handler::handle_request(const request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description: 
    - Handler uses request URI to find mapping of client path to server path.
    Once path is found, file is opened and served back to client. */
Response static_request_handler::handle_request(const Request& request) {
    // Find the root directory and target file from the client's request uri
    Response response;

    std::string uri = request.uri_;
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
    // Fill out the Response to be sent to the client.
    std::string file_name = server_root_path_ + sub_uri_path;
    std::ifstream send_file;
    send_file.open(file_name.c_str());
    if (!send_file.good()) {
        BOOST_LOG_TRIVIAL(error) << "Could not open file at path: " << file_name;
        default_bad_request(response);
    } else {
        std::vector<char> send_data;
        char c = '\0';
        while (send_file.get(c)) {
            send_data.push_back(c);
        }
        std::string send_data_string(send_data.begin(), send_data.end());
        response.code_ = Response::ok;
        response.body_ = send_data_string;
        response.headers_["Content-Length"] = std::to_string(response.body_.size());
        response.headers_["Content-Type"] = get_mime_type(uri);
    }
    return response;
}
