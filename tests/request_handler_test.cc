#include "gtest/gtest.h"

#include "header.h"
#include "request.h"
#include "request_parser.h"
#include "request_handler.h"
#include "reply.h"

class RequestHandlerTest : public ::testing::Test {
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

TEST_F(RequestHandlerTest, GETRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\n\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/index.html");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_TRUE(request_.headers.empty());

    request_handler_.handle_request(request_, reply_, data);
    EXPECT_EQ(reply_.status, http::server::reply::ok);
    EXPECT_EQ(reply_.content, data);

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/plain"}};
    EXPECT_EQ(reply_.headers, reply_header);
}

TEST_F(RequestHandlerTest, BadRequest) {
    char data[] = "HTTP REQUEST GET /";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);

    reply_ = http::server::reply::stock_reply(http::server::reply::bad_request);

    EXPECT_EQ(reply_.status, http::server::reply::bad_request);

    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(reply_.content, std::string(bad_request));

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/html"}};
    EXPECT_EQ(reply_.headers, reply_header);
}

TEST_F(RequestHandlerTest, EmptyRequest) {
    char data[] = "";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + sizeof(data));
    EXPECT_EQ(result, http::server::request_parser::bad);

    reply_ = http::server::reply::stock_reply(http::server::reply::bad_request);

    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(reply_.content, std::string(bad_request));

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/html"}};
    EXPECT_EQ(reply_.headers, reply_header);
}

TEST_F(RequestHandlerTest, HeaderBodyRequest) {
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

    request_handler_.handle_request(request_, reply_, data);
    EXPECT_EQ(reply_.status, http::server::reply::ok);
    EXPECT_EQ(reply_.content, data);

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/plain"}};
    EXPECT_EQ(reply_.headers, reply_header);
}


TEST_F(RequestHandlerTest, EmptyBodyRequest) {
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

    request_handler_.handle_request(request_, reply_, data);
    EXPECT_EQ(reply_.status, http::server::reply::ok);
    EXPECT_EQ(reply_.content, data);

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/plain"}};
    EXPECT_EQ(reply_.headers, reply_header);
}

TEST_F(RequestHandlerTest, POSTRequest) {
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

    request_handler_.handle_request(request_, reply_, data);
    EXPECT_EQ(reply_.status, http::server::reply::ok);
    EXPECT_EQ(reply_.content, data);

    std::vector<http::server::header> reply_header { \
    http::server::header{"Content-Length", \
    std::to_string(reply_.content.size())}, \
    http::server::header{"Content-Type", "text/plain"}};
    EXPECT_EQ(reply_.headers, reply_header);
}
