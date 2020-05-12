/* response_helper_library.cc
Helper library for Response class.

Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin

Date Created:
    May 12th, 2020
*/

#include <string>
#include "response_helper_library.h"

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings


boost::asio::const_buffer ResponseHelperLibrary::to_buffer(http::server::Response::StatusCode status) {
    switch (status) {
    case http::server::Response::ok:
      return boost::asio::buffer(http::server::status_strings::ok);
    case http::server::Response::bad_request:
      return boost::asio::buffer(http::server::status_strings::bad_request);
    case http::server::Response::not_found:
      return boost::asio::buffer(http::server::status_strings::not_found);
    default:
      return boost::asio::buffer(http::server::status_strings::bad_request);
    }
}

std::vector<boost::asio::const_buffer> ResponseHelperLibrary::to_buffers(http::server::Response& response) {
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(to_buffer(response.code_));

    std::map<std::string, std::string>::iterator it;
    for ( it = response.headers_.begin(); it != response.headers_.end(); it++ )
    {
      buffers.push_back(boost::asio::buffer(it->first));
      buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
      buffers.push_back(boost::asio::buffer(it->second));
      buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    }
    buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    buffers.push_back(boost::asio::buffer(response.body_));
    return buffers;
}

std::string ResponseHelperLibrary::to_string(http::server::Response::StatusCode status) {
    switch (status) {
      case http::server::Response::bad_request: {
        return http::server::stock_responses::bad_request;
      }
      case http::server::Response::not_found: {
        return http::server::stock_responses::not_found;
      }
      default:
        return http::server::stock_responses::bad_request;
    }
  }

http::server::Response ResponseHelperLibrary::stock_response(http::server::Response::StatusCode status) {
    http::server::Response response;

    response.code_ = status;
    response.body_ = to_string(status);
    response.headers_["Content-Length"] = std::to_string(response.body_.size());
    response.headers_["Content-Type"] = "text/html";
    return response;
  }
