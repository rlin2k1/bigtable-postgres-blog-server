#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

#include "header.h"
#include "response_builder.h"
#include "response_parser.h"
#include "response.h"

class ResponseParserTest : public ::testing::Test {
    protected:
        response_builder response_builder_;
        response_parser response_parser_;
        response_parser::result_type result;
        Response response_;
};

TEST_F(ResponseParserTest, ParseContentLengthResponse) {
    char data[] = "HTTP/1.1 400 Bad Request\r\n\
Server: awselb/2.0\r\n\
Content-Type: text/html\r\n\
Content-Length: 138\r\n\
Connection: close\r\n\
\r\n\
<html>\r\n\
<head><title>400 Bad Request</title></head>\r\n\
<body bgcolor=\"white\">\r\n\
<center><h1>400 Bad Request</h1></center>\r\n\
</body>\r\n\
</html>\r\n";

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data, data + strlen(data));
    EXPECT_EQ(result, response_parser::good);

    EXPECT_EQ(response_builder_.http_version_major, 1);
    EXPECT_EQ(response_builder_.http_version_minor, 1);
    EXPECT_EQ(response_builder_.code, 400);
    std::string reason_string(response_builder_.reasonphrase.begin(), response_builder_.reasonphrase.end());
    EXPECT_EQ(reason_string, "Bad Request");
    EXPECT_FALSE(response_builder_.keep_alive);
    EXPECT_FALSE(response_builder_.chunked);

    std::vector<header> golden_headers {
        header{"Server", "awselb/2.0"},
        header{"Content-Type", "text/html"},
        header{"Content-Length", "138"},
        header{"Connection", "close"}
    };
    EXPECT_EQ(response_builder_.headers, golden_headers);

    std::string golden_body = "<html>\r\n\
<head><title>400 Bad Request</title></head>\r\n\
<body bgcolor=\"white\">\r\n\
<center><h1>400 Bad Request</h1></center>\r\n\
</body>\r\n\
</html>\r\n";
    std::string body_string(response_builder_.body.begin(), response_builder_.body.end());
    EXPECT_EQ(body_string, golden_body);
}

TEST_F(ResponseParserTest, ParseContentLengthTwoPartResponse) {
    char data1[] = "HTTP/1.1 400 Bad Request\r\n\
Server: awselb/2.0\r\n\
Content-Type: text/html\r\n\
Content-Length: 138\r\n\
Connection:";
    char data2[] = " close\r\n\
\r\n\
<html>\r\n\
<head><title>400 Bad Request</title></head>\r\n\
<body bgcolor=\"white\">\r\n\
<center><h1>400 Bad Request</h1></center>\r\n\
</body>\r\n\
</html>\r\n";

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data1, data1 + strlen(data1));
    EXPECT_EQ(result, response_parser::indeterminate);

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data2, data2 + strlen(data2));
    EXPECT_EQ(result, response_parser::good);

    EXPECT_EQ(response_builder_.http_version_major, 1);
    EXPECT_EQ(response_builder_.http_version_minor, 1);
    EXPECT_EQ(response_builder_.code, 400);
    std::string reason_string(response_builder_.reasonphrase.begin(), response_builder_.reasonphrase.end());
    EXPECT_EQ(reason_string, "Bad Request");
    EXPECT_FALSE(response_builder_.keep_alive);
    EXPECT_FALSE(response_builder_.chunked);

    std::vector<header> golden_headers {
        header{"Server", "awselb/2.0"},
        header{"Content-Type", "text/html"},
        header{"Content-Length", "138"},
        header{"Connection", "close"}
    };
    EXPECT_EQ(response_builder_.headers, golden_headers);

    std::string golden_body = "<html>\r\n\
<head><title>400 Bad Request</title></head>\r\n\
<body bgcolor=\"white\">\r\n\
<center><h1>400 Bad Request</h1></center>\r\n\
</body>\r\n\
</html>\r\n";
    std::string body_string(response_builder_.body.begin(), response_builder_.body.end());
    EXPECT_EQ(body_string, golden_body);
}

