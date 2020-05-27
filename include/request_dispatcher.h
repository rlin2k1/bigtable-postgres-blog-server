/* request_dispatcher.h
Header file for dispatching requests based on uri.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 8th, 2020
*/

#ifndef REQUEST_DISPATCHER
#define REQUEST_DISPATCHER

#include <string>
#include "request_handler.h"
#include "config_parser.h"
#include "error_404_request_handler.h"
#include "status_request_handler.h"
#include "proxy_request_handler.h"
#include "health_request_handler.h"

class request_dispatcher {
    public:
        request_dispatcher(const NginxConfig& config);
        void create_handler_mapping();
        request_handler* get_handler(std::string uri);
        status_request_handler* get_status_handler();
        bool status_handler_enabled = false; 

    private:
        const NginxConfig& config_;
        std::unordered_map<std::string, request_handler*> dispatcher;  // URI to Handler Mapping
        std::string longest_prefix_match(std::string uri);
        request_handler* error_handler_ = error_404_request_handler::Init("error_404", config_);
};

#endif  // INCLUDE_REQUEST_DISPATCHER
