/* status_request_handler.h
Header file for handling requests with the /echo uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 12th, 2020
*/

#ifndef HTTP_STATUS_REQUEST_HANDLER_HPP
#define HTTP_STATUS_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"
#include "response.h"

namespace http {
namespace server {

class Response;
struct request;

class status_request_handler: public request_handler {
 public:  // API uses public functions
    status_request_handler(const NginxConfig& config);
    static status_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    void record_received_request(std::string request_uri, Response::StatusCode response_status);
    virtual Response handle_request(const request& request);
 private:
    std::string status_path_;
    std::string handler_list;
    std::string received_request_list;
    int request_counter;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_STATUS_REQUEST_HANDLER_H_
