/* status_request_handler.cc
Description:
    Request handler that serves responses to status requests.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 12th, 2020
*/

#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>

#include "request.h"
#include "response.h"
#include "status_request_handler.h"

/*  status_request_handler Constructor
    Parameter(s):
        - config: parsed representation of configuration file (see config_parser.h)
    Description:
        - Initializes response body. Obtains information on list of what request handlers exist,
        and for what URL prefixes. */
status_request_handler::status_request_handler(const NginxConfig& config) {
    // Initializing list of handlers from config file.
    std::unordered_set<std::string> echo_locations = config.echo_locations_;
    std::unordered_map<std::string, std::string> static_locations = config.static_locations_;
    std::unordered_map<std::string, std::pair<std::string, int>> proxy_locations = config.proxy_locations_;
    std::string config_handlers = "";
    if (!(echo_locations).empty()) {
        config_handlers += "EchoHandler(s):\r\n";
        for (std::unordered_set<std::string>::iterator itr = echo_locations.begin(); itr != echo_locations.end(); ++itr) {
            config_handlers += (*itr);
            config_handlers += "\r\n";
        }
    }
    if (!(static_locations).empty()) {
        config_handlers += "StaticHandler(s):\r\n";
        for (std::unordered_map<std::string, std::string>::iterator itr = static_locations.begin(); itr != static_locations.end(); ++itr) {
            config_handlers += (itr->first);
            config_handlers += "\r\n";
        }
    }
    if (!(proxy_locations).empty()) {
        config_handlers += "ProxyHandler(s):\r\n";
        for (std::unordered_map<std::string, std::pair<std::string, int>>::iterator itr = proxy_locations.begin(); itr != proxy_locations.end(); ++itr) {
            config_handlers += (itr->first);
            config_handlers += "\r\n";
        }
    }
    BOOST_LOG_TRIVIAL(info) << "StatusHandler found list of existing handlers.";
    handler_list = config_handlers;
    // Initializing counter that keeps track of num of requests.
    request_counter = 0;
    // Initializing string that will keep track of requests received.
    received_request_list = "Received request(s):\r\n";
}

/* status_request_handler* Init(const std::string& location_path, const NginxConfig& config)
    Parameter(s):
        - location_path: path provided in config file which corresponds to handler
        - config: parsed representation of configuration file (see config_parser.h)
    Returns:
        - Pointer to a status_request_handler object.
    Description:
        - Uses values provided in the function's parameter to initialize a status_request_handler object */
status_request_handler* status_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    status_request_handler* srh = new status_request_handler(config);
    srh->status_path_ = location_path;
    return srh;
}

/*  Response status_request_handler::handle_request(const request& request)
    Parameter(s):
        - request: Request object (see request.h)
    Returns:
        - Response object (see response.h)
    Description:
        - Response object is generated and returned, with status information stored in the response body. */
Response status_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "Currently serving status requests on path: " << request.uri_;
    Response response;
    std::string num_received_req = "Number of requests received: " + std::to_string(request_counter) + "\r\n";
    std::string formatted_content = num_received_req + received_request_list + handler_list;
    // Fill out the Response to be sent to the client.
    response.code_ = Response::ok;
    response.body_ = formatted_content;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/plain";

    return response;
}

/* void status_request_handler::record_received_request(std::string request_uri, Response::StatusCode response_status)
    Parameter(s):
        - request_uri: URI value from parsed config file (see request.h)
        - response_status: status code returned from request (see response.h)
    Returns:
        - N/A, functions as a setter.
    Description:
        - Function allows status handler to store information about requests made and their returned status code. */
void status_request_handler::record_received_request(std::string request_uri, Response::StatusCode response_status) {
    BOOST_LOG_TRIVIAL(info) << "Recieved request with URI " << request_uri << " and a status type " << response_status;
    std::string new_record = request_uri + " " + std::to_string(response_status) + "\r\n";
    received_request_list += new_record;
    request_counter++;
}
