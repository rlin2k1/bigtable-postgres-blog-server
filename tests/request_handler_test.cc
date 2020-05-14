#include <string>

#include "gtest/gtest.h"
#include "header.h"
#include "request_parser.h"
#include "response.h"
#include "echo_request_handler.h"
#include "session.h"

class RequestHandlerTest : public ::testing::Test {
 protected:
        // The incoming request.
        request_builder request_builder_;
        // The parser for the incoming request.
        request_parser request_parser_;
        request_parser::result_type result;
        // The handler used to process the incoming request.
        echo_request_handler echo_request_handler_;
        // The Response to be sent back to the client.
        Response response_;
};

TEST_F(RequestHandlerTest, GETRequest) {
    char data[] = "GET /echo HTTP/1.1\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };

    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, BadRequest) {
    char data[] = "HTTP REQUEST GET /";
    std::tie(result, std::ignore) = request_parser_.parse(
             request_builder_, data, data + sizeof(data));

    response_ = ResponseHelperLibrary::stock_response(Response::bad_request);

    EXPECT_EQ(response_.code_, Response::bad_request);

    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(response_.body_, std::string(bad_request));

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/html"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, EmptyRequest) {
    char data[] = "";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = ResponseHelperLibrary::stock_response(Response::bad_request);

    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(response_.body_, std::string(bad_request));

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/html"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, InvalidURI) {
    char data[] = "GET / HTTP/2.1\r\n\
User-Agent: nc/0.01\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\nBODY";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = ResponseHelperLibrary::stock_response(Response::bad_request);

    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(response_.body_, std::string(bad_request));

   std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/html"} };
    EXPECT_EQ(response_.headers_, response_header);
}


TEST_F(RequestHandlerTest, EmptyBodyRequest) {
    char data[] = "GET /echo HTTP/3.1\r\nAccept: */*\r\nHost: 127.0.0.1\r\nUser-Agent: Chrome\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, POSTRequest) {
    char data[] = "POST /echo HTTP/1.0\r\n\
Content-Length: 25\r\nHost: 127.1.1.1\r\nUser-Agent: Firefox\r\n\r\n\
name1=value1&name2=value2";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, LongPOSTRequest) {
    char data[] = "POST /echo HTTP/1.0\r\n\
Content-Length: 1479\r\nHost: 127.1.1.1\r\nUser-Agent: Firefox\r\n\r\n\
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
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, TrickyLongPOSTRequest) {
    char data[] = "POST /echo HTTP/1.0\r\n\
Content-Length: 1601\r\nHost: 127.1.1.1\r\nUser-Agent: Firefox\r\n\r\n\
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
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, Exactly1024POSTRequest) {
    char data[] = "POST /echo HTTP/1.0\r\n\
Content-Length: 954\r\nHost: 127.1.1.1\r\nUser-Agent: Firefox\r\n\r\n\
once upon a time.............................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
..the end";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}

TEST_F(RequestHandlerTest, Exactly1025POSTRequest) {
    char data[] = "POST /echo HTTP/1.0\r\n\
Content-Length: 955\r\nHost: 127.1.1.1\r\nUser-Agent: Firefox\r\n\r\n\
once upon a time.............................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
.............................................\
...the end";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_builder_, data, data + sizeof(data));

    response_ = echo_request_handler_.handle_request(request_builder_.build_request());
    EXPECT_EQ(response_.code_, Response::ok);
    EXPECT_EQ(response_.body_, data);

    std::map<std::string, std::string> response_header = { {"Content-Length", std::to_string(response_.body_.size())}, {"Content-Type", "text/plain"} };
    EXPECT_EQ(response_.headers_, response_header);
}
