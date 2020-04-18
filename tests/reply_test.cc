#include "gtest/gtest.h"

#include "header.h"
#include "request.h"
#include "request_parser.h"
#include "request_handler.h"
#include "reply.h"
#include "string"

class ReplyTest : public ::testing::Test {
 protected:

        // The reply to be sent back to the client.
        http::server::reply reply_;
        // Buffer which holds the status string.
        boost::asio::const_buffer const_buffer_;
        // Size of status string buffer.
        std::size_t const_buffer_size_;
        // Pointer which reads values from status string buffer.
        unsigned const char* read_buff_ptr_;
};

TEST_F(ReplyTest, ToBufferOKReply) {

	const_buffer_ = http::server::status_strings::to_buffer(http::server::reply::ok);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 200 OK\r\n");
}

TEST_F(ReplyTest, ToBufferBADReply) {

	const_buffer_ = http::server::status_strings::to_buffer(http::server::reply::bad_request);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 400 Bad Request\r\n");
}

TEST_F(ReplyTest, ToBufferOtherReply) {

	const_buffer_ = http::server::status_strings::to_buffer(http::server::reply::not_implemented);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	std::string check_status_string_(boost::asio::buffer_cast<const char*>(const_buffer_));
	EXPECT_EQ(check_status_string_, "HTTP/1.0 400 Bad Request\r\n");
}

TEST_F(ReplyTest, ReplyToBuffersTest) {
	
	reply_.status = http::server::reply::status_type::ok;
	reply_.headers.push_back(http::server::header{"User-Agent", "Firefox"});
	reply_.headers.push_back(http::server::header{"Host", "127.1.1.1"});
	reply_.headers.push_back(http::server::header{"Content-Length", "0"});
	std::vector<boost::asio::const_buffer> buffers = reply_.to_buffers();
	std::string useragent(boost::asio::buffer_cast<const char*>(buffers[1]));
	std::string useragentval(boost::asio::buffer_cast<const char*>(buffers[3]));
	std::string host(boost::asio::buffer_cast<const char*>(buffers[5]));
	std::string hostval(boost::asio::buffer_cast<const char*>(buffers[7]));
	std::string contentlength(boost::asio::buffer_cast<const char*>(buffers[9]));
	std::string contentlengthval(boost::asio::buffer_cast<const char*>(buffers[11]));
	EXPECT_EQ(useragent, "User-Agent");
	EXPECT_EQ(useragentval, "Firefox");
	EXPECT_EQ(host, "Host");
	EXPECT_EQ(hostval, "127.1.1.1");
	EXPECT_EQ(contentlength, "Content-Length");
	EXPECT_EQ(contentlengthval, "0");
}

TEST_F(ReplyTest, StockRepliesToStringOtherRequest) {
	
	reply_ = http::server::reply::stock_reply(http::server::reply::not_implemented);
    EXPECT_EQ(reply_.status, http::server::reply::not_implemented);
    const char bad_request[] =
    "<html>"
    "<head><title>Bad Request</title></head>"
    "<body><h1>400 Bad Request</h1></body>"
    "</html>";
    EXPECT_EQ(reply_.content, std::string(bad_request));
}