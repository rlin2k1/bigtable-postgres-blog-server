/* request_handler.h
Header file for the request handler (abstract base class) regarding incoming requests.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_handler.hpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <fstream>
#include <sstream>

#include "response.h"
#include "request.h"
#include "request_handler.h"

namespace http {
namespace server {

class Response;
struct request;

// The common handler for all incoming requests.
class request_handler {
 public:
    // Handle a request and produce a Response
    // Pure virtual function. We need to derive from and then implement this method
    virtual Response handle_request(const request& request) = 0;
    // static RequestHandler* Init(const std::string& location_path, const NginxConfig& config);
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_REQUEST_HANDLER_H_
