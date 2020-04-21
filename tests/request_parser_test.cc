#include "gtest/gtest.h"

#include "header.h"
#include "request.h"
#include "request_parser.h"
#include "echo_request_handler.h"
#include "reply.h"

class RequestParserTest : public ::testing::Test {
 protected:
        // The incoming request.
        http::server::request request_;
        // The parser for the incoming request.
        http::server::request_parser request_parser_;
        http::server::request_parser::result_type result;
        // The reply to be sent back to the client.
        http::server::reply reply_;
};

TEST_F(RequestParserTest, ParseGETRequest) {
    char data[] = "GET /index.html HTTP/1.1\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/index.html");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    EXPECT_TRUE(request_.headers.empty());
}

TEST_F(RequestParserTest, ParseTwoPartGETRequest) {
    char data1[] = "GET /index.html HTT";
    char data2[] = "P/1.1\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data1, data1 + strlen(data1));
    EXPECT_EQ(result, http::server::request_parser::indeterminate);

    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data2, data2 + strlen(data2));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/index.html");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    EXPECT_TRUE(request_.headers.empty());
}

TEST_F(RequestParserTest, ParseBadRequest) {
    char data[] = "HTTP REQUEST GET /";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, ParseGETRequestWithIllegalBody) {
    char data[] = "GET /index.html HTTP/1.1\r\n\r\n\
i shouldn't be sending this unless i have a content length";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, ParseEmptyRequest) {
    char data[] = "";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::indeterminate);
}
// This should fail.
TEST_F(RequestParserTest, ParseHeaderBodyRequest) {
    char data[] = "GET / HTTP/2.1\r\n\
User-Agent: nc/0.01\r\nHost: 127.0.0.1\r\nAccept: */*\r\nContent-Length: 4\r\n\r\nBODY";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/");
    EXPECT_EQ(request_.http_version_major, 2);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "nc/0.01"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"},
        http::server::header{"Content-Length", "4"}
    };

    EXPECT_EQ(request_.headers, request_header);
}


TEST_F(RequestParserTest, ParseEmptyBodyGETRequest) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/test.html");
    EXPECT_EQ(request_.http_version_major, 3);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Chrome"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParsePOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\nContent-Length: 25\r\n\r\n\
name1=value1&name2=value2";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Firefox"},
        http::server::header{"Host", "127.1.1.1"},
        http::server::header{"Content-Length", "25"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseEmptyBodyPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\nContent-Length: 0\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Firefox"},
        http::server::header{"Host", "127.1.1.1"},
        http::server::header{"Content-Length", "0"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseIncorrentContentLengthPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\nContent-Length: 1\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::indeterminate);
}

