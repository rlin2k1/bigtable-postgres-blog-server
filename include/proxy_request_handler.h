/* proxy_request_handler.h
Header file for handling requests with any configured reverse proxy uri.

Author(s):
    Daniel Ahn
    Rafael Ning
    David Padlipsky
    Andy Zeff

Date Created:
    May 20th, 2020
*/

#ifndef HTTP_PROXY_REQUEST_HANDLER_HPP
#define HTTP_PROXY_REQUEST_HANDLER_HPP

#include <string>
#include <unordered_map>
#include <boost/log/trivial.hpp>
#include <libxml2/libxml/HTMLparser.h>

#include "request_handler.h"
#include "config_parser.h"

class proxy_request_handler: public request_handler {
 public: // API uses public member functions
    static proxy_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);
 private:
    Response proxy_request(const Request& request, std::string uri, std::string url, int port);
    void modify_html_doc(xmlNode *node);
    void handle_property(xmlNode *node, const char *property);
    Response handle_html(Response& response);
    bool read_response(boost::asio::ip::tcp::socket& socket, Response &response);
    std::string build_request_string(const Request& request);
    std::string decompress_gzip(std::string compressed);
    Response get_error_response();

    enum { max_length = 1024 };
    std::string client_location_path_;
    std::string server_url_;
    std::string server_location_path_;
    int server_port_num;
};

#endif  // INCLUDE_PROXY_REQUEST_HANDLER_H_
