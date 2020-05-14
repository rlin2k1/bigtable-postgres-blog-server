/* error_404_request_handler.cc
Request handler to for 404 errors.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 11th, 2020
*/

#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>

#include "error_404_request_handler.h"
#include "response_helper_library.h"

error_404_request_handler* error_404_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
    // NginxConfig is not used for error requests.
    error_404_request_handler* erh = new error_404_request_handler();
    erh->error_path_ = location_path;
    return new error_404_request_handler();
}

Response error_404_request_handler::handle_request(const Request& request) {
    BOOST_LOG_TRIVIAL(info) << "Request not found: 404 error.";
    Response response;

    // Fill out the Response to be sent to the client.
    response.code_ = Response::not_found;
    response.body_ = stock_responses::not_found;
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";

    return response;
}
