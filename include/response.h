/* response.h
Header file for creating HTTP responses.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/reply.hpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <map>

/// A Response to be sent to a client.
class Response {
  public:
    // An HTML code indicating success/failure of processing
    /// The status of the Response.
    enum StatusCode {
      ok = 200,
      created = 201,
      accepted = 202,
      no_content = 204,
      multiple_choices = 300,
      moved_permanently = 301,
      moved_temporarily = 302,
      not_modified = 304,
      bad_request = 400,
      unauthorized = 401,
      forbidden = 403,
      not_found = 404,
      internal_server_error = 500,
      not_implemented = 501,
      bad_gateway = 502,
      service_unavailable = 503
    } code_;

    // A map of headers, for convenient lookup ("Content-Type", "Cookie", etc)
    std::map<std::string, std::string> headers_;

    // The content of the response
    std::string body_;
};

#endif // HTTP_RESPONSE_HPP
