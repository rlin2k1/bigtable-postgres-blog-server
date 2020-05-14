/* request_parser.h
Header file for parsing incoming requests.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_parser.hpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include <tuple>

struct request_builder;

class request_parser {
public:
  /// Construct ready to parse the request method.
  request_parser();

  /// Reset to initial parser state.
  void reset();

  /// Result of parse.
  enum result_type { good, bad, indeterminate };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(request_builder& req,
      InputIterator begin, InputIterator end) {
    while (begin != end)
    {
      result_type result = consume(req, *begin++);
      // TODO(Kubilay Agi): Doesn't handle the case where a client sends multiple consecutive packets
      if ((result == good && (begin == end)) || result == bad) {
          return std::make_tuple(result, begin);
      } else if ((result == good && (begin != end))) {
        // Request header is good, but unexpected input after request,
        // then signal bad request
          return std::make_tuple(bad, begin);
      }
    }
    return std::make_tuple(indeterminate, begin);
  }

private:
  /// Handle the next character of input.
  result_type consume(request_builder& req, char input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  /// The current state of the parser.
  enum state {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    expecting_body,
  } state_;

  int contentsize_;
};

#endif // HTTP_REQUEST_PARSER_HPP
