/* request.h
Header file for the request struct.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request.hpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_REQUESTBUILDER_HPP
#define HTTP_REQUESTBUILDER_HPP

#include <string>
#include <vector>
#include "header.h"
#include "request.h"

/// A request received from a client.
class request_builder {
    public:
        std::string method;
        std::string uri;
        int http_version_major;
        int http_version_minor;
        std::vector<header> headers;
        int bodysize;
        std::vector<char> body;
        std::vector<char> fullmessage;
        bool keep_alive = false;
        Request build_request() {
            Request req;
            if (method == "GET") {
                req.method_ = Request::MethodEnum::GET;
            } else if (method == "POST") {
                req.method_ = Request::MethodEnum::POST;
            } else if (method == "DELETE") {
                req.method_ = Request::MethodEnum::DELETE;
            }
            req.uri_ = uri;
            std::string version = "HTTP/" + std::to_string(http_version_major) + "." + std::to_string(http_version_minor);
            req.version_ = version;
            for (int i = 0; i < headers.size(); i++) {
                header h = headers[i];
                req.headers_[h.name] = h.value;
            }
            std::string body_string(body.begin(), body.end());
            req.body_ = body_string;
            return req;
        }
};

#endif // HTTP_REQUESTBUILDER_HPP
