#include "gtest/gtest.h"

#include "header.h"
#include "request.h"
#include "request_parser.h"
#include "request_handler.h"
#include "reply.h"

class RequestParserTest : public ::testing::Test {
 protected:
        // The incoming request.
        http::server::request request_;
        // The parser for the incoming request.
        http::server::request_parser request_parser_;
        http::server::request_parser::result_type result;

        // The handler used to process the incoming request.
        http::server::request_handler request_handler_;
        // The reply to be sent back to the client.
        http::server::reply reply_;
};

TEST_F(RequestParserTest, ParseGETRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\n\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/index.html");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_TRUE(request_.headers.empty());
}

TEST_F(RequestParserTest, ParseBadRequest) {
    char data[] = "HTTP REQUEST GET /";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, ParseEmptyRequest) {
    char data[] = "";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, ParseHeaderBodyRequest) {
    char data[] = "GET / HTTP/2.1\r\n\
User-Agent: nc/0.01\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\nBODY";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/");
    EXPECT_EQ(request_.http_version_major, 2);
    EXPECT_EQ(request_.http_version_minor, 1);

    std::vector<http::server::header> request_header {\
    http::server::header{"User-Agent", "nc/0.01"}, \
    http::server::header{"Host", "127.0.0.1"}, \
    http::server::header{"Accept", "*/*"}};

    EXPECT_EQ(request_.headers, request_header);
}


TEST_F(RequestParserTest, ParseEmptyBodyRequest) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/test.html");
    EXPECT_EQ(request_.http_version_major, 3);
    EXPECT_EQ(request_.http_version_minor, 1);

    std::vector<http::server::header> request_header {\
    http::server::header{"User-Agent", "Chrome"}, \
    http::server::header{"Host", "127.0.0.1"}, \
    http::server::header{"Accept", "*/*"}};

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParsePOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\n\r\n\
name1=value1&name2=value2";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);

    std::vector<http::server::header> request_header \
    {http::server::header{"User-Agent", "Firefox"}, \
    http::server::header{"Host", "127.1.1.1"}};

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseLongPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\n\r\n\
once upon a time..............................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
the end";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);

    std::vector<http::server::header> request_header \
    {http::server::header{"User-Agent", "Firefox"}, \
    http::server::header{"Host", "127.1.1.1"}};

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseTrickyLongPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\n\r\n\
once upon a time..............................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
............\r\n\r\n..........................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
............\r\n\r\n..........................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
............\r\n\r\n..........................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
............\r\t\r\r..........................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
..............................................\
the end";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);

    std::vector<http::server::header> request_header \
    {http::server::header{"User-Agent", "Firefox"}, \
    http::server::header{"Host", "127.1.1.1"}};

    EXPECT_EQ(request_.headers, request_header);
}

