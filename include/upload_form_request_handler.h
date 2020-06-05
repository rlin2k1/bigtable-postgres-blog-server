/* upload_form_request_handler.h
Header file for handling requests with the /upload uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    June 4th, 2020
*/

#ifndef HTTP_UPLOAD_FORM_REQUEST_HANDLER_HPP
#define HTTP_UPLOAD_FORM_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"

class upload_form_request_handler: public request_handler {
 public:
    static upload_form_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);

 private:
    std::string form_html_;
};

#endif  // HTTP_UPLOAD_FORM_REQUEST_HANDLER_HPP
