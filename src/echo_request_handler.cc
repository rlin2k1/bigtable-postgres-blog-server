/* echo_request_handler.cc
Request handler to serve echo responses.

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

namespace http {
namespace server {
    echo_request_handler* echo_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
        // NginxConfig is not used for echo requests.
        echo_request_handler* erh = new echo_request_handler();
        erh->echo_path_ = location_path;
        return erh;
    }

    Response echo_request_handler::handle_request(const request& request) {
        BOOST_LOG_TRIVIAL(info) << "Currently serving echo requests on path: " << request.uri;
        Response response;

        std::string rf(request.fullmessage.begin(), request.fullmessage.end());

        // Fill out the Response to be sent to the client.
        response.code_ = Response::ok;
        response.body_ = rf;
        response.headers_["Content-Length"] = std::to_string(response.body_.size());
        response.headers_["Content-Type"] = "text/plain";

        return response;
    }
}  // namespace server
}  // namespace http
