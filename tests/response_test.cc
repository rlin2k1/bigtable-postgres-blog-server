#include "gtest/gtest.h"

#include "header.h"
#include "request.h"
#include "request_parser.h"
#include "request_handler.h"
#include "response.h"
#include "string"
#include "session.h"

class ResponseTest : public ::testing::Test {
 protected:

        // The response to be sent back to the client.
        http::server::Response response_;
        // Buffer which holds the status string.
        boost::asio::const_buffer const_buffer_;
        // Size of status string buffer.
        std::size_t const_buffer_size_;
        // Pointer which reads values from status string buffer.
        unsigned const char* read_buff_ptr_;
};

TEST_F(ResponseTest, ToBufferOKReply) {

	const_buffer_ = ResponseHelperLibrary::to_buffer(http::server::Response::ok);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 200 OK\r\n");
}

TEST_F(ResponseTest, ToBufferBADReply) {

	const_buffer_ = ResponseHelperLibrary::to_buffer(http::server::Response::bad_request);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 400 Bad Request\r\n");
}

TEST_F(ResponseTest, ToBufferOtherReply) {

	const_buffer_ = ResponseHelperLibrary::to_buffer(http::server::Response::not_implemented);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 400 Bad Request\r\n");
}

TEST_F(ResponseTest, ReplyToBuffersTest) {

	response_.code_ = http::server::Response::StatusCode::ok;
	response_.headers_["Content-Length"] = "0";
	response_.headers_["Host"] = "127.1.1.1";
	response_.headers_["User-Agent"] = "Firefox";
	// Ordered by Key
	std::vector<boost::asio::const_buffer> buffers = ResponseHelperLibrary::to_buffers(response_);
	std::string contentlength(boost::asio::buffer_cast<const char*>(buffers[1]));
	std::string contentlengthval(boost::asio::buffer_cast<const char*>(buffers[3]));
	std::string host(boost::asio::buffer_cast<const char*>(buffers[5]));
	std::string hostval(boost::asio::buffer_cast<const char*>(buffers[7]));
	std::string useragent(boost::asio::buffer_cast<const char*>(buffers[9]));
	std::string useragentval(boost::asio::buffer_cast<const char*>(buffers[11]));

	EXPECT_EQ(contentlength, "Content-Length");
	EXPECT_EQ(contentlengthval, "0");
	EXPECT_EQ(host, "Host");
	EXPECT_EQ(hostval, "127.1.1.1");
	EXPECT_EQ(useragent, "User-Agent");
	EXPECT_EQ(useragentval, "Firefox");
}

TEST_F(ResponseTest, StockRepliesToStringOtherRequest) {

	response_ = ResponseHelperLibrary::stock_response(http::server::Response::not_implemented);
    EXPECT_EQ(response_.code_, http::server::Response::not_implemented);
    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(response_.body_, std::string(bad_request));
}
