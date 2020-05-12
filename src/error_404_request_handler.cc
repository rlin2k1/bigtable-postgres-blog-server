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

#include "request.h"
#include "reply.h"
#include "error_404_request_handler.h"

namespace http {
namespace server {
    error_404_request_handler* error_404_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
        // NginxConfig is not used for error requests.
        error_404_request_handler* erh = new error_404_request_handler();
        erh->error_path_ = location_path;
        return new error_404_request_handler();
    }

    void error_404_request_handler::handle_request(request& req, reply& rep,  const char *data) {
        BOOST_LOG_TRIVIAL(info) << "Request not found: 404 error.";
        // Fill out the reply to be sent to the client.
        rep.status = reply::not_found;
        rep.content = http::server::stock_replies::not_found;
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = "text/html";
    }
}  // namespace server
}  // namespace http
