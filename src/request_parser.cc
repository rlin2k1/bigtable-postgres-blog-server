/* request_parser.cc
Description:
    Parser for Client HTTP requests to the server.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Sources taken from:
https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_parser.cpp
https://github.com/nekipelov/httpparser

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#include <strings.h>
#include "request_builder.h"
#include "request_parser.h"
#include "iostream"

/* Constructor */
request_parser::request_parser()
  : state_(method_start), contentsize_(0)
{}

void request_parser::reset() {
  state_ = method_start;
}

/* NOTE: Function is called in request_parser.h */
/* std::string request_dispatcher::longest_prefix_match(std::string uri)
Parameter(s):
    - req: Request builder object which stores values from the HTTP request.
    - input: 
Returns:
    - Result type which determines if the request is either good, bad, or indeterminate.
Description: 
    - Parses received request to determine if it is syntactically valid. */
request_parser::result_type request_parser::consume(request_builder& req, char input) {
  req.fullmessage.push_back(input);
  switch (state_) {
  case method_start:
    if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
      return bad;
    } else {
      state_ = method;
      req.method.push_back(input);
      return indeterminate;
    }
  case method:
    if (input == ' ') {
      state_ = uri;
      return indeterminate;
    } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
      return bad;
    } else {
      req.method.push_back(input);
      return indeterminate;
    }
  case uri:
    if (input == ' ') {
      state_ = http_version_h;
      return indeterminate;
    } else if (is_ctl(input)) {
      return bad;
    } else {
      req.uri.push_back(input);
      return indeterminate;
    }
  case http_version_h:
    if (input == 'H') {
      state_ = http_version_t_1;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_t_1:
    if (input == 'T') {
      state_ = http_version_t_2;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_t_2:
    if (input == 'T') {
      state_ = http_version_p;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_p:
    if (input == 'P') {
      state_ = http_version_slash;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_slash:
    if (input == '/') {
      req.http_version_major = 0;
      req.http_version_minor = 0;
      state_ = http_version_major_start;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_major_start:
    if (is_digit(input)) {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      state_ = http_version_major;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_major:
    if (input == '.') {
      state_ = http_version_minor_start;
      return indeterminate;
    } else if (is_digit(input)) {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_minor_start:
    if (is_digit(input)) {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      state_ = http_version_minor;
      return indeterminate;
    } else {
      return bad;
    }
  case http_version_minor:
    if (input == '\r') {
      state_ = expecting_newline_1;
      return indeterminate;
    } else if (is_digit(input)) {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      return indeterminate;
    } else {
      return bad;
    }
  case expecting_newline_1:
    if (input == '\n') {
      state_ = header_line_start;
      return indeterminate;
    } else {
      return bad;
    }
  case header_line_start:
    if (input == '\r') {
      state_ = expecting_newline_3;
      return indeterminate;
    } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
      state_ = header_lws;
      return indeterminate;
    } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
      return bad;
    } else {
      req.headers.push_back(header());
      req.headers.back().name.push_back(input);
      state_ = header_name;
      return indeterminate;
    }
  case header_lws:
    if (input == '\r') {
      state_ = expecting_newline_2;
      return indeterminate;
    } else if (input == ' ' || input == '\t') {
      return indeterminate;
    } else if (is_ctl(input)) {
      return bad;
    } else {
      state_ = header_value;
      req.headers.back().value.push_back(input);
      return indeterminate;
    }
  case header_name:
    if (input == ':') {
      state_ = space_before_header_value;
      return indeterminate;
    } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
      return bad;
    } else {
      req.headers.back().name.push_back(input);
      return indeterminate;
    }
  case space_before_header_value:
    if (input == ' ') {
      state_ = header_value;
      return indeterminate;
    } else {
      return bad;
    }
  case header_value:
    if (input == '\r') {
      std::string current_header = req.headers.back().name;
      if (strcasecmp(current_header.c_str(), "Content-Length") == 0) {
        try {
          req.bodysize = std::stoi(req.headers.back().value);
          contentsize_ = std::stoi(req.headers.back().value);
          req.body.reserve(req.bodysize);
        } catch (std::exception e) {
          return bad;
        }
      }

      if ( (strcasecmp(current_header.c_str(), "Connection") == 0) &&\
      (strcasecmp(req.headers.back().value.c_str(), "Keep-Alive") == 0) ) {
      req.keep_alive = true;
      }
      state_ = expecting_newline_2;
      return indeterminate;
    } else if (is_ctl(input)) {
      return bad;
    } else {
      req.headers.back().value.push_back(input);
      return indeterminate;
    }
  case expecting_newline_2:
    if (input == '\n') {
      state_ = header_line_start;
      return indeterminate;
    } else {
      return bad;
    }
  case expecting_newline_3:
    if (contentsize_ == 0) {
      return (input == '\n') ? good : bad;
    } else {
      state_ = expecting_body;
      return indeterminate;
    }
  case expecting_body:
    --contentsize_;
    req.body.push_back(input);
    if (contentsize_ == 0) {
      return good;
    } else {
      return indeterminate;
    }

  default:
    return bad;
  }
}

/* Functions below allow the request parser to easily distinguish
   character types and which are considered valid in HTTP requests. */
bool request_parser::is_char(int c) {
  return c >= 0 && c <= 127;
}

bool request_parser::is_ctl(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}

bool request_parser::is_tspecial(int c) {
  switch (c) {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}

bool request_parser::is_digit(int c) {
  return c >= '0' && c <= '9';
}
