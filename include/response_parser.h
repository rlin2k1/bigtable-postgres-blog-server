/* response_parser.h
Header file for parsing incoming responses.

Library Source modified from https://www.boost.org/doc/libs/1_65_1/doc/html/boost_asio/example/cpp11/http/server/request_parser.cpp

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 23rd, 2020
*/

#ifndef HTTP_RESPONSE_PARSER_HPP
#define HTTP_RESPONSE_PARSER_HPP

#include <tuple>

struct response_builder;

class response_parser {
    public:
        /// Construct ready to parse the response status line.
        response_parser();

        /// Reset to initial parser state.
        void reset();

        /// Result of parse.
        enum result_type { good, bad, indeterminate };

        /// Parse some data. The enum return value is good when a complete request has
        /// been parsed, bad if the data is invalid, indeterminate when more data is
        /// required. The InputIterator return value indicates how much of the input
        /// has been consumed.
        template <typename InputIterator>
        std::tuple<result_type, InputIterator> parse(response_builder& res,
            InputIterator begin, InputIterator end) {
          result_type result = indeterminate;
          while (begin != end) {
            result = consume(res, *begin++);
            if (result == bad) {
              return std::make_tuple(result, begin);
            } if (result == good && res.chunked && res.chunksize == 0) {
              return std::make_tuple(result, begin);
            }
          }
          return std::make_tuple(result, begin);
        }

    private:
        /// Handle the next character of input.
        result_type consume(response_builder& res, char input);

        /// Check if a byte is an HTTP character.
        static bool is_char(int c);

        /// Check if a byte is an HTTP control character.
        static bool is_ctl(int c);

        /// Check if a byte is defined as an HTTP tspecial character.
        static bool is_tspecial(int c);

        /// Check if a byte is a digit.
        static bool is_digit(int c);

        /// Check if a byte is a hex char.
        static bool is_hex(int c);

        /// The current state of the parser.
        enum state {
          http_version_h,
          http_version_t_1,
          http_version_t_2,
          http_version_p,
          http_version_slash,
          http_version_major_start,
          http_version_major,
          http_version_minor_start,
          http_version_minor,
          status_code_1,
          status_code_2,
          status_code_3,
          space_before_reason_phrase,
          reason_phrase,
          expecting_newline_1,
          header_line_start,
          header_lws,
          header_name,
          space_before_header_value,
          header_value,
          expecting_newline_2,
          expecting_newline_3,
          expecting_body,
          chunk_size_start,
          chunk_size,
          expecting_newline_4,
          expecting_chunk,
          expecting_newline_5
        } state_;
};

#endif // HTTP_RESPONSE_PARSER_HPP
