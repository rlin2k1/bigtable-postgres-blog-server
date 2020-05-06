/* reply.cc
Reply creation for client reception.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/reply.cpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#include "reply.h"
#include <string>

namespace http {
namespace server {
namespace status_strings {
  boost::asio::const_buffer to_buffer(reply::status_type status) {
    switch (status) {
    case reply::ok:
      return boost::asio::buffer(http::server::status_strings::ok);
    case reply::bad_request:
      return boost::asio::buffer(http::server::status_strings::bad_request);
    case reply::not_found:
      return boost::asio::buffer(http::server::status_strings::not_found);
    default:
      return boost::asio::buffer(http::server::status_strings::bad_request);
    }
  }
}  // namespace status_strings

  std::vector<boost::asio::const_buffer> reply::to_buffers() {
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(status_strings::to_buffer(status));
    for (std::size_t i = 0; i < headers.size(); ++i) {
      header& h = headers[i];
      buffers.push_back(boost::asio::buffer(h.name));
      buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
      buffers.push_back(boost::asio::buffer(h.value));
      buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    }
    buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    buffers.push_back(boost::asio::buffer(content));
    return buffers;
  }

namespace stock_replies {
  std::string to_string(reply::status_type status) {
    switch (status) {
      case reply::bad_request: {
        return http::server::stock_replies::bad_request;
      }
      case reply::not_found: {
        return http::server::stock_replies::not_found;
      }
      default:
        return http::server::stock_replies::bad_request;
    }
  }
}  // namespace stock_replies

  reply reply::stock_reply(reply::status_type status) {
    reply rep;
    rep.status = status;
    rep.content = stock_replies::to_string(status);
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/html";
    return rep;
  }
}  // namespace server
}  // namespace http
