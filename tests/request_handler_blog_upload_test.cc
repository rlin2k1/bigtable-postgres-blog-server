#include "gtest/gtest.h"
#include "blog_upload_request_handler.h"
#include "config_parser.h"
#include "request.h"
#include "mock_database.h"

class Blog_Upload_Request_Handler_Test : public ::testing::Test {
 protected:
  blog_upload_request_handler* blog_upload_request_handler_;
  Request request_;
  NginxConfig config;
  mock_database* md = new mock_database;

  void SetUp() override {
    NginxConfigParser parser;
    parser.Parse("blog_upload_test_request_config", &config);
  }

  void SetBlogHandler(std::string path, std::string uri, Request::MethodEnum method) {
    blog_upload_request_handler_ = new blog_upload_request_handler(path, md);
    request_.method_ = method;
    request_.uri_ = path + uri;
    request_.version_ = "HTTP/1.1";
  }

  virtual ~Blog_Upload_Request_Handler_Test() { 
    delete blog_upload_request_handler_;
    delete md;
    blog_upload_request_handler_ = NULL;
    md = NULL;
  }
};

TEST_F(Blog_Upload_Request_Handler_Test, GetBlogUploadFailureTest) {
  // Set up request for get handle
  std::string path = "/blog";
  SetBlogHandler(path, "/we", Request::MethodEnum::GET);

  // Unable to get id body response
  std::string html_body_get_response = "<!DOCTYPE html>\n\
<html>\n\
    <head>\n\
        <meta charset='utf-8'>\n\
        <title>Blog Upload Form</title>\n\
    </head>\n\
    <body style=\"text-align:center;\">\n\
      <h1>\n";
  html_body_get_response += "Error: Unable to get blog entry from id.";
  html_body_get_response += "</h1>\n";
  html_body_get_response += "<p>\nPlease use a valid id number</p>\n\n";
  html_body_get_response += "</body>\n\
  </html>\n";

  Response test = blog_upload_request_handler_->handle_request(request_);
  std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(test.body_.size())}, {"Content-Type", "text/html"} };
  EXPECT_EQ(test.code_, Response::ok);
  EXPECT_EQ(test.body_, html_body_get_response);
  EXPECT_EQ(test.headers_, response_header);
}

TEST_F(Blog_Upload_Request_Handler_Test, POSTBlogUploadSuccessTest) {
  //***** Check that POST method properly inserts blog entry *********
  std::string path = "/blog";
  SetBlogHandler(path, "", Request::MethodEnum::POST);
  const std::string entry_number = "1";

  Response test = blog_upload_request_handler_->handle_request(request_);
  std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(test.body_.size())}, {"Content-Type", "text/html"}, {"Location", std::string(blog_upload_request_handler_->getLocationPrefix()) + "/" + entry_number} };
  EXPECT_EQ(test.code_, Response::moved_temporarily);
  EXPECT_EQ(test.headers_, response_header);

  //***** Check that GET method works after inserting blog to mock database *********
  SetBlogHandler(path, "/" + entry_number, Request::MethodEnum::GET);

  Response test_2 = blog_upload_request_handler_->handle_request(request_);
  std::string html_body_get_response = "<!DOCTYPE html>\n\
<html>\n\
    <head>\n\
        <meta charset='utf-8'>\n\
        <title>Blog Upload Form</title>\n\
    </head>\n\
    <body style=\"text-align:center;\">\n\
      <h1>\n";
  html_body_get_response += "Blog Entry " + entry_number + "\n\n";
  html_body_get_response += "</h1>\n";
  html_body_get_response += std::string("<p>\n") + "</p>\n\n";
  html_body_get_response += std::string("<p>\n") + "</p>\n";
  html_body_get_response += "</body>\n\
  </html>\n";

  std::map<std::string, std::string> response_header_2 = { {"Content-Length", std::to_string(test_2.body_.size())}, {"Content-Type", "text/html"} };
  EXPECT_EQ(test_2.code_, Response::ok);
  EXPECT_EQ(test_2.body_, html_body_get_response);
  EXPECT_EQ(test_2.headers_, response_header_2);
}
