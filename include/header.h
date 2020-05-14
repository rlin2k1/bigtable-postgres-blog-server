/* header.h
Header file for HTTP Request and Response headers.

Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

Library Source taken from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/header.hpp

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    April 11th, 2020
*/

#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>

struct header {
  std::string name;
  std::string value;
};

inline bool operator==(const header& lhs, const header& rhs) {
    return (lhs.name == rhs.name) && (lhs.value == rhs.value);
}

#endif // HTTP_HEADER_HPP
