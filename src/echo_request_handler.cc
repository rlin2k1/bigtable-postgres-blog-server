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

#include "request.h"
#include "reply.h"
#include "echo_request_handler.h"

namespace http {
namespace server {
    void echo_request_handler::handle_request(const request& req, reply& rep,  const char *data) { 
        // Fill out the reply to be sent to the client.
        rep.status = reply::ok;
        rep.content = data;
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = std::to_string(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = "text/plain";
    }
}  // namespace server
}  // namespace http
