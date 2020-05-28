#include <string>

#include "gtest/gtest.h"
#include "request_parser.h"
#include "response.h"
#include "health_request_handler.h"
#include "session.h"

class HealthRequestHandler : public ::testing::Test {
 protected:
        // The incoming request.
        request_builder request_builder_;
        // The parser for the incoming request.
        request_parser request_parser_;
        request_parser::result_type result;
        // The handler used to process the incoming request.
        health_request_handler health_request_handler_;
        // The Response to be sent back to the client.
        Response response_;
};

TEST_F(HealthRequestHandler, SimpleGetRequest) {
  char request_data[] = "GET /health HTTP/1.1\r\n\r\n";
  std::string text_payload = "OK";
  std::tie(result, std::ignore) = request_parser_.parse(
            request_builder_, request_data, request_data + sizeof(request_data));
  response_ = health_request_handler_.handle_request(request_builder_.build_request());
  EXPECT_EQ(response_.code_, Response::ok);
  EXPECT_EQ(response_.body_, text_payload);
  std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
  EXPECT_EQ(response_.headers_, response_header);
}
