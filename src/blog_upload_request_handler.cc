/* blog_upload_request_handler.cc
Description:
  Request handler for request to get the html form to submit blog entries

Author(s):
  Kubilay Agi
  Michael Gee
  Jane Lee
  Roy Lin

Date Created:
  June 4th, 2020
*/
#include <fstream>
#include <sstream>
#include <string>
#include <boost/log/trivial.hpp>
#include "blog_upload_request_handler.h"
#include "response_helper_library.h"
#include "request.h"

blog_upload_request_handler* blog_upload_request_handler::Init(const std::string& location_path, const NginxConfig& config) {
  blog_upload_request_handler* burh = new blog_upload_request_handler();
  const std::string database_name = "postgres";
  std::string username = config.blog_usernames_.at(location_path);
  std::string password = config.blog_passwords_.at(location_path);
  std::string host = config.blog_ips_.at(location_path);
  std::string port_num = config.blog_ports_.at(location_path);
  burh->bd_ = new blog_database(database_name, username, password, host, port_num);
  burh->location_prefix_ = location_path;
  return burh;
}

// Constructor
blog_upload_request_handler::blog_upload_request_handler() {}

// Constructor
blog_upload_request_handler::blog_upload_request_handler(const std::string& location_path, database* db) {
  bd_ = db;
  location_prefix_ = location_path;
}

blog_upload_request_handler::~blog_upload_request_handler() {
  delete bd_;
  bd_ = NULL;
}

Response blog_upload_request_handler::handle_request(const Request& request) {
  BOOST_LOG_TRIVIAL(info) << "[ResponseMetrics]Request_Handler: Blog Post Upload";
  BOOST_LOG_TRIVIAL(info) << "Sending back blog upload form.";
  Response response_;
  std::string decoded_str = urldecode(request.body_);
  form_to_value_ = parseRequestBody(decoded_str);

  if (request.method_ == Request::MethodEnum::GET) {
    std::string remain_uri = request.uri_.substr(location_prefix_.size());
    if ( (location_prefix_ + "/").find(request.uri_) != std::string::npos) {
      response_ = handle_get_all_blogs();
    }
    // If user enters unparsable id, return bad id error page to client
    else if (remain_uri.size() <= 1 || !is_number(remain_uri.substr(1))) {
      response_ = handle_get_one_blog(-1);
    } else {  // Use handle_get to check whether id can be gathered from database
      remain_uri = remain_uri.substr(1);
      response_ = handle_get_one_blog(stoi(remain_uri));
    }
  } else {
    response_ = handle_post_blog(form_to_value_["submissiontitle"], form_to_value_["submissionbody"]);
  }
  return response_;
}

// Check if all characters are digits
bool blog_upload_request_handler::is_number(const std::string& enter_string) {
  if (enter_string.empty()) {
    return false;
  }
  for (int i = 0; i < enter_string.size(); i++) {
    if (!std::isdigit(enter_string[i]))
      return false;
  }
  return true;
}
std::string blog_upload_request_handler::getLocationPrefix() {
  return location_prefix_;
}

// Handle get request by entering your id
Response blog_upload_request_handler::handle_get_one_blog(int postid) {
  Blog blog = bd_->get_blog(postid);

  std::string html_body_get_response = "<!DOCTYPE html>\n\
<html>\n\
    <head>\n\
        <meta charset='utf-8'>\n\
        <title>Blog Post</title>\n\
    </head>\n\
    <body style=\"text-align:center;\">\n\
      <h1>\n";

  if (blog.postid < 0) {
    html_body_get_response += "Error: Unable to get blog entry from id.";
    html_body_get_response += "</h1>\n";
    html_body_get_response += "<p>\nPlease use a valid id number</p>\n\n";
    html_body_get_response += "</body>\n</html>\n";
  } else {
    html_body_get_response += blog.title + "</h1>\n";
    html_body_get_response += "<p>\n" + blog.body + "</p>\n";
    html_body_get_response += "</body>\n<div style=\"position: fixed;bottom: 0;right: 0;\">POSTID: " + std::to_string(blog.postid) + "</div></html>\n";
  }

  Response response;
  response.code_ = Response::ok;
  response.body_ = html_body_get_response;
  response.headers_["Content-Length"] = std::to_string(response.body_.size());
  response.headers_["Content-Type"] = "text/html";
  return response;
}

