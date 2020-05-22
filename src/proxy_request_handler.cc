/* proxy_request_handler.cc
Description:
    Request handler to serve reverse proxy responses.

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 20th, 2020
*/

#include <string>
#include <unordered_map>
#include <boost/log/trivial.hpp>

#include "proxy_request_handler.h"
#include "response_helper_library.h"

/* proxy_request_handler* Init(const std::string& location_path, const NginxConfig& config)
Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a proxy_request_handler object.
Description:
    - Uses values provided in the function's parameter to initialize a proxy_request_handler object */
proxy_request_handler* proxy_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    proxy_request_handler* prh = new proxy_request_handler();
    prh -> client_location_path_ = location_path;
    prh -> server_location_path_ = config.proxy_locations_.at(location_path).first;
    prh -> server_port_num = config.proxy_locations_.at(location_path).second;
    return prh;
}

/*  Response proxy_request_handler::handle_request(const request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description:
    - Handler uses request URI to get full proxy destination, issue a request to that destination,
    and return the response. Also handles HTTP 302 Redirect. */
Response proxy_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "Currently serving proxy requests on path: " << request.uri_;
    Response response;

    // Temporary implementation: return 501 Not Implemented
    response.code_ = Response::not_implemented;
    response.body_ = stock_responses::not_implemented;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";

    return response;
}
