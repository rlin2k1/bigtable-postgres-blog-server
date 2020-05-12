/* error_404_request_handler.h
Header file for handling requests with the / uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 10th, 2020
*/

#ifndef HTTP_ERROR_404_REQUEST_HANDLER_HPP
#define HTTP_ERROR_404_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"

namespace http {
namespace server {

class Response;
struct request;

class error_404_request_handler: public request_handler {
 public:
    static error_404_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const request& request);

 private:
    std::string error_path_;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_ERROR_404_REQUEST_HANDLER_H_
