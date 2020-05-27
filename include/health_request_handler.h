/* health_request_handler.h
Header file for handling requests with the /health uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 27th, 2020
*/

#ifndef HTTP_HEALTH_REQUEST_HANDLER_HPP
#define HTTP_HEALTH_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"

class health_request_handler: public request_handler {
 public:
    static health_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);

 private:
    std::string health_path_;
};

#endif  // INCLUDE_health_REQUEST_HANDLER_H_
