/* static_request_handler.h
Header file for handling requests with the /static uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 21st, 2020
*/

#ifndef HTTP_STATIC_REQUEST_HANDLER_HPP
#define HTTP_STATIC_REQUEST_HANDLER_HPP

#include <string>
#include <unordered_map>

#include "request_handler.h"
#include "config_parser.h"
namespace http {
namespace server {

struct reply;
struct request;

class static_request_handler: public request_handler {
    public:
        static_request_handler(NginxConfig* config);
        static static_request_handler* Init(NginxConfig* config);
        virtual void handle_request(request& req, reply& rep, const char *data);
        void default_handle_bad_request(reply& rep);  // TODO (JANE): Jane's 404 Handler will get rid of this
        std::string get_mime_type(std::string file_name);
        std::unordered_map<std::string, std::string> static_locations_;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_STATIC_REQUEST_HANDLER_H_
