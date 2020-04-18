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
	read_buff_ptr_ = boost::asio::buffer_cast<unsigned const char*>(const_buffer_);
	std::string check_status_string_ = "";

	for (int i = 0; i < const_buffer_size_; i++){
		if(*(read_buff_ptr_)!= '\0')
			check_status_string_ += *(read_buff_ptr_);
		read_buff_ptr_++;
	}

	EXPECT_EQ(check_status_string_, " 200 OK\r\n");
}

TEST_F(ReplyTest, ToBufferBADReply) {

	const_buffer_ = http::server::status_strings::to_buffer(http::server::reply::bad_request);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	read_buff_ptr_ = boost::asio::buffer_cast<unsigned const char*>(const_buffer_);
	std::string check_status_string_ = "";

	for (int i = 0; i < const_buffer_size_; i++){
		if(*(read_buff_ptr_)!= '\0')
			check_status_string_ += *(read_buff_ptr_);
		read_buff_ptr_++;
	}

	EXPECT_EQ(check_status_string_, " 400 Bad Request\r\n");
}

TEST_F(ReplyTest, ToBufferOtherReply) {

	const_buffer_ = http::server::status_strings::to_buffer(http::server::reply::not_implemented);
	const_buffer_size_ = boost::asio::buffer_size(const_buffer_);
	read_buff_ptr_ = boost::asio::buffer_cast<unsigned const char*>(const_buffer_);
	std::string check_status_string_ = "";

	for (int i = 0; i < const_buffer_size_; i++){
		if(*(read_buff_ptr_)!= '\0')
			check_status_string_ += *(read_buff_ptr_);
		read_buff_ptr_++;
	}

	EXPECT_EQ(check_status_string_, " 400 Bad Request\r\n");
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