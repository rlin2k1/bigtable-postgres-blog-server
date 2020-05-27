/* echo_request_handler.cc
Description:
    Request handler that serves responses to echo requests.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 21st, 2020
*/

#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>

#include "request.h"
#include "response.h"
#include "echo_request_handler.h"

/* echo_request_handler* Init(const std::string& location_path, const NginxConfig& config)
Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a echo_request_handler object.
Description: 
    - Uses values provided in the function's parameter to initialize a echo_request_handler object */
echo_request_handler* echo_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    // NginxConfig is not used for echo requests.
    echo_request_handler* erh = new echo_request_handler();
    erh->echo_path_ = location_path;
    return erh;
}

/*  std::string echo_request_handler::build_request_string(const Request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - string that formats the request body.
Description: 
    - Echo request is reformatted to be returned as a response body. */
std::string echo_request_handler::build_request_string(const Request& request) {
    std::string request_string;
    if (request.method_ == Request::MethodEnum::GET) {
        request_string += "GET ";
    } else if (request.method_ == Request::MethodEnum::POST) {
        request_string += "POST ";
    } else if (request.method_ == Request::MethodEnum::DELETE) {
        request_string += "DELETE ";
    }
    request_string += request.uri_;
    request_string += " ";
    request_string += request.version_;
    request_string += "\r\n";

    for (std::map<std::string,std::string>::const_iterator it=request.headers_.begin(); it!=request.headers_.end(); ++it) {
        std::string new_headerline = it->first + ": " + it->second + "\r\n";
        request_string += new_headerline;
    }
    request_string += "\r\n";

    request_string += request.body_;
    return request_string;
}

/*  Response echo_request_handler::handle_request(const request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description: 
    - Response object is generated and returned, using the request message as the response body. */
Response echo_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]Request_Handler: echo" ;
    // BOOST_LOG_TRIVIAL(info) << "Currently serving echo requests on path: " << request.uri_;
    Response response;

    std::string full_request_str = build_request_string(request);

    // Fill out the Response to be sent to the client.
    response.code_ = Response::ok;
    response.body_ = full_request_str;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/plain";

    return response;
}
