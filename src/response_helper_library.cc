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

}  // namespace misc_strings


boost::asio::const_buffer ResponseHelperLibrary::to_buffer(Response::StatusCode status) {
    switch (status) {
    case Response::ok:
      return boost::asio::buffer(status_strings::ok);
    case Response::bad_request:
      return boost::asio::buffer(status_strings::bad_request);
    case Response::not_found:
      return boost::asio::buffer(status_strings::not_found);
    default:
      return boost::asio::buffer(status_strings::bad_request);
    }
}

std::vector<boost::asio::const_buffer> ResponseHelperLibrary::to_buffers(Response& response) {
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(to_buffer(response.code_));

    std::map<std::string, std::string>::iterator it;
    for ( it = response.headers_.begin(); it != response.headers_.end(); it++) {
      buffers.push_back(boost::asio::buffer(it->first));
      buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
      buffers.push_back(boost::asio::buffer(it->second));
      buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    }
    buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    buffers.push_back(boost::asio::buffer(response.body_));
    return buffers;
}

std::string ResponseHelperLibrary::to_string(Response::StatusCode status) {
  switch (status) {
    case Response::bad_request: {
      return stock_responses::bad_request;
    }
    case Response::not_found: {
      return stock_responses::not_found;
    }
    default:
      return stock_responses::bad_request;
  }
}

Response ResponseHelperLibrary::stock_response(Response::StatusCode status) {
  Response response;

  response.code_ = status;
  response.body_ = to_string(status);
  response.headers_["Content-Length"] = std::to_string(response.body_.size());
  response.headers_["Content-Type"] = "text/html";
  return response;
}
