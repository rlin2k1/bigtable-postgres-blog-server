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
#include "database.h"
#include "blog_database.h"

class blog_upload_request_handler: public request_handler {
 public:
    blog_upload_request_handler();
    blog_upload_request_handler(const std::string& location_path, database* db);
    ~blog_upload_request_handler();
    static blog_upload_request_handler* Init(const std::string& location_path, const NginxConfig& config);
    virtual Response handle_request(const Request& request);
    std::string getLocationPrefix();

 private:
    Response handle_get_one_blog(int id);
    Response handle_get_all_blogs();
    Response handle_post_blog(std::string title, std::string body);

    std::map<std::string, std::string> parseRequestBody(std::string body);
    std::string urldecode(const std::string & sSrc);
    unsigned char from_hex (unsigned char ch);
    bool is_number(const std::string& enter_string);

    std::map<std::string, std::string> form_to_value_;
    std::string location_prefix_;
    database* bd_;
};
#endif  // HTTP_BLOG_UPLOAD_REQUEST_HANDLER_HPP
