//
// static_request_handler.h
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_handler.hpp

#ifndef HTTP_STATIC_REQUEST_HANDLER_HPP
#define HTTP_STATIC_REQUEST_HANDLER_HPP

#include <string>
#include "request_handler.h"
#include "config_parser.h"
namespace http {
namespace server {

struct reply;
struct request;

// Echo handler for requests with /echo uri.
class static_request_handler: public request_handler {
 public:
    virtual void handle_request(const request& req, reply& rep, const char *data);
    void default_handle_bad_request(reply& rep);
    std::string get_mime_type(std::string file_name);
    NginxConfig* config_;
    std::string target_dir_;
    std::string target_file_;
    std::string partial_uri_;
};

}  // namespace server
}  // namespace http

#endif  // INCLUDE_STATIC_REQUEST_HANDLER_H_