TEST_F(ResponseParserTest, ParseChunkedResponse) {
    char data1[] = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Transfer-Encoding: chunked\r\n\
Age: 0\r\n\
Connection: keep-alive\r\n\
\r\n\
00000F\r\n\
<!doctype html>\r\n";
    char data2[] = "00000F\r\n\
<!doctype html>\r\n";
    char data3[] = "0\r\n";

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data1, data1 + strlen(data1));
    EXPECT_EQ(result, response_parser::indeterminate);

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data2, data2 + strlen(data2));
    EXPECT_EQ(result, response_parser::indeterminate);

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data3, data3 + strlen(data3));
    EXPECT_EQ(result, response_parser::good);

    EXPECT_EQ(response_builder_.http_version_major, 1);
    EXPECT_EQ(response_builder_.http_version_minor, 1);
    EXPECT_EQ(response_builder_.code, 200);
    std::string reason_string(response_builder_.reasonphrase.begin(), response_builder_.reasonphrase.end());
    EXPECT_EQ(reason_string, "OK");
    EXPECT_TRUE(response_builder_.keep_alive);
    EXPECT_TRUE(response_builder_.chunked);

    std::vector<header> golden_headers {
        header{"Content-Type", "text/html; charset=UTF-8"},
        header{"Transfer-Encoding", "chunked"},
        header{"Age", "0"},
        header{"Connection", "keep-alive"}
    };
    EXPECT_EQ(response_builder_.headers, golden_headers);

    std::string golden_body = "<!doctype html><!doctype html>";
    std::string body_string(response_builder_.body.begin(), response_builder_.body.end());
    EXPECT_EQ(body_string, golden_body);
}

TEST_F(ResponseParserTest, ParseUCLAChunkedResponse) {
    std::ifstream t1("ucla_http_response");
    std::stringstream buffer1;
    buffer1 << t1.rdbuf();
    std::string data = buffer1.str();

    std::tie(result, std::ignore) = response_parser_.parse(
              response_builder_, data.begin(), data.end());
    EXPECT_EQ(result, response_parser::good);

    EXPECT_EQ(response_builder_.http_version_major, 1);
    EXPECT_EQ(response_builder_.http_version_minor, 1);
    EXPECT_EQ(response_builder_.code, 200);
    std::string reason_string(response_builder_.reasonphrase.begin(), response_builder_.reasonphrase.end());
    EXPECT_EQ(reason_string, "OK");
    EXPECT_TRUE(response_builder_.keep_alive);
    EXPECT_TRUE(response_builder_.chunked);

    std::vector<header> golden_headers {
        header{"Content-Type", "text/html; charset=UTF-8"},
        header{"Vary", "Accept-Encoding"},
        header{"Set-Cookie", "PHPSESSID=c6pc83tafttnimu1go6h85rh47; path=/"},
        header{"Expires", "Thu, 19 Nov 1981 08:52:00 GMT"},
        header{"Cache-Control", "no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=86400, public, must-revalidate, proxy-revalidate"},
        header{"Pragma", "no-cache"},
        header{"X-Frame-Options", "SAMEORIGIN"},
        header{"Transfer-Encoding", "chunked"},
        header{"Date", "Sat, 23 May 2020 22:13:05 GMT"},
        header{"Age", "171"},
        header{"Connection", "keep-alive"},
        header{"X-Cache", "HIT"}
    };
    EXPECT_EQ(response_builder_.headers, golden_headers);

    std::ifstream t2("ucla_http_response_golden_body");
    std::stringstream buffer2;
    buffer2 << t2.rdbuf();
    std::string golden_body = buffer2.str();
    std::string body_string(response_builder_.body.begin(), response_builder_.body.end());
    EXPECT_EQ(body_string, golden_body);
}
