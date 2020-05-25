#include "gtest/gtest.h"
#include "proxy_request_handler.h"
#include "config_parser.h"
#include "request.h"

class Proxy_Request_HandlerTest : public ::testing::Test {
 protected:
  std::unique_ptr<proxy_request_handler> proxy_request_handler_;
  Request request_;
  NginxConfig config;
  void SetUp() override {
    NginxConfigParser parser;
    parser.Parse("proxy_test_config", &config);
  }

  void SetHandler(std::string path, std::string uri) {
    proxy_request_handler_.reset(proxy_request_handler::Init(path, config));
    request_.method_ = Request::MethodEnum::GET;
    request_.uri_ = path + uri;
    request_.version_ = "HTTP/1.1";
  }

  std::string GetGolden(std::string path) {
    std::ifstream fs(path, std::ios_base::in);
    std::stringstream ss;
    ss << fs.rdbuf();
    return ss.str();
  }

  void RemovePath(std::string& body, std::string& path) {
    std::string::size_type n = path.length();
    for (std::string::size_type i = body.find(path); i != std::string::npos;
         i = body.find(path)) {
      body.erase(i, n);
    }
  }
};

TEST_F(Proxy_Request_HandlerTest, UCLATest) {
  // Tests if proxy handler properly gets hotdog image from radd.cs130.org
  std::string golden = GetGolden("./favicongolden.ico");
  std::string path = "/UCLAtest";
  SetHandler(path, "/favicon.ico");
  Response test = proxy_request_handler_->handle_request(request_);
  EXPECT_EQ(test.code_, 200);
  EXPECT_EQ(test.body_, golden);
}

TEST_F(Proxy_Request_HandlerTest, ExampleTest) {
  // Tests if proxy handler properly gets website wwww.example.com
  std::string golden = GetGolden("./example.html");
  std::string path = "/exampletest";
  SetHandler(path, "");
  Response test = proxy_request_handler_->handle_request(request_);
  RemovePath(test.body_, path);
  EXPECT_EQ(test.code_, 200);
  EXPECT_EQ(test.body_, golden);
}
