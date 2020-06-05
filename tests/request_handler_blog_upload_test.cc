#include "gtest/gtest.h"
#include "blog_upload_request_handler.h"
#include "config_parser.h"
#include "request.h"

class Blog_Upload_Request_Handler_Test : public ::testing::Test {
 protected:
  std::unique_ptr<blog_upload_request_handler> blog_upload_request_handler_;
  Request request_;
  NginxConfig config;
  void SetUp() override {
    NginxConfigParser parser;
    parser.Parse("blog_upload_test_request_config", &config);
  }

  void SetGetBlogHandler(std::string path, std::string uri) {
    blog_upload_request_handler_.reset(blog_upload_request_handler::Init(path, config));
    request_.method_ = Request::MethodEnum::GET;
    request_.uri_ = path + uri;
    request_.version_ = "HTTP/1.1";
  }
  void SetPostBlogHandler(std::string path) {
    blog_upload_request_handler_.reset(blog_upload_request_handler::Init(path, config));
    request_.method_ = Request::MethodEnum::POST;
    request_.uri_ = path;
    request_.version_ = "HTTP/1.1";
  }
};

TEST_F(Blog_Upload_Request_Handler_Test, GetBlogUploadFailureTest) {
  // Set up request for get handle
  std::string path = "/blog";
  SetGetBlogHandler(path, "/we");

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

// TODO(Michael): Add functionality and other header values/body values when Roy finishes mock database
TEST_F(Blog_Upload_Request_Handler_Test, POSTBlogUploadSuccessTest) {
  // Set up request for get handle
  std::string path = "/blog";
  SetPostBlogHandler(path);

  Response test = blog_upload_request_handler_->handle_request(request_);
  EXPECT_EQ(test.code_, Response::moved_temporarily);
}


