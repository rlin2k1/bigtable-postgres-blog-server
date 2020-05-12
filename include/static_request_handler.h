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
    public: // API uses public functions
        static static_request_handler* Init(const std::string& location_path, const NginxConfig& config);
        virtual void handle_request(request& req, reply& rep, const char *data);

    private:
        void default_handle_bad_request(reply& rep);
        std::string get_mime_type(std::string file_name);
        std::string client_location_path_;
        std::string server_root_path_;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_STATIC_REQUEST_HANDLER_H_