TEST_F(RequestParserTest, ParseLongPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\nContent-Length: 1024\r\n\r\n\
a...................................................................\
....................................................................\
.....................................................................\
.....................................................................\
.....................................................................\
......................................................................\
......................................................................\
......................................................................\
......................................................................\
.......................................................................\
.......................................................................\
......................................................................\
......................................................................\
.......................................................................\
...............................................z";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Firefox"},
        http::server::header{"Host", "127.1.1.1"},
        http::server::header{"Content-Length", "1024"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseTrickyLongPOSTRequest) {
    char data[] = "POST /test/test.php HTTP/1.0\r\n\
User-Agent: Firefox\r\nHost: 127.1.1.1\r\nContent-Length: 1047\r\n\r\n\
once upon a timea............................................................\
.............................................................................\
........................................................................\
........................................................................\
.........................................................................\
.........................................................................\
.....................................................................\r\n\r\n.\
..............................................................................\
..............................................................................\
.............................................................................\
..............................................................................\
..............................................................................\
..............................................................................\
......................................................zthe end";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "POST");
    EXPECT_EQ(request_.uri, "/test/test.php");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 0);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Firefox"},
        http::server::header{"Host", "127.1.1.1"},
        http::server::header{"Content-Length", "1047"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, ParseGoogleGETRequest) {
    char data[] = "GET / HTTP/1.1\r\nHost: 34.83.52.12\r\n\
Connection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_0) AppleWebKit/537.36 \
(KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36\r\nAccept: \
text/html,application/xhtml+xml,application/xml;q=0.9,\
image/webp,image/apng,*/*;q=0.8,application/signed-exchange;\
v=b3;q=0.9\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: \
en-US,en;q=0.9\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_TRUE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"Host", "34.83.52.12"},
        http::server::header{"Connection", "keep-alive"},
        http::server::header{"Upgrade-Insecure-Requests", "1"},
        http::server::header{"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36"},
        http::server::header{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"},
        http::server::header{"Accept-Encoding", "gzip, deflate"},
        http::server::header{"Accept-Language", "en-US,en;q=0.9"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, GETRequestNoNewLine2) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\rHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestNoSpaceBeforeHeaderValue) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent:Chrome\r\nHost: 127.0.0.1\r\n\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestInvalidCharBeginning) {
    char data[] = "{GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestControlCodeInHeaderValue) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: '\a'Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadHeaderName) {
    char data[] = "GET /test.html HTTP/1.1\r\n\
User-AgentÇ Chrome\r\nHost: 127.0.0.1\r\n\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadHeaderName2) {
    char data[] = "GET /test.html HTTP/1.1\r\n\
User-Agent\t Chrome\r\nHost: 127.0.0.1\r\n\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadHeaderName3) {
    char data[] = "GET /test.html HTTP/1.1\r\n\
User-Agent^ Chrome\r\nHost: 127.0.0.1\r\n\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETHTTPVersionMajorTwoDigits) {
    char data[] = "GET /test.html HTTP/13.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/test.html");
    EXPECT_EQ(request_.http_version_major, 13);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Chrome"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, GETHTTPVersionMinorTwoDigits) {
    char data[] = "GET /test.html HTTP/3.12\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/test.html");
    EXPECT_EQ(request_.http_version_major, 3);
    EXPECT_EQ(request_.http_version_minor, 12);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Chrome"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, GETHTTPVersionMinorLetterInsteadOfNum) {
    char data[] = "GET /test.html HTTP/3.a\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETBadHeaderLineStartExtraSpace) {
    char data[] = "GET /test.html HTTP/3.1 \n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETBadHeaderExpectingNewLine) {
    char data[] = "GET /test.html HTTP/3.1\r\r\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETBadRequestHeaderSpecialCharAtLineStart) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
 : Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMinorVersion) {
    char data[] = "GET /test.html HTTP/3. \r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMajorVersionStart) {
    char data[] = "GET /test.html HTTP/Ç.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMajorVersionEnd) {
    char data[] = "GET /test.html HTTP/3Ç.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestCtlAsURI) {
    char data[] = "GET /\v HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMethodName) {
    char data[] = "GÇT /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMethodName2) {
    char data[] = "G@T /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestBadMethodName3) {
    char data[] = "G\tT /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestHTTPBad1stT) {
    char data[] = "GET /test.html HÇTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestHTTPBad2ndT) {
    char data[] = "GET /test.html HTÇP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestHTTPBadP) {
    char data[] = "GET /test.html HTTÇ/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETRequestHTTPNoVersionSlash) {
    char data[] = "GET /test.html HTTP3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, RequestParserReset) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    // TODO/HELP: simply called reset function for branch coverage,
    // wasn't sure how to check expected output after resetting
    // request_parser.
    request_parser_.reset();
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/test.html");
    EXPECT_EQ(request_.http_version_major, 3);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_FALSE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"User-Agent", "Chrome"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"}
    };

    EXPECT_EQ(request_.headers, request_header);
}

TEST_F(RequestParserTest, GETBadHeaderLineStartSpecial) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
\tUser-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETBadHeaderLineStartNotChar) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
ÇUser-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, GETMethodStartNotChar) {
    char data[] = "ÇET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, HeaderLWS1) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\n\t\rAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, HeaderLWS4) {
    char data[] = "GET /test.html HTTP/3.1\r\n\
User-Agent: Chrome\r\nHost: 127.0.0.1\r\n\t\vAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::bad);
}

TEST_F(RequestParserTest, ConnectionKeepAlive) {
    char data[] = "GET /index.html HTTP/1.1\r\n\
Connection: Keep-Alive\r\nUser-Agent: Chrome\r\n\
Host: 127.0.0.1\r\nAccept: */*\r\n\r\n";
    std::tie(result, std::ignore) = request_parser_.parse(
              request_, data, data + strlen(data));
    EXPECT_EQ(result, http::server::request_parser::good);

    EXPECT_EQ(request_.method, "GET");
    EXPECT_EQ(request_.uri, "/index.html");
    EXPECT_EQ(request_.http_version_major, 1);
    EXPECT_EQ(request_.http_version_minor, 1);
    EXPECT_TRUE(request_.keep_alive);

    std::vector<http::server::header> request_header {
        http::server::header{"Connection", "Keep-Alive"},
        http::server::header{"User-Agent", "Chrome"},
        http::server::header{"Host", "127.0.0.1"},
        http::server::header{"Accept", "*/*"}
    };

    EXPECT_EQ(request_.headers, request_header);
}
