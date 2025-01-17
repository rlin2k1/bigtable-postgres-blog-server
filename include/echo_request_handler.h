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

class echo_request_handler: public request_handler {
 public:  // API uses public functions
    static echo_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);
 private:
    std::string echo_path_;
    std::string build_request_string(const Request& request);
};

#endif  // INCLUDE_ECHO_REQUEST_HANDLER_H_
