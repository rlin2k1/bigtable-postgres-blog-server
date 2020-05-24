/* redirect_request_handler.cc
Description:
    Request handler to serve redirect responses.

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 23rd, 2020
*/

#include "redirect_request_handler.h"

/* redirect_request_handler* Init(const std::string& location_path, const NginxConfig& config) {

Parameter(s):
    - location_path: path provided in config file which corresponds to handler
    - config: parsed representation of configuration file (see config_parser.h)
Returns:
    - Pointer to a proxy_request_handler object.
Description:
    - Uses values provided in the function's parameter to initialize a redirect_request_handler object */
redirect_request_handler* redirect_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    redirect_request_handler* rrh = new redirect_request_handler();
    rrh -> server_url_ = config.redirect_locations_.at(location_path);
    return rrh;
}

/*  Response handle_request(const Request& request)
Parameter(s):
    - request: Request object (see request.h)
Returns:
    - Response object (see response.h)
Description:
    - Handler returns 302 redirects to server_url_ */
Response redirect_request_handler::handle_request(const Request& request) {
    Response response;
    response.code_ = Response::moved_temporarily;
    response.headers_["Location"] = server_url_;
    return response;
}
