/* health_request_handler.cc
Description:
    Request handler that will always return a 200 OK response, with a simple plain text payload of OK.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 27th, 2020
*/

#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>

#include "health_request_handler.h"
#include "response_helper_library.h"


health_request_handler* health_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    health_request_handler* health_handler = new health_request_handler();
    health_handler->health_path_ = location_path;
    return new health_request_handler();
}

Response health_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]Request_Handler: health" ;
    BOOST_LOG_TRIVIAL(info) << "Health Request ok: success.";
    Response response;

    // Fill out the Response to be sent to the client.
    response.code_ = Response::ok;
    response.body_ = "OK";
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/plain";

    return response;
}
