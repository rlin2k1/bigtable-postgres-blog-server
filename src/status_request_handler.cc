/* status_request_handler.cc
Request handler to serve echo responses.

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

namespace http {
namespace server {
    status_request_handler::status_request_handler(const NginxConfig& config) {
        // Initializing list of handlers from config file.
        std::unordered_set<std::string> echo_locations = config.echo_locations_;
        std::unordered_map<std::string, std::string> static_locations = config.static_locations_;
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
        BOOST_LOG_TRIVIAL(info) << "StatusHandler found list of existing handlers.";
        handler_list = config_handlers;
        // Initializing counter that keeps track of num of requests.
        request_counter = 0;
        // Initializing string that will keep track of requests received.
        received_request_list = "Received request(s):\r\n";
    }

    status_request_handler* status_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
        status_request_handler* srh = new status_request_handler(config);
        srh->status_path_ = location_path;
        return srh;
    }

    Response status_request_handler::handle_request(const request& request) {
        BOOST_LOG_TRIVIAL(info) << "Currently serving status requests on path: " << request.uri;
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

    void status_request_handler::record_received_request(std::string request_uri, Response::StatusCode response_status) {
        BOOST_LOG_TRIVIAL(info) << "Recieved request with URI " << request_uri << " and a status type " << response_status;
        std::string new_record = request_uri + " " + std::to_string(response_status) + "\r\n";
        received_request_list += new_record;
        request_counter++;
    }
}  // namespace server
}  // namespace http