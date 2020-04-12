//
// request_handler.cc
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_handler.cpp

#include <fstream>
#include <sstream>
#include <string>

#include "request.h"
#include "request_handler.h"
#include "reply.h"

namespace http {
namespace server {

request_handler::request_handler(){}

void request_handler::handle_request(const request& req, reply& rep,  const char *data) {
  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;

  rep.content = data;

  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = "text/plain";
}
} // namespace server
} // namespace http
