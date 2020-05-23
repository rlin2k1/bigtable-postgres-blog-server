/* response_builder.h
Header file for the response struct builder.

Library Source modified from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_parser.cpp

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 23rd, 2020
*/

#ifndef HTTP_RESPONSEBUILDER_HPP
#define HTTP_RESPONSEBUILDER_HPP

#include <string>
#include <vector>
#include "header.h"
#include "response.h"

/// A response received from a client.
class response_builder {
    public:
        int http_version_major;
        int http_version_minor;
        int code;
        std::vector<char> reasonphrase;
        std::vector<header> headers;
        std::vector<char> body;
        std::vector<char> fullmessage;
        bool keep_alive = false;
        bool chunked = false;
        int contentsize;
        std::vector<char> chunksizehex;
        int chunksize;
        Response build_response() {
            Response res;
            // Undefined behavior if code not in StatusCode enum.
            res.code_ = static_cast<Response::StatusCode>(code);
            for (int i = 0; i < headers.size(); i++) {
                header h = headers[i];
                res.headers_[h.name] = h.value;
            }
            std::string body_string(body.begin(), body.end());
            res.body_ = body_string;
            return res;
        }
};

#endif // HTTP_RESPONSEBUILDER_HPP
