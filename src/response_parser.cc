/* response_parser.cc
Description:
    Parser for HTTP responses to the server.

Library Source modified from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_parser.cpp

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 23rd, 2020
*/

#include <strings.h>
#include "response_builder.h"
#include "response_parser.h"

/* Constructor */
response_parser::response_parser()
  : state_(http_version_h)
{}

void response_parser::reset() {
  state_ = http_version_h;
}

/* response_parser::result_type response_parser::consume(request_builder& req, char input) {
Parameter(s):
    - res: Response builder object which stores values from the HTTP response.
    - input: Character to be consumed to cause state change.
Returns:
    - Result type which determines if the response is either good, bad, or indeterminate.
Description:
    - Parses received response to determine if it is syntactically valid. */
response_parser::result_type response_parser::consume(response_builder& res, char input) {
    res.fullmessage.push_back(input);
    switch (state_) {
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
            res.http_version_major = 0;
            res.http_version_minor = 0;
            state_ = http_version_major_start;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_major_start:
        if (is_digit(input)) {
            res.http_version_major = res.http_version_major * 10 + input - '0';
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
            res.http_version_major = res.http_version_major * 10 + input - '0';
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor_start:
        if (is_digit(input)) {
            res.http_version_minor = res.http_version_minor * 10 + input - '0';
            state_ = http_version_minor;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor:
        if (input == ' ') {
            state_ = status_code_1;
            return indeterminate;
        } else if (is_digit(input)) {
            res.http_version_minor = res.http_version_minor * 10 + input - '0';
            return indeterminate;
        } else {
            return bad;
        }
    case status_code_1:
        if (input >= '1' && input <= '5') {
            state_ = status_code_2;
            res.code = input - '0';
            return indeterminate;
        } else {
          return bad;
        }
    case status_code_2:
        if (is_digit(input)) {
            state_ = status_code_3;
            res.code = res.code * 10 + input - '0';
            return indeterminate;
        } else {
          return bad;
        }
    case status_code_3:
        if (is_digit(input)) {
            state_ = space_before_reason_phrase;
            res.code = res.code * 10 + input - '0';
            return indeterminate;
        } else {
            return bad;
        }
    case space_before_reason_phrase:
        if (input == ' ') {
            state_ = reason_phrase;
            return indeterminate;
        } else {
            return bad;
        }
    case reason_phrase:
        if (input == '\r') {
            state_ = expecting_newline_1;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || (is_tspecial(input) && input != ' ')) {
            return bad;
        } else {
            res.reasonphrase.push_back(input);
            return indeterminate;
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
        } else if (!res.headers.empty() && (input == ' ' || input == '\t')) {
            state_ = header_lws;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            res.headers.push_back(header());
            res.headers.back().name.push_back(input);
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
            res.headers.back().value.push_back(input);
            return indeterminate;
        }
    case header_name:
        if (input == ':') {
            state_ = space_before_header_value;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            res.headers.back().name.push_back(input);
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
            std::string current_header = res.headers.back().name;

            if (strcasecmp(current_header.c_str(), "Content-Length") == 0) {
                try {
                    res.contentsize = std::stoi(res.headers.back().value);
                    res.body.reserve(res.contentsize);
                } catch (std::exception e) {
                    return bad;
                }
            }

            if ( (strcasecmp(current_header.c_str(), "Transfer-Encoding") == 0) &&
                 (strcasecmp(res.headers.back().value.c_str(), "chunked") == 0) ) {
                res.chunked = true;
            }

            if ( (strcasecmp(current_header.c_str(), "Connection") == 0) &&
                 (strcasecmp(res.headers.back().value.c_str(), "Keep-Alive") == 0) ) {
                res.keep_alive = true;
            }

            state_ = expecting_newline_2;
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            res.headers.back().value.push_back(input);
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
        if (input != '\n') {
          return bad;
        } else if (res.chunked) {
            state_ = chunk_size_start;
            return indeterminate;
        } else if (res.contentsize == 0) {
            return (input == '\n') ? good : bad;
        } else {
            state_ = expecting_body;
            return indeterminate;
        }
    case expecting_body:
        --(res.contentsize);
        res.body.push_back(input);
        if (res.contentsize == 0) {
            return good;
        } else {
            return indeterminate;
        }
    case chunk_size_start:
        if (is_hex(input)) {
            state_ = chunk_size;
            res.chunksizehex.clear();
            res.chunksizehex.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case chunk_size:
        if (is_hex(input)) {
            res.chunksizehex.push_back(input);
            return indeterminate;
        } else if (input == '\r') { // Does not support chunk extension.
            state_ = expecting_newline_4;
            try {
                std::string chunksizehex_string(res.chunksizehex.begin(), res.chunksizehex.end());
                res.chunksize = std::stoi(chunksizehex_string, 0, 16);
            } catch (std::exception e) {
                return bad;
            }
            if (res.chunksize == 0) {
              return good; // Does not support chunk trailer.
            } else {
              return indeterminate;
            }
        } else {
            return bad;
        }
    case expecting_newline_4:
        if (input == '\n') {
            state_ = expecting_chunk;
            return indeterminate;
        } else {
            return bad;
        }
    case expecting_chunk:
        --(res.chunksize);
        if (input == '\r' && res.chunksize == -1) {
            state_ = expecting_newline_5;
        } else {
          res.body.push_back(input);
        }
        return indeterminate;
    case expecting_newline_5:
        if (input == '\n') {
            state_ = chunk_size_start;
            return indeterminate;
        } else {
            return bad;
        }
    default:
        return bad;
    }
}

/* Functions below allow the request parser to easily distinguish
   character types and which are considered valid in HTTP requests. */
bool response_parser::is_char(int c) {
    return c >= 0 && c <= 127;
}

bool response_parser::is_ctl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

bool response_parser::is_tspecial(int c) {
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

bool response_parser::is_digit(int c) {
    return c >= '0' && c <= '9';
}

bool response_parser::is_hex(int c) {
    return is_digit(c)
        || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F'
        || c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f';
}
