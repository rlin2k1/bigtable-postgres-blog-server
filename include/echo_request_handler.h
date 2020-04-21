//
// echo_request_handler.h
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_handler.hpp

#ifndef HTTP_ECHO_REQUEST_HANDLER_HPP
#define HTTP_ECHO_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"

namespace http {
namespace server {

struct reply;
struct request;

// Echo handler for requests with /echo uri.
class echo_request_handler: public request_handler {
 public:
    virtual void handle_request(const request& req, reply& rep, const char *data);
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_ECHO_REQUEST_HANDLER_H_