Response blog_upload_request_handler::handle_get_all_blogs() {
  std::string html_body_get_response = "<!DOCTYPE html>\n\
<html>\n\
    <head>\n\
        <meta charset='utf-8'>\n\
        <title>All Blogs</title>\n\
    </head>\n\
    <body style=\"text-align:center;\">\n";
  std::vector<Blog> blogs = bd_->get_all_blogs();
  for (auto vectorit = blogs.begin(); vectorit != blogs.end(); ++vectorit) {
    html_body_get_response += "<div>*****POSTID: " + std::to_string((*vectorit).postid) +  "*****</div>";
    html_body_get_response += "<h1>\n" + (*vectorit).title + "</h1>\n";
    html_body_get_response += "<p>\n" + (*vectorit).body + "</p>\n";
    html_body_get_response += "<div>-------------------------------------------------------------------------------------</div>";
  }
  html_body_get_response += "</body>\n</html>\n";

  Response response;
  response.code_ = Response::ok;
  response.body_ = html_body_get_response;
  response.headers_["Content-Length"] = std::to_string(response.body_.size());
  response.headers_["Content-Type"] = "text/html";
  return response;
}

// Handle post requests after you submit your form
Response blog_upload_request_handler::handle_post_blog(std::string title, std::string body) {
  int postid = bd_->insert_blog(title, body);

  Response response;
  response.code_ = Response::moved_temporarily;
  response.headers_["Content-Length"] = std::to_string(response.body_.size());
  response.headers_["Content-Type"] = "text/html";
  // Redirected to new location
  if (location_prefix_[location_prefix_.size() - 1] == '/') {
    response.headers_["Location"] = location_prefix_ + std::to_string(postid);
  } else {
    response.headers_["Location"] = std::string(location_prefix_) + "/" + std::to_string(postid);
  }
  return response;
}

// http://dlib.net/dlib/server/server_http.cpp.html
unsigned char blog_upload_request_handler::from_hex(unsigned char ch) {
  if (ch <= '9' && ch >= '0')
      ch -= '0';
  else if (ch <= 'f' && ch >= 'a')
      ch -= 'a' - 10;
  else if (ch <= 'F' && ch >= 'A')
      ch -= 'A' - 10;
  else
      ch = 0;
  return ch;
}

// http://dlib.net/dlib/server/server_http.cpp.html
// Decode URL for the database
std::string blog_upload_request_handler::urldecode (const std::string& str) {
  using namespace std;
  string result;
  string::size_type i;
  for (i = 0; i < str.size(); ++i) {
      // Replaces +'s with spaces
      if (str[i] == '+') {
        result += ' ';
      } else if (str[i] == '%' && str.size() > i+2) {  // Replace percent encoding to ascii characters
        const unsigned char ch1 = from_hex(str[i+1]);
        const unsigned char ch2 = from_hex(str[i+2]);
        const unsigned char ch = (ch1 << 4) | ch2;
        result += ch;
        i += 2;
      } else {
        result += str[i];
      }
  }
  return result;
}

// Parse the request body so that we can store in the database properly
std::map<std::string, std::string> blog_upload_request_handler::parseRequestBody(std::string body) {
  std::map <std::string, std::string> temp;
  std::string key = "";
  std::string value = "";
  int counter = 0;
  bool atkey = true;
  while (counter < body.length()) {
    if (body[counter] == '=') {
      atkey = false;
      counter++;
      continue;
    } else if (body[counter] == '&') {
      atkey = true;
      temp[key] = value;
      counter++;
      key = "";
      value = "";
      continue;
    }
    if (atkey) {
      key += body.at(counter);
    } else {
      value += body.at(counter);
    }
    counter++;
  }
  temp[key] = value;
  return temp;
}
