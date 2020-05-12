/* echo_request_handler.h
Header file for handling requests with the /echo uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 21st, 2020
*/

#ifndef HTTP_ECHO_REQUEST_HANDLER_HPP
#define HTTP_ECHO_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"

namespace http {
namespace server {

struct reply;
struct request;

class echo_request_handler: public request_handler {
 public:  // API uses public functions
    static echo_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual void handle_request(request& req, reply& rep, const char *data);
 private:
    std::string echo_path_;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_ECHO_REQUEST_HANDLER_H_
