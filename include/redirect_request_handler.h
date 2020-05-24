/* redirect_request_handler.h
Header file for handling requests with any configured reverse proxy uri.

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 20th, 2020
*/

#ifndef HTTP_REDIRECT_REQUEST_HANDLER_HPP
#define HTTP_REDIRECT_REQUEST_HANDLER_HPP

#include <string>
#include <unordered_map>
#include <boost/log/trivial.hpp>

#include "request_handler.h"
#include "config_parser.h"

class redirect_request_handler: public request_handler {
 public: // API uses public member functions
    static redirect_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);
 private:
    std::string server_url_;
};

#endif  // INCLUDE_PROXY_REQUEST_HANDLER_H_
