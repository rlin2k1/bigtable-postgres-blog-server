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

class request_dispatcher {
    public:
        request_dispatcher(NginxConfig* config);
        void create_handler_mapping();
        http::server::request_handler* get_handler(std::string uri);

    private:
        NginxConfig* config_;
        std::unordered_map<std::string, http::server::request_handler*> dispatcher;  // URI to Handler Mapping
};

#endif  // INCLUDE_REQUEST_DISPATCHER
