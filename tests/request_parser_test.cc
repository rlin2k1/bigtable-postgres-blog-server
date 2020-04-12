#include "gtest/gtest.h"

#include "request_parser.h"
#include "request.h"

class RequestParserTest : public ::testing::Test {
    protected:
        // The incoming request.
  		http::server::request request_;
  		// The parser for the incoming request.
  		http::server::request_parser request_parser_;
        http::server::request_parser::result_type result;
};

TEST_F(RequestParserTest, GETRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\n\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);
}

TEST_F(RequestParserTest, BadRequest) {
    char data[] = "HTTP REQUEST GET /";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, EmptyRequest) {
    char data[] = "";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, HeaderBodyRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\nUser-Agent: nc/0.01\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\nBODY";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);
}


TEST_F(RequestParserTest, EmptyBodyRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);
}

TEST_F(RequestParserTest, POSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.1\r\nUser-Agent: nc/0.0.1\r\nHost: 127.0.0.1\r\n\r\nname1=value1&name2=value2";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);
}
