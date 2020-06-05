/* blog_upload_request_handler.h
Header file for handling requests with the /upload uri.
Author(s):
    Kubilay Agi
    Michael Gee
    Jane Lee
    Roy Lin
Date Created:
    June 4th, 2020
*/
#ifndef HTTP_BLOG_UPLOAD_REQUEST_HANDLER_HPP
#define HTTP_BLOG_UPLOAD_REQUEST_HANDLER_HPP
#include <string>
#include "request_handler.h"
#include "config_parser.h"
#include "blog_database.h"


class blog_upload_request_handler: public request_handler {
 public:
    static blog_upload_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);
    std::map<std::string, std::string> parseRequestBody(std::string body);
    std::string urldecode(const std::string & sSrc);
    unsigned char from_hex (unsigned char ch);
    Response handle_post(std::string title, std::string body, std::string host, std::string port_num);
    Response handle_get(int id, std::string host, std::string port_num);
    bool is_number(const std::string& enter_string);
    ~blog_upload_request_handler();
 private:
    std::map<std::string, std::string> form_to_value;
    std::string server_host;
    std::string server_port_num;
    std::string location_prefix_;
    blog_database* bd;
};
#endif  // HTTP_BLOG_UPLOAD_REQUEST_HANDLER_HPP
